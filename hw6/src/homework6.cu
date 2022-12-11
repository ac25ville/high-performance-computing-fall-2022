#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <chrono>
#include <ctime>
#include <cuda_runtime.h>

#include <unistd.h>
#include <cstdlib>

#define checkCudaErrors(err)           __checkCudaErrors (err, __FILE__, __LINE__)

using namespace std;

struct cntNode{
    double x;
    double y;
    double dy;
};

struct growthInfo{
    double x_offset;
    double y_offset;
    double theta;
    double v;
};

//host functions
vector<cntNode> read_file(string filename, int N);
vector<growthInfo> get_growth_info(vector<cntNode> v, int N);
int check_tubes(cntNode* shm, growthInfo* g, int start, int end, int N, int C, int gen);
double calculate_distance(cntNode a, cntNode b);
void write_to_file(const cntNode * shm, string inputFile, int C, int N);
growthInfo calculate_new_node(cntNode a, cntNode b, growthInfo g);
std::chrono::time_point<std::chrono::steady_clock> get_time();
std::chrono::duration<double> calculate_elapsed_time(std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::time_point<std::chrono::steady_clock> end);

//kernel functions
__global__ void grow_tubes(cntNode* readVector, const unsigned int readVectorSize, cntNode* shm, growthInfo* g, int N, int C, int B);

