#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <cuda_runtime.h>
#include "helper_image.h"

#define checkCudaErrors(err)           __checkCudaErrors (err, __FILE__, __LINE__)

#define THREAD_COUNT 128

__global__ void
medianFilter(const unsigned char *inImg, unsigned char *outImg, unsigned int filterSize, unsigned int imgSize){
    unsigned int p = (blockIdx.x * blockDim.x + threadIdx.x) + (blockIdx.y * blockDim.y + threadIdx.y);
    
    if(p < imgSize){
        memcpy(outImg, inImg, imgSize);
    }
}

int main(int argc, char * argv[]){
    cudaError_t err = cudaSuccess;
    
    if(argc<4){
        std::cout << "Not enough arguments. Format:" << std::endl;
        std::cout << "path/to/homework5_kernel filterSize /path/to/input/file /path/to/output/file" << std::endl;
        std::cout << "Valid filter sizes: 3,7,11,15" << std::endl;
        
        return 0;
    }
    
    unsigned int filterSize = std::stoi(argv[1]);
    std::string tempIn = argv[2];
    std::string tempOut = argv[3];
    
    char inFile[tempIn.length()+1];
    char outFile[tempOut.length()+1];
    
    strcpy(inFile, tempIn.c_str());
    strcpy(outFile, tempOut.c_str());
    
    unsigned int width, height;
    unsigned char *dInImg = NULL;
    unsigned char *dOutImg = NULL;
    
    unsigned char *hInImg = NULL;
    unsigned char *hOutImg = NULL;
    
    
    sdkLoadPGM(inFile, &hInImg, &width, &height);
    
    const unsigned int size = width * height * sizeof(unsigned char);
    
    
    hOutImg = (unsigned char *) malloc(size);
    
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
    
    
    medianFilter<<<size/THREAD_COUNT,THREAD_COUNT>>>(dInImg, dOutImg, filterSize, size);
    
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
    
   // memcpy(hOutImg, hInImg, size);
    
    sdkSavePGM(outFile, hOutImg, width, height);
    
    std::cout << "made it" << std::endl;
    
    cudaFree(dInImg);
    cudaFree(dOutImg);
    
    free(hInImg);
    free(hOutImg);
    
    err = cudaDeviceReset();
    

    return 0;
}