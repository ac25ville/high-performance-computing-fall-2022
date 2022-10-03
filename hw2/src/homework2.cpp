#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>
#include <chrono>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

struct cntNode{
    double x;
    double y;
    int generation;
    int column;
};

struct shmCNTNode{
    double x;
    double y;
    double x_offset;
};

struct growthInfo{
    double theta; //angle of vector growth
    double v; //magnitutde of vector growth
    double x_offset;
    double y_offset;
};

union senum {
    int val;
    struct semid_ds *buf;
    ushort * array;
} argument;

vector<cntNode> read_file(string filename, int N);
vector<growthInfo> get_growth_info(vector<cntNode> v, int N);
vector<vector<cntNode>> grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C);
double grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C, int P, int start, int end, shmCNTNode shm[], sembuf operations[], int process, int semId);
double check_tubes(int start, int end, shmCNTNode shm[], int N, int C, int gen);
double calculate_distance(shmCNTNode a, shmCNTNode b);
void write_to_file(shmCNTNode shm[], string inputFile, int C, int N);
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
    double growthInfoTime = 0;
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

    //growth info collection start
    startTime = get_time();

    vector<growthInfo> growthInfoVector = get_growth_info(readVector, N);

    endTime = get_time();

    growthInfoTime = calculate_elapsed_time(startTime, endTime).count();

    //growth info collection end

    //error check growth info vector
    if(growthInfoVector.empty()){
        cout << "growthInfoVector empty" << endl;
        return 1;
    }

    //semaphore creation start

    int semId;
    key_t semKey = 123789;
    int semFlag = IPC_CREAT | 0666;

    int semCount = 1;

    if((semId = semget(semKey, semCount, semFlag)) == -1){
        cerr << "Failed to semget(" << semKey << "," << semCount << "," << semFlag << ")" << endl;
        exit(1);
    } else {
        cout << "Successful semget resulted in (" << semId << endl;
    }

    argument.val = 0;
    if( semctl(semId, 0, SETVAL, argument) < 0){
		std::cerr << "Init: Failed to initialize (" << semId << ")" << std::endl; 
		exit(1);
	}
	else{
		std::cout << "Init: Initialized (" << semId << ")" << std::endl; 
	}

    struct sembuf operations[2];

    operations[0].sem_num = 0;
    operations[0].sem_op = 1;
    operations[0].sem_flg = 0;

    operations[1].sem_num = 0;
    operations[1].sem_op = -1;
    operations[1].sem_flg = 0;
    //semaphore creation end

    //shared memory start

    int shmId; 			
	key_t shmKey = 123460; 		
	int shmFlag = IPC_CREAT | 0666; 
	
	shmCNTNode * shm;

    if((shmId = shmget(shmKey, sizeof(shmCNTNode) * N * C, shmFlag)) < 0){
        cerr << "Init: Failed to initialize shared memory (" << shmId << ")" << endl; 
		exit(1);
    }

    if ((shm = (shmCNTNode *)shmat(shmId, NULL, 0)) == (shmCNTNode *) -1){
		cerr << "Init: Failed to attach shared memory (" << shmId << ")" << endl; 
		exit(1);
	}

    //shared memory end    

    //process creation start

    cout << "------------------------------" << endl;
    cout << "      Multi Process Start     " << endl;
    cout << "------------------------------" << endl << endl;

    int pCounter;
    pid_t pid;
    

    startTime = get_time();
    for(pCounter=0; pCounter<P && pid != 0; pCounter++){
        pid = fork();
    }

    if(pid < 0){
        cerr << "Could not fork!!! ("<< pid <<")" << endl;
		exit(1);
    }

    
    if(pid == 0){
        int offset = N/P;
        int start = offset*(pCounter-1);
        int end = offset*pCounter;
        
        if(end == (N-(N%offset)) && offset%N!=0){
            end+=N%offset;
        }
        grow_tubes(readVector, growthInfoVector, N, C, P, start, end, shm, operations, pCounter, semId);
        _exit(0);
    }
    

    //process creation end

    int status;	// catch the status of the child

	do  // in reality, mulptiple signals or exit status could come from the child
	{

		pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (w == -1)
		{
			std::cerr << "Error waiting for child process ("<< pid <<")" << std::endl;
			break;
		}
		
		if (WIFEXITED(status))
		{
			if (status > 0)
			{
				cerr << "Child process ("<< pid <<") exited with non-zero status of " << WEXITSTATUS(status) << endl;
				continue;
			}
			else
			{
				cout << "Child process ("<< pid <<") exited with status of " << WEXITSTATUS(status) << endl;
				continue;
			}
		}
		else if (WIFSIGNALED(status))
		{
			cout << "Child process ("<< pid <<") killed by signal (" << WTERMSIG(status) << ")" << endl;
			continue;			
		}
		else if (WIFSTOPPED(status))
		{
			cout << "Child process ("<< pid <<") stopped by signal (" << WSTOPSIG(status) << ")" << endl;
			continue;			
		}
		else if (WIFCONTINUED(status))
		{
			cout << "Child process ("<< pid <<") continued" << endl;
			continue;
		}
	}
	while (!WIFEXITED(status) && !WIFSIGNALED(status));

    endTime = get_time();
    multiProcessTime = calculate_elapsed_time(startTime, endTime).count();

    cout << "------------------------------" << endl;
    cout << "       Multi Process End      " << endl;
    cout << "------------------------------" << endl << endl;

    //write start
    startTime = get_time();
    
    write_to_file(shm, filename, C, N);

    endTime = get_time();

    //write end
    writeTime = calculate_elapsed_time(startTime, endTime).count();

    //free shared memory, detach, then ctl
    shmdt(shm);
    shmctl(shmId, IPC_RMID, 0);

    semctl(semId, IPC_RMID, 0);

    totalEndTime = get_time();
    totalTime = calculate_elapsed_time(totalStartTime, totalEndTime).count();

    cout << endl;
    cout << "--------------------------------------------" << endl;
    cout << "              Timing Summary                " << endl;
    cout << "--------------------------------------------" << endl;
    cout << " Total              | " << totalTime   << "s" << endl;
    cout << " Read               | " << readTime    << "s" << endl;
    cout << " Growth Info        | " << growthInfoTime << "s" << endl;
    cout << " Multi Proc Portion | " << multiProcessTime  << "s" << endl;
    cout << " Write              | " << writeTime   << "s" << endl;
    cout << endl;

    return 0;
}

