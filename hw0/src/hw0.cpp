#include <vector>
#include <fstream>
#include <iostream>

using namespace std;
int U_file(string filepath);

struct UObj{
    double x;
    double y;
    double theta;
};

int main(int argc, char *  argv[]){
    U_file("U.csv");
    return 0;
}

int U_file(string filepath){
    vector<UObj> vector;
    ifstream fs(filepath);
    double x,y,theta;

    //read from file
    int node_count = 0;

    while(fs >> x >> y >> theta){
        UObj temp;
        
        temp.x = x;
        temp.y = y;
        temp.theta = theta;

        vector.push_back(temp);
        node_count++;
        
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
    cout << "The number of nodes/structs I had in my vector was " << node_count << endl;

    return node_count;
}