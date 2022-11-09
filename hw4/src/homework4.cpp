#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <chrono>
#include <ctime>
#include <mpi.h>

#include <unistd.h>
#include <cstdlib>

using namespace std;

namespace acc9cm {
    MPI_Datatype initPackageType;
    MPI_Datatype cntNodeType;
    MPI_Datatype growthInfoType;

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

    struct initPackage {
        vector<acc9cm::cntNode> readVector;
        vector<acc9cm::growthInfo> g;
    };
}



vector<acc9cm::cntNode> read_file(string filename, int N);
vector<acc9cm::growthInfo> get_growth_info(vector<acc9cm::cntNode> v, int N);
void grow_tubes(vector<acc9cm::cntNode> readVector, acc9cm::cntNode* shm, acc9cm::growthInfo* g, int N, int C, int P, int start, int end);
int check_tubes(acc9cm::cntNode* shm, acc9cm::growthInfo* g, int start, int end, int N, int C, int gen);
double calculate_distance(acc9cm::cntNode a, acc9cm::cntNode b);
void write_to_file(const acc9cm::cntNode * shm, string inputFile, int C, int N);
acc9cm::growthInfo calculate_new_node(acc9cm::cntNode a, acc9cm::cntNode b, acc9cm::growthInfo g);
std::chrono::time_point<std::chrono::steady_clock> get_time();
std::chrono::duration<double> calculate_elapsed_time(std::chrono::time_point<std::chrono::steady_clock> start, std::chrono::time_point<std::chrono::steady_clock> end);

int main(int argc, char * argv[]){

    if(argc < 5){ // check for arguments, give hopefully helpful feedback
        cout << "Invalid Argument Amount. Required Arguments & Format:" << endl 
        << "/path/to/homework2 /path/to/input_file N C P" << endl 
        << "N: Number of CNT in file, C: number of growth cycles, P: integer level of parallelism" << endl;
        return -1;
    } 

    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //start and end time for each subsection of the program
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> endTime;

    string filename = argv[1];
    int N = stoi(argv[2]);
    int C = stoi(argv[3]);
    int P = stoi(argv[4]);

    if (rank == 0){
        std::chrono::time_point<std::chrono::steady_clock> totalStartTime;
        std::chrono::time_point<std::chrono::steady_clock> totalEndTime;

        totalStartTime = get_time(); //start timer

        //argument reads and conversions

        // type creation
        
        int blockcountCntNode[2] = {1,1};
        MPI_Aint offsetsCntNode[2] = {offsetof(acc9cm::cntNode, x), offsetof(acc9cm::cntNode, y)};
        MPI_Datatype dataTypeCntNode[2] = {MPI_DOUBLE, MPI_DOUBLE};
        MPI_Type_create_struct(2, blockcountCntNode, offsetsCntNode, dataTypeCntNode, &(acc9cm::cntNodeType));
        MPI_Type_commit(&(acc9cm::cntNodeType));

        int blockcountGrowthInfo[4] = {1,1,1,1};
        MPI_Aint offsetsGrowthInfo[4] = {offsetof(acc9cm::growthInfo, x_offset), offsetof(acc9cm::growthInfo, y_offset), offsetof(acc9cm::growthInfo, theta), offsetof(acc9cm::growthInfo, v)};
        MPI_Datatype dataTypeGrowthInfo[4] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
        MPI_Type_create_struct(4, blockcountGrowthInfo, offsetsGrowthInfo, dataTypeGrowthInfo, &(acc9cm::growthInfoType));
        MPI_Type_commit(&(acc9cm::growthInfoType));

        int blockcount[2] = {1,1};
        MPI_Aint offsets[2] = {offsetof(acc9cm::initPackage, readVector), offsetof(acc9cm::initPackage, g)};
        MPI_Datatype dataType[2] = {acc9cm::cntNodeType, acc9cm::growthInfoType};
        MPI_Type_create_struct(2, blockcount, offsets, dataType, &(acc9cm::initPackageType));
        MPI_Type_commit(&(acc9cm::initPackageType));

        //declare doubles to record times
        double readTime = 0;
        double writeTime = 0;
        double totalTime = 0;
        double multiProcessTime = 0;

        //read start

        startTime = get_time();

        vector<acc9cm::cntNode> readVector = read_file(filename, N);

        endTime = get_time();

        readTime = calculate_elapsed_time(startTime, endTime).count();

        //read end

        //error check read vector
        if(readVector.empty()){
            cout << "readVector empty" << endl;
            return 1;
        }

        //growthInfo calc start

        vector<acc9cm::growthInfo> growthInfoVector = get_growth_info(readVector, N);

        //growthInfo calc end

        //error check read vector
        if(growthInfoVector.empty()){
            cout << "growthInfoVector empty" << endl;
            return 1;
        }

        acc9cm::initPackage package;
        package.readVector = readVector;

        package.g = growthInfoVector; //get the intial growth data

        int nodeCount;

        int offset = N/P;
        int start;
        // int end;  
        startTime = get_time();
        for(nodeCount = 0; nodeCount<P; nodeCount++){
            start = offset*(nodeCount);
            MPI_Send(&package,		        /* message buffer */
		    1,                             /* buffer size */
		    acc9cm::initPackageType,		/* data item is an integer */
		    rank,	                        /* destination process rank */
		    start,	                        /* user chosen message tag */
		    MPI_COMM_WORLD);	            /* default communicator */
        }

        MPI_Barrier(MPI_COMM_WORLD);

        endTime = get_time();
        multiProcessTime = calculate_elapsed_time(startTime, endTime).count();

        //write start
        // startTime = get_time();
        
        // write_to_file(shmPointer, filename, C, N);

        // endTime = get_time();

        // //write end
        // writeTime = calculate_elapsed_time(startTime, endTime).count();

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
    } else {

        cout << "------------------------------" << endl;
        cout << "      Multi Process Start     " << endl;
        cout << "------------------------------" << endl << endl;

        vector<acc9cm::cntNode> shm(C*N);
        acc9cm::cntNode * shmPointer = shm.data(); //pointer to pre-allocated vector, to avoid resizing issues
        
        // vector<acc9cm::growthInfo> growthInfoVector = get_growth_info(readVector, N); //get the intial growth data
        

        acc9cm::initPackage recvPackage;

        int offset = N/P;
        int start;
        int end;
        int nodeCount;

        for(nodeCount = 0; nodeCount<P; nodeCount++){
            //calculations as in previous assignment, but just with nodeCount instead of pCount or tCount
            MPI_Status status;

            MPI_Recv(&recvPackage,	/* message buffer           */
		    1,                      /* buffer size              */
		    acc9cm::initPackageType,/* data item is an integer  */
		    MPI_ANY_SOURCE,	        /* destination process rank */
		    MPI_ANY_TAG,	        /* user chosen message tag  */
		    MPI_COMM_WORLD,         /* default communicator     */
            &status);        

            acc9cm::growthInfo * g = recvPackage.g.data(); //pointer to vector

            start = offset*(nodeCount);
            int sourceCaught = status.MPI_SOURCE;
            end = offset*(nodeCount+1);
            if(end == (N-(N%offset)) && offset%N!=0){
                end+=N%offset;
            }

            grow_tubes(recvPackage.readVector, shmPointer, g, N, C, P, start, end);

            MPI_Send(&shm,		/* message buffer */
		    shm.size(),            /* buffer size */
		    acc9cm::cntNodeType,		/* data item is an integer */
		    sourceCaught,	/* destination process rank */
		    start,	/* user chosen message tag */
		    MPI_COMM_WORLD);	/* default communicator */
        }

        

        cout << "------------------------------" << endl;
        cout << "       Multi Process End      " << endl;
        cout << "------------------------------" << endl << endl;
    }
    
    MPI_Finalize();

    return 0;
}

