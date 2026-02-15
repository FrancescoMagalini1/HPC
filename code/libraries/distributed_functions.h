#ifndef OPENMP_FUNCTIONS_H
#define OPENMP_FUNCTIONS_H
#include <vector>
#include <omp.h>
using namespace std;
using DVector = vector<double>;

namespace distributed_functions
{

    DVector vectorMultiply(DVector v1, DVector v2)
    // Element-wise multiplication of two vectors
    {
        int n = v1.size();
        DVector result(n, 0.0);
        for (int i = 0; i < n; i++)
        {
            result[i] = v1[i] * v2[i];
        }
        return result;
    }

    DVector sumAlongDistributed(DVector flatBuffer, int n, int m, int axis)
    {
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

    DVector normalizeDistributed(DVector flatBuffer, DVector sums, int n, int m, int axis)
    {
        DVector result(n * m, 0.0);
        if (axis == 0)
        { // Normalize along colums
#pragma omp parallel for
            for (int j = 0; j < m; j++)
            {
                double col_sum = sums[j];
                for (int i = 0; i < n; i++)
                {
                    result[i * m + j] = flatBuffer[i * m + j] / col_sum;
                }
            }
        }
        else if (axis == 1)
        { // Normalize along rows
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                double rowSum = sums[i];
                double invSum = 1.0 / rowSum;
                int rowStart = i * m;
                for (int j = 0; j < m; j++)
                {
                    result[rowStart + j] = flatBuffer[rowStart + j] * invSum;
                }
            }
        }
        return result;
    }

}

#endif