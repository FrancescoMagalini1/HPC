#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;
using IntMatrix = vector<vector<int>>;
using DMatrix = vector<vector<double>>;

namespace utils
{
    IntMatrix getRand(int n, int minVal, int maxVal)
    // Generates an n x n matrix with random integers between minVal and maxVal
    {
        IntMatrix grid(n, vector<int>(n));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                grid[i][j] = rand() % (maxVal - minVal + 1) + minVal;
            }
        }
        return grid;
    }

    DMatrix toDouble(IntMatrix intMatrix)
    // Converts an integer matrix to a double matrix
    {
        int n = intMatrix.size();
        DMatrix doubleMatrix(n, vector<double>(n));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                doubleMatrix[i][j] = static_cast<double>(intMatrix[i][j]);
            }
        }
        return doubleMatrix;
    }

}

#endif