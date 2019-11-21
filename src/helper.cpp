#include "helper.hpp"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

const int MAXLINE = 2048;

/* int main()
{
  readFile("./data/out-5.txt", 100);
} */

std::vector<std::array<std::array<float, 4>, 4>>
readFile(const std::string &file_path, unsigned frame_id) {
    ifstream inFile(file_path);

    if (!inFile) {
        cerr << "File empty ?" << endl;
        return {};
    }

    char oneline[MAXLINE];

    std::vector<std::array<std::array<float, 4>, 4>> poses;

    cout << "Started reading file " << file_path << "...";
    int frame_i = -1;
    while (inFile) {
        inFile.getline(oneline, MAXLINE);
        if (oneline[0] == 'f') {
            sscanf(oneline, "frame: %d", &frame_i);
            // cout << frame << endl;
            if (frame_i == (int)frame_id) {
                // cout << "found frame" << endl;
            }
        } else if (oneline[0] == 'p') {
            if (frame_i == (int)frame_id) {
                std::array<std::array<float, 4>, 4> matrix{
                    {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

                int pose_i = 0;
                sscanf(oneline,
                       "pose_%d: [%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, "
                       "%f, %f, %f, %f, %f]",
                       &pose_i, &matrix[0][0], &matrix[0][1], &matrix[0][2],
                       &matrix[0][3], &matrix[1][0], &matrix[1][1],
                       &matrix[1][2], &matrix[1][3], &matrix[2][0],
                       &matrix[2][1], &matrix[2][2], &matrix[2][3],
                       &matrix[3][0], &matrix[3][1], &matrix[3][2],
                       &matrix[3][3]);
                //  cout << pose_i << endl;
                // cout << matrix[1][4] << endl;
                poses.push_back(matrix);
            }
        }
    }
    cout << "Done reading file " << file_path << endl;
    cout << "Found " << poses.size() << " matrices" << endl;

    inFile.close();

    return poses;
}

// g++ -I . -std=c++11  helper.cpp -o helper
