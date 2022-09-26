#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

struct cntNode{
    double x;
    double y;
    int generation;
    int column;
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
vector<growthInfo> check_tubes(vector<vector<cntNode>> tubes, vector<growthInfo> infoVector, int N, int C);
double calculate_distance(cntNode a, cntNode b);
void write_to_file(vector<vector<cntNode>> tubes, string inputFile, int C);

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
    // int P = stoi(argv[4]);

    vector<cntNode> readVector = read_file(filename, N);

    if(readVector.empty()){
        cout << "readVector empty" << endl;
        return -1;
    }

    vector<growthInfo> growthInfoVector = get_growth_info(readVector, N);

    if(growthInfoVector.empty()){
        cout << "growthInfoVector empty" << endl;
        return -1;
    }

    vector<vector<cntNode>> tubes = grow_tubes(readVector, growthInfoVector, N, C);

    // for(auto j:tubes){
    //     cout << endl << endl;
    //     for(auto i:j){
    //         cout << "(" << i.x << ", " << i.y << ")" << " | " << "(COL: " << i.column << ", GEN: " << i.generation << ")" << endl;
    //     }
    // }

    write_to_file(tubes, filename, C);

    // for(auto i:readVector){
    //     cout << "(" << i.x << ", " << i.y << ")" << " | " << "(COL: " << i.column << ", GEN: " << i.generation << ")" << endl;
    // }

    return 0;
}



vector<vector<cntNode>> grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C){

    vector<vector<cntNode>> tubes;
    vector<growthInfo> tempInfoVector = infoVector;

    int count = 0;

    if(count == 0){
        for(auto i:infoVector){
            cout 
            << " Theta: " << i.theta << ", Magnitutde: " << i.v << endl;
            // << " X Offset: " << i.x_offset << ", Y Offset: " << i.y_offset << endl;
        }
    }
    
    
    while((count/N)<C-1){
        // cout << (count / N) << endl;
        vector<cntNode> newGen;
        int generation;
        int init_bit = 0;
        if((int)tubes.size()==0 && init_bit==0){ //initial growth positions
            generation = C-1;
            // cout << "init loop" << endl;
            for(int i=0; i<N; i++){
                cntNode newNode;
                newNode.x = readVector[readVector.size()-N+i].x;
                newNode.y = readVector[readVector.size()-N+i].y;
                newNode.column = readVector[readVector.size()-N+i].column;
                newNode.generation = generation;

                // cout << "(" << newNode.x << ", " << newNode.y << ")" << " | " << "(COL: " << newNode.column << ", GEN: " << newNode.generation << ")" << endl;

                newGen.push_back(newNode);
                
            }
            init_bit = 1;
            tubes.push_back(newGen);      
        }
        int column;
        // cout << newGen.size() << endl;
        while((int)newGen.size()<N){ //growth beyond intial positions

            cntNode newNode;

            column = count % N;
            // int previousRow = count / N;
            
            newNode.column = column;
            newNode.generation = generation;

            // cout 
            // << "CNT #: " << column 
            // << " | Theta: " << tempInfoVector[column].theta << ", Magnitutde: " << tempInfoVector[column].v 
            // << " | X Offset: " << tempInfoVector[column].x_offset << ", Y Offset: " << tempInfoVector[column].y_offset << endl;
            

            // cout << "Tubes Size: " << tubes.size() << endl;
            newNode.x = tubes[(count / N)][column].x + tempInfoVector[column].x_offset;

            // cout << "Made it past x calc" << endl;
            
            newNode.y = tubes[(count / N)][column].y + tempInfoVector[column].y_offset;

            // cout << "(" << newNode.x << ", " << newNode.y << ")" << " | " << "(COL: " << newNode.column << ", GEN: " << newNode.generation << ")" << endl;

            newGen.push_back(newNode);

            count++;
        }
        if(init_bit==0){
            tubes.push_back(newGen);
            tempInfoVector = check_tubes(tubes, tempInfoVector, N, C);
        }
        generation--;
        // cout << endl << endl;
        
    }

    return tubes;

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

void write_to_file(vector<vector<cntNode>> tubes, string inputFile, int C){
    stringstream ss(inputFile);
    string substring;
    while(getline(ss, substring, '/')){}
    string out_name = to_string(C) +"_"+substring;
    // cout << out_name << endl;
    ofstream out(out_name);

    for(auto gen:tubes){
        for(auto i:gen){
            out << i.x << ",";
            out << i.y << ",";
            out << i.column << ",";
            out << i.generation << endl;
        }
    }

    out.close();
}