//grow and check

double grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C, int P, int start, int end, shmCNTNode shm[], sembuf operations[], int process, int semId){

    if(start == 0){
        for(auto i:infoVector){
            std::cout << " Theta: " << i.theta << ", Magnitude: " << i.v << endl;
        }
    }

    vector<shmCNTNode> initTemp;
    int readVectorSize = (int)readVector.size();
    for(int i=start; i<end; i++){
        shmCNTNode newNode;
        newNode.x = readVector.at(readVectorSize-N+i).x;
        newNode.y = readVector.at(readVectorSize-N+i).y;
        newNode.x_offset = infoVector.at(i).x_offset;
        

        initTemp.push_back(newNode);
    }
    std::copy(initTemp.begin(),initTemp.end(), shm + start);

    int column;
    int row;
    for(int j = 0; j<C; j++){
        row = j * N;
        vector<shmCNTNode> temp;
        for(column=start; column<end; column++){
            shmCNTNode newNode;
            newNode.x = (*(shm + row + column)).x + (*(shm + row + column)).x_offset;
            newNode.y = (*(shm + row + column)).y + infoVector.at(column).y_offset;
            newNode.x_offset = (*(shm + row + column)).x_offset;

            temp.push_back(newNode);
        }
        std::copy(temp.begin(),temp.end(), shm+(row+start+N));

        while(semctl(semId, 0, GETVAL, argument) !=0){}
            semop(semId, (operations+0), 1);
               
        check_tubes(start, end, shm, N, C, j);
            semop(semId, (operations+1), P);
    }

    return 0;

}

double check_tubes(int start, int end, shmCNTNode shm[], int N, int C, int gen){

    int row;
    int column;
    for(column = start; column<end; column++){
        for(int j = 1; j<gen+1; j++){
            shmCNTNode checkVal = *(shm + ((gen+1) * N) + column);
            row = j * N;
            if(
                calculate_distance(*(shm + row + (column -1)), checkVal) < 5e-08 
                || 
                calculate_distance(*(shm + row + (column +1)), checkVal) < 5e-08
            ){

                if(calculate_distance(*(shm + row + (column +1)), checkVal) < 5e-08){
                    shm[((gen+1) * N) + column+1].x_offset = 0;
                } else {
                    shm[((gen+1) * N) + column-1].x_offset = 0;
                }
                shm[((gen+1) * N) + column].x_offset = 0;
                
            }
        }
    }

    return 0;
}

double calculate_distance(shmCNTNode a, shmCNTNode b){
    return sqrt((pow((a.x - b.x), 2) + pow((a.y - b.y), 2)));
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

            newNode.column = count % N;
            newNode.generation = count / N;

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
        
        rVector.push_back(temp);
    }

    cout << "------------------------------" << endl;
    cout << "       Growth Info End        " << endl;
    cout << "------------------------------" << endl << endl;

    return rVector;
}

void write_to_file(shmCNTNode shm[], string inputFile, int C, int N){

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