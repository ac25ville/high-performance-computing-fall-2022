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
    unsigned int *dInImg = NULL;
    unsigned int *dOutImg = NULL;
    
    unsigned int *hInImg = NULL;
    unsigned int *hOutImg = NULL;
    
    
    sdkLoadPGM(inFile, (unsigned char **) &hInImg, &width, &height);
    
    const unsigned int size = width * height;
    
    err = cudaMalloc((void **)&dInImg, size);
    
    err = cudaMalloc((void **)&dOutImg, size);
    
    err = cudaMemcpy(dInImg, hInImg, size, cudaMemcpyHostToDevice);
    
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy img from host to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    err = cudaMemcpy(dOutImg, hOutImg, size, cudaMemcpyDeviceToHost);
    
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy img from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    
    sdkSavePGM(outFile, hOutImg, width, height);
    
    free(hInImg);
    free(hOutImg);
    
    
    cudaFree(dInImg);
    cudaFree(dOutImg);
    

    return 0;
}