int main(int argc, char * argv[]){

    //cuda specfics
    cudaError_t err = cudaSuccess;
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    //total time counters
    std::chrono::time_point<std::chrono::steady_clock> totalStartTime;
    std::chrono::time_point<std::chrono::steady_clock> totalEndTime;

    totalStartTime = get_time(); //start timer

    if(argc < 5){ // check for arguments, give hopefully helpful feedback
        cout << "Invalid Argument Amount. Required Arguments & Format:" << endl 
        << "/path/to/homework2 /path/to/input_file N C B" << endl 
        << "N: Number of CNT in file, C: number of growth cycles, B: Number of Blocks (1-4)" << endl;
        return -1;
    }

    //argument reads and conversions
    string filename = argv[1];
    int N = stoi(argv[2]);
    int C = stoi(argv[3]);
    int B = stoi(argv[4]);

    //start and end time for each subsection of the program
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;

    //declare doubles to record times
    double readTime = 0;
    float  multiProcessTime = 0;
    double writeTime = 0;
    double totalTime = 0;

    //read start

    startTime = get_time();

    vector<cntNode> readVector = read_file(filename, N);

    endTime = get_time();

    readTime = calculate_elapsed_time(startTime, endTime).count();

    //read end

    //error check read vector
    if(readVector.empty()){
        cout << "readVector empty" << endl;
        return 1;
    }

    //thread creation start

    cout << "------------------------------" << endl;
    cout << "           GPU START          " << endl;
    cout << "------------------------------" << endl << endl;

    cntNode * hShmPointer = NULL; //pointer to pre-allocated vector, to avoid resizing issues

    cntNode * dShmPointer = NULL;

    hShmPointer = (cntNode *) malloc(sizeof(cntNode) * C*N);

    err = cudaMalloc(&dShmPointer, sizeof(cntNode)  * C*N);

    if (err != cudaSuccess){
        fprintf(stderr, "dShmPointer Alloc Failed (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    vector<growthInfo> growthInfoVector = get_growth_info(readVector, N); //get the intial growth data
    growthInfo * growthInfoPointer = growthInfoVector.data(); //pointer to vector, same as shm

    growthInfo * dGrowthInfoPointer = NULL;

    err = cudaMalloc(&dGrowthInfoPointer, sizeof(growthInfo) * N);

    if (err != cudaSuccess){
        fprintf(stderr, "dGrowthInfoPointer Alloc Failed (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaMemcpy(dGrowthInfoPointer, growthInfoPointer, sizeof(growthInfo) * N, cudaMemcpyHostToDevice);

    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch grow_tubes kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    int offset = N/B;
    
    dim3 block(offset,1,1);
    dim3 grid(N,1,1);

    cudaEventRecord(start);
    grow_tubes<<<grid, block>>>(readVector.data(), readVector.size(), dShmPointer, dGrowthInfoPointer, N, C, B);

    err = cudaGetLastError();

    if (err != cudaSuccess){
        fprintf(stderr, "Failed to launch grow_tubes kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&multiProcessTime, start, stop);
    multiProcessTime /= 1000;

    err = cudaMemcpy(hShmPointer, dShmPointer, sizeof(cntNode) * C*N, cudaMemcpyDeviceToHost);
    
    if (err != cudaSuccess){
        fprintf(stderr, "Failed to copy data from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    cout << "------------------------------" << endl;
    cout << "            GPU END           " << endl;
    cout << "------------------------------" << endl << endl;

    //write start
    startTime = get_time();
    
    write_to_file(hShmPointer, filename, C, N);

    endTime = get_time();

    //write end
    writeTime = calculate_elapsed_time(startTime, endTime).count();

    free(hShmPointer);
    cudaFree(dShmPointer);
    // cudaFree(dGrowthInfoPointer);

    err = cudaDeviceReset();

    totalEndTime = get_time();
    totalTime = calculate_elapsed_time(totalStartTime, totalEndTime).count();

    cout << endl;
    cout << "--------------------------------------------" << endl;
    cout << "              Timing Summary                " << endl;
    cout << "--------------------------------------------" << endl;
    cout << " Total              | " << totalTime   << "s" << endl;
    cout << " Read               | " << readTime    << "s" << endl;
    cout << " Multi Proc Portion | " << multiProcessTime  << "s" << endl;
    cout << " Write              | " << writeTime   << "s" << endl;
    cout << endl;

    return 0;
}

//grow and check

__global__ void 
grow_tubes(cntNode* readVector, const unsigned int readVectorSize, cntNode* shm, growthInfo* g, int N, int C, int B){

    //print inital values as asked for previously, decided not to change this functionality since it oucld be useful
    /* if(start == 0){
        for(int i = 0; i<N; i++){
            std::cout << " Theta: " << (*(g+i)).theta << ", Magnitude: " << (*(g+i)).v << endl;
        }
    */ 

    const unsigned int column = (blockIdx.x * blockDim.x) + threadIdx.x;
    int row;
    /*
    for(int j=0; j<2; j++){
        vector<cntNode> initTemp;
        int readVectorSize = (int)readVector.size();
        row = j * N;
        for(int column=start; column<end; column++){
            cntNode newNode;
            newNode.x = readVector.at(readVectorSize-(N+row)+column).x;
            newNode.y = readVector.at(readVectorSize-(N+row)+column).y;

            initTemp.push_back(newNode);
        }
        std::copy(initTemp.begin(),initTemp.end(), shm + (start+row));
    }
    */
    cntNode newNode_a;
    newNode_a.x = (*(readVector + (readVectorSize-(column)))).x;
    newNode_a.y = (*(readVector + (readVectorSize-(column)))).y;
    
    shm[column] = newNode_a;

    cntNode newNode_b;
    newNode_b.x = (*(readVector + (readVectorSize-(N+column)))).x;
    newNode_b.y = (*(readVector + (readVectorSize-(N+column)))).y;
    
    shm[N+column] = newNode_b;

    

    for(int j = 0; j<C-1; j++){
        row = j * N;
        // g[column] = calculate_new_node((*(shm + column)), (*(shm + column + N)), (*(g+column))); 
                
        cntNode newNode;

        //still adding x & y offset
        newNode.x = (*(shm + column + row + N)).x + (*(g+column)).x_offset; 
        newNode.y = (*(shm + column + row + N)).y + (*(g+column)).y_offset;

        if((column+(N*2))<C*N) //to avoid seg fault; only need to insert above the 2 previous
            shm[row+column+(N*2)] = newNode;
        
        
        
        //maybe barrier? We are going to try hx
        // check_tubes(shm, g, start, end, N, C, j); //check tubes
    }

}

int check_tubes(cntNode* shm, growthInfo* g, int start, int end, int N, int C, int gen){

    int row;
    int column;
    for(column = start; column<end; column++){
        for(int j = 1; j<gen+1; j++){
            cntNode checkVal = *(shm + ((gen+1) * N) + column);
            row = j * N;
            if(
                calculate_distance(*(shm + row + (column -1)), checkVal) < 5e-08 
                || 
                calculate_distance(*(shm + row + (column +1)), checkVal) < 5e-08
            ){

                //sets offset to zero here, the calculate node function still does trig though.

                if(calculate_distance(*(shm + row + (column +1)), checkVal) < 5e-08){
                    g[column+1].x_offset = 0;
                } else if(calculate_distance(*(shm + row + (column -1)), checkVal) < 5e-08) {
                    g[column-1].x_offset = 0;
                }
                g[column].x_offset = 0;
                
            }
        }
    }

    return 0;
}

//distance between two nodes calc

double calculate_distance(cntNode a, cntNode b){
    return sqrt((pow((a.x - b.x), 2) + pow((a.y - b.y), 2)));
}

__device__ growthInfo 
calculate_new_node(cntNode a, cntNode b, growthInfo g){

    //trig is done with every iteration
    growthInfo newGrowthInfo;
    double x_0 = a.x;
    double y_0 = a.y;
    double x_1 = b.x;
    double y_1 = b.y;

    double theta = atan2(y_1, (x_1-x_0));

    double v = sqrt((pow((x_1 - x_0), 2) + pow((y_1 - y_0), 2)));

    double y_offset = v * sin(theta);

    double x_offset = v * cos(theta);
    
    //if offset is already 0 then can't change it, otherwise, set it again 
    //(it will always be the same value, but it is still recalculated every time)
    if(g.x_offset == 0)
        newGrowthInfo.x_offset = g.x_offset;
    else
        newGrowthInfo.x_offset = x_offset;

    newGrowthInfo.y_offset = y_offset;

    return newGrowthInfo;
}

//initalization and write functions

vector<cntNode> read_file(string filename, int N){
    vector<cntNode> cntVector;
    ifstream fs(filename);
    string line;
    int count = 0;

    if(!fs){
        cout << "File invalid or does not exist." << endl;
        return cntVector;
    }

    cout << "------------------------------" << endl;
    cout << "          Read Start          " << endl;
    cout << "------------------------------" << endl << endl;

    while(getline(fs, line)){
        istringstream ss(line);
        string substring;
        cntNode newNode;

        if(!line.empty()){
            getline(ss, substring, ',');
            newNode.x = stod(substring);
            
            getline(ss, substring, ',');
            newNode.y = stod(substring);

        }

        cntVector.push_back(newNode);

        count++;
    }

    fs.close();

    cout << "------------------------------" << endl;
    cout << "           Read End           " << endl;
    cout << "------------------------------" << endl << endl;

    return cntVector;
}

vector<growthInfo> get_growth_info(vector<cntNode> v, int N){

    cout << "------------------------------" << endl;
    cout << "       Growth Info Start      " << endl;
    cout << "------------------------------" << endl << endl;

    vector<growthInfo> rVector;

    //calculates initial angles, magninute and dx,dy
    const int size = v.size()-1;
    for(int i = N-1; i>=0; i--){
        growthInfo temp;
        double x_1 = v.at(size - (N + i)).x;
        double y_1 = v.at(size - (N + i)).y;

        double x_0 = v.at(size - i).x;
        double y_0 = v.at(size - i).y;
        
        temp.theta = atan2(y_1, (x_1-x_0));

        temp.v = sqrt((pow((x_1 - x_0), 2) + pow((y_1 - y_0), 2)));

        temp.y_offset = temp.v * sin(temp.theta);

        temp.x_offset = temp.v * cos(temp.theta);
        
        rVector.push_back(temp);
    }

    cout << "------------------------------" << endl;
    cout << "       Growth Info End        " << endl;
    cout << "------------------------------" << endl << endl;

    return rVector;
}

void write_to_file(const cntNode * shm, string inputFile, int C, int N){

    cout << "------------------------------" << endl;
    cout << "         Begin Write          " << endl;
    cout << "------------------------------" << endl << endl;

    stringstream ss(inputFile);
    string substring;
    string cntCount;
    while(getline(ss, substring, '/')){}
    stringstream newSS(substring);
    getline(newSS, cntCount, '_');
    string out_name = to_string(C) + "_" + cntCount + ".csv";
    ofstream out(out_name);

    for(int i = 0; i<C*N; i++){
        out << (*(shm + i)).x << "," << (*(shm + i)).y << endl;
        
    }

    out.close();

    cout << "------------------------------" << endl;
    cout << "         End Write            " << endl;
    cout << "------------------------------" << endl << endl;
}

//timing

std::chrono::time_point<std::chrono::steady_clock> get_time(){
    return std::chrono::steady_clock::now();
}

std::chrono::duration<double> calculate_elapsed_time(
    std::chrono::time_point<std::chrono::steady_clock> start,
    std::chrono::time_point<std::chrono::steady_clock> end
    ){
        return end - start;
}