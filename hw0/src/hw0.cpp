#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

//file functions
int U_file(string filepath);
int node_coords(string filename, int N);
int K_file(string filename);

//helper functions
double compute_sum(vector<vector<double>> stiffness_matrix);
vector<double> compute_sum_rows(vector<vector<double>> stiffness_matrix);
vector<double> compute_sum_columns(vector<vector<double>> stiffness_matrix);
double compute_row(vector<double> matrix_row);
double compute_column(vector<vector<double>> stiffness_matrix, int column_number);

//custom data types

struct UObj{
    double x;
    double y;
    double theta;
};

struct NodeCoords{
    double x;
    double y;
    int row;
    int column;
};

int main(int argc, char * argv[]){

    if(argc<5){
        cout << "REQUIRED ARGUMENTS: hw0/homework_0 /path/to/U.csv /path/to/nodeCoordinates.csv /path/to/K.csv N" << endl ;
        return -1;
    }

    cout << argc << endl;

    U_file(argv[1]);

    node_coords(argv[2], stoi(argv[4]));

    K_file(argv[3]);

    return 0;
}

//start of file functions

int U_file(string filepath){
    vector<UObj> vector;
    ifstream fs(filepath);
    double x,y,theta;

    cout << endl << "1) U.csv" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;

    if(!fs){
        cout << endl << "Invalid file or filepath" << endl;
        return -1;
    }

    while(fs >> x >> y >> theta){
        UObj temp;
        
        temp.x = x;
        temp.y = y;
        temp.theta = theta;

        vector.push_back(temp);
        
    }

    fs.close();

    //write to file

    ofstream out("displacements.csv");

    for(auto i:vector){
        out << i.x << ",";
        out << i.y << ",";
        out << i.theta << endl;
    }

    out.close();

    

    cout << "I used a vector of structs. Each struct has an x, y, and theta value in it." << endl;
    cout << "The number of nodes/structs I had in my vector was " << vector.size() << endl;
    cout << "Lines Parsed: " << vector.size() << endl;

    return vector.size();
}

int node_coords(string filename, int N){
    vector<NodeCoords> vector;
    ifstream fs(filename);
    string line;
    int column_count = 0;
    int row_count = 0;

    cout << endl << "2) nodeCoordinates.csv" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;

    if(!fs){
        cout << endl << "Invalid file or filepath" << endl;
        return -1;
    }
    
    while(getline(fs, line)){
        NodeCoords temp;
        istringstream ss(line);
        string token;
        getline(ss, token, ',');
        temp.x = stod(token);

        getline(ss, token, ',');
        temp.y = stod(token);

        temp.row = row_count;
        temp.column = column_count;
        
        vector.push_back(temp);

        column_count++;
        if(column_count == N){
            column_count = 0;
            row_count++;
        }
    }

    int count = 0;
    
    for(auto i:vector){
        cout << "(X: " << i.x << ", Y: " << i.y << ") | (ROW: " << i.row << ", COLUMN:" << i.column << ")" << endl;
        count++;
        if(count % N == 0){
            cout << endl;
        }
    }

    cout << "I used a vector of structs. Each struct has an x, y, value in it and a row, column value in it." << endl;
    cout << "The number of nodes/structs I had in my vector was " << vector.size() << endl;
    cout << "Lines Parsed: " << vector.size() << endl;
    
    return vector.size();
}

int K_file(string filename){
    vector<vector<double>> stiffness_matrix;

    ifstream fs(filename);
    string line;

    cout << endl << "3) K.csv" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;

    if(!fs){
        cout << endl << "Invalid file or filepath" << endl;
        return -1;
    }

    while(getline(fs, line)){
        vector<double> temp;
        istringstream ss(line);
        string substring;

        if(!line.empty()){
            while(getline(ss, substring, ',')){
            temp.push_back(stod(substring));
        }
        }
        stiffness_matrix.push_back(temp);
    }

    fs.close();

    double sum_of_all = compute_sum(stiffness_matrix);

    vector<double> rowSums = compute_sum_rows(stiffness_matrix);

    vector<double> columnSums = compute_sum_columns(stiffness_matrix);

    int count = 0;

    cout << "Sum of all elements: " << sum_of_all << endl;

    cout << endl << "Rows" << endl;
    for(auto i:rowSums){
        cout << "Sum of row " << count << ": " << i << endl;
        count++;
    }
    
    count = 0;

    cout << endl << "Columns:" << endl;
    for(auto i:columnSums){
        cout << "Sum of column " << count << ": " << i << endl;
        count++;
    }

    cout << endl << "I used a vector of vector<double>, i.e. a 2D vector." << endl;
    cout << "Stiffness Matrix size: " << stiffness_matrix.size() << " rows and columns." << endl;
    cout << "Lines Parsed: " << stiffness_matrix.size() << endl;

    return stiffness_matrix.size();
}

//start of helper functions

double compute_sum(vector<vector<double>> stiffness_matrix){
    double total = 0;
    for(auto i:stiffness_matrix){
        total+=compute_row(i);
    }
    return total;
}

vector<double> compute_sum_rows(vector<vector<double>> stiffness_matrix){
    vector<double> rowSums;

    for(auto i:stiffness_matrix){
        rowSums.push_back(compute_row(i));
    }
    return rowSums;
}

vector<double> compute_sum_columns(vector<vector<double>> stiffness_matrix){
    vector<double> columnSums;
    int column_number = 0;

    for(auto i:stiffness_matrix){
        columnSums.push_back(compute_column(stiffness_matrix, column_number));
        column_number++;
    }
    return columnSums;
}

double compute_row(vector<double> matrix_row){
    double total = 0;
    for(auto i:matrix_row){
        total+=i;
    }
    return total;
}

double compute_column(vector<vector<double>> stiffness_matrix, int column_number){
    double total = 0;
    for(auto i:stiffness_matrix){
        total+=i[column_number];
    }
    return total;
}