#include <vector>
#include <fstream>
#include <iostream>

using namespace std;
int U_file();

struct UObj{
    double x;
    double y;
    double theta;
};

int main(int argc, char *  argv[]){
    U_file();
    return 0;
}

int U_file(){
    vector<UObj> vector;
    ifstream fs("../U.csv");


    //read from file
    int node_count = 0;

    while(!fs.eof()){
        UObj temp;

        fs >> temp.x;
        fs >> temp.y;
        fs >> temp.theta;

        cout << vector.back().x << "\n";

        vector.push_back(temp);
        node_count++;
    }

    fs.close();


    //write to file

    ofstream out;
    out.open("../displacements.csv");

    for(auto i:vector){
        out << i.x << ",";
        out << i.y << ",";
        out << i.theta << "\n";
    }

    out.close();

    cout << "I used a vector of structs. Each struct has an x, y, and theta value in it.\n";
    cout << "The number of nodes/structs I had in my vector was " << node_count << "\n";

    return node_count;
}