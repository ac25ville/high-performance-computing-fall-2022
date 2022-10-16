#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <chrono>
#include <ctime>
#include <thread>
#include <functional>
#include <mutex>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include <unistd.h>
#include <cstdlib>

using namespace std;

struct cntNode{
    double x;
    double y;
};

struct growthInfo{
    double x_offset;
    double y_offset;
    double theta;
    double v;
};

boost::mutex m;

vector<cntNode> read_file(string filename, int N);
vector<growthInfo> get_growth_info(vector<cntNode> v, int N);
void grow_tubes(vector<cntNode> readVector, cntNode* shm, growthInfo* g, int N, int C, int P, int start, int end, boost::barrier& bar);
int check_tubes(cntNode* shm, growthInfo* g, int start, int end, int N, int C, int gen);
double calculate_distance(cntNode a, cntNode b);
void write_to_file(const cntNode * shm, string inputFile, int C, int N);
growthInfo calculate_new_node(cntNode a, cntNode b, growthInfo g);
std::chrono::time_point<std::chrono::steady_clock> get_time();
std::chrono::duration<double> calculate_elapsed_time(std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::time_point<std::chrono::steady_clock> end);

int main(int argc, char * argv[]){

    std::chrono::time_point<std::chrono::steady_clock> totalStartTime;
    std::chrono::time_point<std::chrono::steady_clock> totalEndTime;

    totalStartTime = get_time();

    if(argc < 5){
        cout << "Invalid Argument Amount. Required Arguments & Format:" << endl 
        << "/path/to/homework2 /path/to/input_file N C P" << endl 
        << "N: Number of CNT in file, C: number of growth cycles, P: integer level of parallelism" << endl;
        return -1;
    }
    string filename = argv[1];
    int N = stoi(argv[2]);
    int C = stoi(argv[3]);
    int P = stoi(argv[4]);

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;

    double readTime = 0;
    double multiProcessTime = 0;
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
    cout << "      Multi Process Start     " << endl;
    cout << "------------------------------" << endl << endl;

    startTime = get_time();
    vector<thread> tg;
    int tCount;

    vector<cntNode> shm(C*N);
    cntNode * shmPointer = shm.data();

    boost::barrier bar(P);
    
    vector<growthInfo> growthInfoVector = get_growth_info(readVector, N);
    growthInfo * growthInfoPointer = growthInfoVector.data();

    int offset = N/P;
    int start;
    int end;
    

    for(tCount = 0; tCount<P; tCount++){
        start = offset*(tCount);
        end = offset*(tCount+1);
        if(end == (N-(N%offset)) && offset%N!=0){
            end+=N%offset;
        }
        tg.push_back(thread(grow_tubes, readVector, shmPointer, growthInfoPointer, N, C, P, start, end, std::ref(bar)));
    }

    cout << "Cleaning up..." << endl;
    //wait for threads


    for (vector<thread>::iterator it = tg.begin() ; it != tg.end(); ++it){
        it->join();
        cout << "Joined Thread" << endl;
    }

    endTime = get_time();
    multiProcessTime = calculate_elapsed_time(startTime, endTime).count();

    cout << "------------------------------" << endl;
    cout << "       Multi Process End      " << endl;
    cout << "------------------------------" << endl << endl;

    //write start
    startTime = get_time();
    
    write_to_file(shmPointer, filename, C, N);

    endTime = get_time();

    //write end
    writeTime = calculate_elapsed_time(startTime, endTime).count();

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

void grow_tubes(vector<cntNode> readVector, cntNode* shm, growthInfo* g, int N, int C, int P, int start, int end, boost::barrier& bar){

    if(start == 0){
        for(int i = 0; i<N; i++){
            std::cout << " Theta: " << (*(g+i)).theta << ", Magnitude: " << (*(g+i)).v << endl;
        }
    }

    int column;
    int row;

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
    
    for(int j = 0; j<C-1; j++){
        row = j * N;
        vector<cntNode> temp;
        for(column=start; column<end; column++){
            calculate_new_node((*(shm + column)), (*(shm + column + N)), (*(g+column))); //calculates trig everytime, serving the expressed intention of taking time as stated in class         
            cntNode newNode;

            //still adding x & y offset
            newNode.x = (*(shm + column + row + N)).x + (*(g+column)).x_offset; 
            newNode.y = (*(shm + column + row + N)).y + (*(g+column)).y_offset;

            temp.push_back(newNode);
        }
        if((row+start+(N*2))<C*N)
            std::copy(temp.begin(),temp.end(), shm+(row+start+(N*2)));
        
        bar.wait();
        boost::lock_guard<boost::mutex> locker(m);      
        check_tubes(shm, g, start, end, N, C, j);

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

double calculate_distance(cntNode a, cntNode b){
    return sqrt((pow((a.x - b.x), 2) + pow((a.y - b.y), 2)));
}

growthInfo calculate_new_node(cntNode a, cntNode b, growthInfo g){
    growthInfo newGrowthInfo;
    double x_0 = a.x;
    double y_0 = a.y;
    double x_1 = b.x;
    double y_1 = b.y;

    double theta = atan2(y_1, (x_1-x_0));

    double v = sqrt((pow((x_1 - x_0), 2) + pow((y_1 - y_0), 2)));

    double y_offset = v * sin(theta);

    double x_offset = v * cos(theta);

    newGrowthInfo.x_offset = x_offset;

    newGrowthInfo.y_offset = y_offset;

    newGrowthInfo.x_offset = g.x_offset;

    newGrowthInfo.y_offset = g.y_offset;

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

        cout << temp.x_offset << " " << temp.y_offset << endl;
        
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