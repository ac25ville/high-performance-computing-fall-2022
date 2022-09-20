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

int main(int agrc, char * argv[]){

    string filename = "../50_Coord.csv";
    int N = 50;
    int C = 50;

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

    // for(auto i:readVector){
    //     cout << "(" << i.x << ", " << i.y << ")" << " | " << "(COL: " << i.column << ", GEN: " << i.generation << ")" << endl;
    // }

    return 0;
}

vector<vector<cntNode>> grow_tubes(vector<cntNode> readVector, vector<growthInfo> infoVector, int N, int C){

    vector<vector<cntNode>> tubes;

    int count = 0;
    
    
    while((count/N)<C-1){
        // cout << (count / N) << endl;
        vector<cntNode> newGen;
        int generation;
        int init_bit = 0;
        if((int)tubes.size()==0 && init_bit==0){ //initial growth positions
            generation = C-1;
            cout << "init loop" << endl;
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

            cout 
            << "CNT #: " << column 
            << " | Theta: " << infoVector[column].theta << ", Magnitutde: " << infoVector[column].v 
            << " | X Offset: " << infoVector[column].x_offset << ", Y Offset: " << infoVector[column].y_offset << endl;

            // cout << "Tubes Size: " << tubes.size() << endl;

            if(infoVector[column].theta!=90){
                newNode.x = tubes[(count / N)][column].x + infoVector[column].x_offset;
            } else {
                newNode.x = tubes[(count / N) - 1][column].x + infoVector[column].x_offset;
            }

            // cout << "Made it past x calc" << endl;

            if(newGen.size() > 0 && newNode.x - newGen.at(newGen.size()-1).x == 5e-8){
                infoVector[column].theta = 90;
                infoVector[column-1].theta = 90;
                cout << "In corrections" << endl;
                newNode.x = tubes[(count / N) - 1][count % N].x + infoVector[column].x_offset;

            }

            // cout << "Made it past corrections" << endl;
            
            newNode.y = tubes[(count / N)][column].y + infoVector[column].y_offset;

            // cout << "(" << newNode.x << ", " << newNode.y << ")" << " | " << "(COL: " << newNode.column << ", GEN: " << newNode.generation << ")" << endl;

            newGen.push_back(newNode);

            count++;
        }
        if(init_bit==0){
            tubes.push_back(newGen);
        }
        generation--;
        // cout << endl << endl;
        
    }

    return tubes;

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