//grow and check

void grow_tubes(vector<acc9cm::cntNode> readVector, acc9cm::cntNode* shm, acc9cm::growthInfo* g, int N, int C, int P, int start, int end){

    //print inital values as asked for previously, decided not to change this functionality since it oucld be useful
    if(start == 0){
        for(int i = 0; i<N; i++){
            std::cout << " Theta: " << (*(g+i)).theta << ", Magnitude: " << (*(g+i)).v << endl;
        }
    }

    int column;
    int row;

    for(int j=0; j<2; j++){
        vector<acc9cm::cntNode> initTemp;
        int readVectorSize = (int)readVector.size();
        row = j * N;
        for(int column=start; column<end; column++){
            acc9cm::cntNode newNode;
            newNode.x = readVector.at(readVectorSize-(N+row)+column).x;
            newNode.y = readVector.at(readVectorSize-(N+row)+column).y;

            initTemp.push_back(newNode);
        }
        std::copy(initTemp.begin(),initTemp.end(), shm + (start+row));
    }
    
    for(int j = 0; j<C-1; j++){
        row = j * N;
        vector<acc9cm::cntNode> temp;
        for(column=start; column<end; column++){
            //calculates trig everytime, serving the expressed intention of taking time as stated in class 
            //still returns offsets since addition is simpler logically than returning a new node (at least for me)
            g[column] = calculate_new_node((*(shm + column)), (*(shm + column + N)), (*(g+column))); 
                    
            acc9cm::cntNode newNode;

            //still adding x & y offset
            newNode.x = (*(shm + column + row + N)).x + (*(g+column)).x_offset; 
            newNode.y = (*(shm + column + row + N)).y + (*(g+column)).y_offset;

            temp.push_back(newNode);
        }
        if((row+start+(N*2))<C*N) //to avoid seg fault; only need to insert above the 2 previous
            std::copy(temp.begin(),temp.end(), shm+(row+start+(N*2)));
          
        check_tubes(shm, g, start, end, N, C, j); //check tubes

    }

}

int check_tubes(acc9cm::cntNode* shm, acc9cm::growthInfo* g, int start, int end, int N, int C, int gen){

    int row;
    int column;
    for(column = start; column<end; column++){
        for(int j = 1; j<gen+1; j++){
            acc9cm::cntNode checkVal = *(shm + ((gen+1) * N) + column);
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

double calculate_distance(acc9cm::cntNode a, acc9cm::cntNode b){
    return sqrt((pow((a.x - b.x), 2) + pow((a.y - b.y), 2)));
}

acc9cm::growthInfo calculate_new_node(acc9cm::cntNode a, acc9cm::cntNode b, acc9cm::growthInfo g){

    //trig is done with every iteration
    acc9cm::growthInfo newGrowthInfo;
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

vector<acc9cm::cntNode> read_file(string filename, int N){
    vector<acc9cm::cntNode> cntVector;
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
        acc9cm::cntNode newNode;

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

vector<acc9cm::growthInfo> get_growth_info(vector<acc9cm::cntNode> v, int N){

    cout << "------------------------------" << endl;
    cout << "       Growth Info Start      " << endl;
    cout << "------------------------------" << endl << endl;

    vector<acc9cm::growthInfo> rVector;

    //calculates initial angles, magninute and dx,dy
    const int size = v.size()-1;
    for(int i = N-1; i>=0; i--){
        acc9cm::growthInfo temp;
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

void write_to_file(const acc9cm::cntNode * shm, string inputFile, int C, int N){

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