#ifndef OPENMP_FUNCTIONS_H
#define OPENMP_FUNCTIONS_H
#include <vector>
#include <omp.h>
using namespace std;
using DVector = vector<double>;

namespace distributed_functions
{
    DVector sumAlongDistributed(DVector flatBuffer, int n, int m, int axis)
    {
        int s = flatBuffer.size();
        DVector sums;
        if (axis == 0)
        { // Sum along columns
            sums.resize(m, 0.0);
#pragma omp parallel for
            for (int j = 0; j < m; j++)
            {
                double sum = 0.0;
                for (int i = 0; i < n; i++)
                {
                    sum += flatBuffer[i * m + j];
                }

                sums[j] = sum;
            }
        }
        else if (axis == 1)
        { // Sum along rows
            sums.resize(n, 0.0);
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                double sum = 0.0;
                int rowStart = i * m;

                for (int j = 0; j < m; j++)
                {
                    sum += flatBuffer[rowStart + j];
                }

                sums[i] = sum;
            }
        }
        return sums;
    }

}

#endif