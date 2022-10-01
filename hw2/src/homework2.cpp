#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string.h>

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

vector<cntNode> read_file(string filename, int N);
vector<growthInfo> get_growth_info(vector<cntNode> v, int N);
vector<vector<cntNode>> grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C);
int grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C, int start, int end, shmCNTNode shm[], int process);
vector<growthInfo> check_tubes(vector<vector<cntNode>> tubes, vector<growthInfo> infoVector, int N, int C);
double calculate_distance(cntNode a, cntNode b);
void write_to_file(shmCNTNode shm[], string inputFile, int C, int N);

int main(int argc, char * argv[]){

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

    vector<cntNode> readVector = read_file(filename, N);

    if(readVector.empty()){
        cout << "readVector empty" << endl;
        return 1;
    }

    vector<growthInfo> growthInfoVector = get_growth_info(readVector, N);

    if(growthInfoVector.empty()){
        cout << "growthInfoVector empty" << endl;
        return 1;
    }

    // int semId;
    // key_t semKey = 123789;
    // int semFlag = IPC_CREAT | 0666;

    // int semCount = 1;
    // int numOps = 1;

    // if((semId = semget(semKey, semCount, semFlag)) == -1){
    //     cerr << "Failed to semget(" << semKey << "," << semCount << "," << semFlag << ")" << endl;
    //     exit(1);
    // } else {
    //     cout << "Successful semget resulted in (" << semId << endl;
    // }

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

    int pCounter;
    int growTubesCheck = 0;
    pid_t pid;

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
        growTubesCheck = grow_tubes(readVector, growthInfoVector, N, C, start, end, shm, pCounter);
        _exit(0);
    }

    if(growTubesCheck == 1){
        cerr << "Grow Tubes Failure!!" << endl;
        exit(1);
    }

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

    /**
     * SHM = malloc(N*C*sizeof(shmNodes));
     * pid_t pid;
     * int i; //preserves i b/c copy stack
     * for(i = 0; i < P && pid > 0; i++){
     *      pid = fork();
     * }
     * 
     * 
     * 
     * if(pid == 0){
     *  size_t offset = N/P;
     *  
     *  grow_tubes(readVector, infoVector, start = offset*i, end = offset*(i+1)-1, int C, SHM, semaphore);
     *      check_tubes(SHM, N , C);
     * }
    */

    // for(int i = 0; i<N*C; i++){
    //         cout << "(" << shm[i].x << ", " << shm[i].y << ")" << " | " << "dx: " << shm[i].x_offset << endl;
    // }

    write_to_file(shm, filename, C, N);

    // for(auto i:readVector){
    //     cout << "(" << i.x << ", " << i.y << ")" << " | " << "(COL: " << i.column << ", GEN: " << i.generation << ")" << endl;
    // }

    //detach shm, then IPC key to delete

    shmdt(shm);
    shmctl(shmId, IPC_RMID, 0);

    return 0;
}



int grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C, int start, int end, shmCNTNode shm[], int process){

    if(start == 0){
        for(auto i:infoVector){
            std::cout << " Theta: " << i.theta << ", X_offset: " << i.x_offset << endl;
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
            newNode.x = shm[row + column].x + shm[row + column].x_offset;
            newNode.y = shm[row + column].y + infoVector.at(column).y_offset;
            newNode.x_offset = shm[row + column].x_offset;

            temp.push_back(newNode);
        }
        std::copy(temp.begin(),temp.end(), shm+(row+start+N));
    }
    
    return 0;

}

vector<growthInfo> check_tubes(vector<vector<cntNode>> tubes, vector<growthInfo> infoVector, int N, int C){

    vector<growthInfo> temp = infoVector;
    const int tubes_size = (int)tubes.size();

    for(int i = 0; i<N; i++){
        for(int j = 0; j<tubes_size; j++){
            if(calculate_distance(tubes[j][i+1], tubes[tubes_size-1][i]) < 5e-08 || calculate_distance(tubes[j][i-1], tubes[tubes_size-1][i]) < 5e-08){
                temp[i].x_offset = 0;

                if(calculate_distance(tubes[j][i+1], tubes[tubes_size-1][i]) < 5e-08){
                    temp[i+1].x_offset = 0;
                } else {
                    temp[i-1].x_offset = 0;
                }
            }
        }
    }

    return temp;
}

double calculate_distance(cntNode a, cntNode b){
    return sqrt((pow((a.x - b.x), 2) + pow((a.y - b.y), 2)));
}

vector<growthInfo> get_growth_info(vector<cntNode> v, int N){

    vector<growthInfo> rVector;

    const int size = v.size()-1;
    for(int i = N-1; i>=0; i--){
        growthInfo temp;
        double x_1 = v[size - (i+N)].x;
        double y_1 = v[size - (i+N)].y;

        double x_0 = v[size - (i)].x;
        double y_0 = v[size - (i)].y;
        // std::cout << "botttom: ";
        // std::cout << size << " | " << x_0 << ", " << y_0 << endl;
        temp.theta = atan2(y_1, abs(x_1-x_0));

        temp.v = sqrt((pow((x_1 - x_0), 2) + pow((y_1 - y_0), 2)));

        temp.y_offset = temp.v * sin(temp.theta);

        temp.x_offset = temp.v * cos(temp.theta);

        rVector.push_back(temp);
    }

    return rVector;
}

vector<cntNode> read_file(string filename, int N){
    vector<cntNode> cntVector;
    ifstream fs(filename);
    string line;
    int count = 0;

    if(!fs){
        cout << "File invalid or does not exist." << endl;
        return cntVector;
    }

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

    return cntVector;
}

void write_to_file(shmCNTNode shm[], string inputFile, int C, int N){
    stringstream ss(inputFile);
    string substring;
    string cntCount;
    while(getline(ss, substring, '/')){}
    stringstream newSS(substring);
    getline(newSS, cntCount, '_');
    string out_name = to_string(C) + "_" + cntCount + ".csv";
    // cout << out_name << endl;
    ofstream out(out_name);

    for(int i = 0; i<C*N; i++){
        out << shm[i].x << "," << shm[i].y << endl;
        
    }

    out.close();
}