#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <ctime>
#include <cuda_runtime.h>
#include "helper_image.h"

#define checkCudaErrors(err)           __checkCudaErrors (err, __FILE__, __LINE__)

//#define THREAD_COUNT 128
#define MAX_SIZE 225

void goldMedianFilter(const unsigned char *inImg, unsigned char *outImg, unsigned int width, unsigned int height, unsigned int filterSize, unsigned int imgSize){
    
    unsigned int filterRadius = filterSize/2;
    
    std::vector<unsigned char> in(inImg, inImg+imgSize); 
    for(unsigned int p=0; p<in.size(); p++){ //pixel
        std::vector<unsigned char> sortable;
        for(unsigned int j=0; j<filterSize; j++){ //row
            for(unsigned int k=0; k<filterSize; k++){ // column
                if(p+((filterRadius)*j + (k-filterRadius))<imgSize && filterRadius*j<height && j*filterRadius>0 && k-filterRadius>0 && (k)-filterRadius<width){
                    sortable.push_back(in.at(p+((filterRadius)*j + (k-filterRadius))));
                }
            }
        }
        std::sort(sortable.begin(), sortable.end());
        const unsigned int sortableSize = sortable.size();
        //std::cout << sortableSize << std::endl;
        if(sortableSize>0)
            outImg[p] = sortable.at(sortableSize/2);
        else
            outImg[p] = in.at(p);
    }
        
}

__global__ void
medianFilter(const unsigned char *inImg, unsigned char *outImg, unsigned int width, unsigned int height, unsigned int filterSize){
    unsigned int xPos = (blockIdx.x * blockDim.x) + threadIdx.x;
    unsigned int yPos = (blockIdx.y * blockDim.y) + threadIdx.y;
    
    unsigned int filterRadius = filterSize/2;
    unsigned char sortable[MAX_SIZE];
    if(xPos < width && yPos < height){
        unsigned int count = 0;
        for(unsigned int i = 0; i<filterSize; i++){
            for(unsigned int j = 0; j<filterSize; j++){
                if(xPos-filterRadius+i < width && yPos-filterRadius+j < height){
                    sortable[count] = 
                    inImg[(xPos-filterRadius+i)*width+(yPos-filterRadius+j)];
                    count++;
                }
            }
        }
        //printf("Count %d", count);
        int i, key, j;
        for (i = 1; i < filterSize*filterSize; i++){
            key = sortable[i];
            j = i - 1;
            while (j >= 0 && sortable[j] > key){
                sortable[j + 1] = sortable[j];
                j = j - 1;
           }
            sortable[j + 1] = key;
        }
        outImg[xPos*width+yPos] = sortable[(filterSize*filterSize)/2];
    }
}

std::chrono::time_point<std::chrono::steady_clock> get_time(){
    return std::chrono::steady_clock::now();
}

std::chrono::duration<double> calculate_elapsed_time(
    std::chrono::time_point<std::chrono::steady_clock> start,
    std::chrono::time_point<std::chrono::steady_clock> end
    ){
        return end - start;
}

int main(int argc, char * argv[]){
    cudaError_t err = cudaSuccess;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    std::chrono::time_point<std::chrono::steady_clock> begin;
    std::chrono::time_point<std::chrono::steady_clock> end;
    
    std::chrono::time_point<std::chrono::steady_clock> overallBegin;
    std::chrono::time_point<std::chrono::steady_clock> overallEnd;
    
    double totalTime = 0;
    double goldTime = 0;
    
    
    overallBegin = get_time();
    
    
    if(argc<4){
        std::cout << "Not enough arguments. Format:" << std::endl;
        std::cout << "path/to/homework5_kernel filterSize /path/to/input/file /path/to/output/file" << std::endl;
        std::cout << "Valid filter sizes: 3,7,11,15" << std::endl;
        
        return 0;
    }
    
    unsigned int filterSize = std::stoi(argv[1]);
    std::string tempIn = argv[2];
    std::string tempOut = argv[3];
    std::string tempGold = "gold_";
    std::string tempOutGold = tempGold+tempOut;
    
    char inFile[tempIn.length()+1];
    char outFile[tempOut.length()+1];
    char goldOutFile[tempOutGold.length()+1];
    
    strcpy(inFile, tempIn.c_str());
    strcpy(outFile, tempOut.c_str());
    strcpy(goldOutFile, tempOutGold.c_str());
    
    unsigned int width, height;
    unsigned char *dInImg = NULL;
    unsigned char *dOutImg = NULL;
    
    unsigned char *hInImg = NULL;
    unsigned char *hOutImg = NULL;
    unsigned char *goldOutImg = NULL;
    
    
    sdkLoadPGM(inFile, &hInImg, &width, &height);
    
    const unsigned int size = width * height * sizeof(unsigned char);
    
    hOutImg = (unsigned char *) malloc(size);
    
    goldOutImg = (unsigned char *) malloc(size);
    
    err = cudaMalloc(&dInImg, size);
    
    if (err != cudaSuccess){
        fprintf(stderr, "dInImg Alloc Failed (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    err = cudaMalloc(&dOutImg, size);
    
    if (err != cudaSuccess){
        fprintf(stderr, "dOutImg Alloc Failed (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    std::cout << width * height << std::endl;
    std::cout << size << std::endl;
    
    
    err = cudaMemcpy(dInImg, hInImg, size, cudaMemcpyHostToDevice);
    
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy img from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    dim3 block(8, 8, 1);
    dim3 grid(64,64,1);
    
    cudaEventRecord(start);
    
    medianFilter<<<grid,block>>>(dInImg, dOutImg, width, height, filterSize);
    
    cudaEventRecord(stop);
    
    err = cudaGetLastError();

    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch medianFilter kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    err = cudaMemcpy(hOutImg, dOutImg, size, cudaMemcpyDeviceToHost);
    
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy img from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
   
    cudaEventSynchronize(stop);
    
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    begin = get_time();
    
    goldMedianFilter(hInImg, goldOutImg, width, height, filterSize, size);
    
    end = get_time();
    
    goldTime = calculate_elapsed_time(begin, end).count();
    
    sdkSavePGM(outFile, hOutImg, width, height);
    
    sdkSavePGM(goldOutFile, goldOutImg, width, height);
    
    std::cout << milliseconds/1000 << std::endl;
    
    cudaFree(dInImg);
    cudaFree(dOutImg);
    
    free(hInImg);
    free(hOutImg);
    free(goldOutImg);
    
    err = cudaDeviceReset();
    
    overallEnd = get_time();
    
    totalTime = calculate_elapsed_time(overallBegin, overallEnd).count();
    
    std::cout << "Total Time: " << totalTime << "s Gold Standard Time: " << goldTime << "s Kernel Time: " << milliseconds/1000 << "s" << std::endl;  

    return 0;
}