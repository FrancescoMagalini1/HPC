#ifndef OPENMP_FUNCTIONS_H
#define OPENMP_FUNCTIONS_H
#include <vector>
using namespace std;
using DMatrix = vector<vector<double>>;

using SinkhornReturnType = struct
{
    DMatrix original;
    vector<double> D1;
    vector<double> D2;
    DMatrix matrix;
    int iterations;
    double error;
    int size;
};

namespace openmp_functions
{
    DMatrix diagonal(vector<double> v)
    // Creates a diagonal matrix from a vector
    {
        int n = v.size();
        DMatrix diagonalMatrix(n, vector<double>(n, 0.0));
        for (int i = 0; i < n; i++)
        {
            diagonalMatrix[i][i] = v[i];
        }
        return diagonalMatrix;
    }

    vector<double> vectorMultiply(vector<double> v1, vector<double> v2)
    // Element-wise multiplication of two vectors
    {
        int n = v1.size();
        vector<double> result(n, 0.0);
        for (int i = 0; i < n; i++)
        {
            result[i] = v1[i] * v2[i];
        }
        return result;
    }

    vector<double> reciprocal(vector<double> v)
    // Computes the reciprocal of each element in a vector
    {
        int n = v.size();
        vector<double> reciprocalVector(n, 0.0);
        for (int i = 0; i < n; i++)
        {
            reciprocalVector[i] = 1 / v[i];
        }
        return reciprocalVector;
    }

    vector<double> sumAlongOpenmp(DMatrix matrix, int axis)
    {
        int n = matrix.size();
        vector<double> sums(n, 0.0);

        if (axis == 0)
        { // Sum along columns (sum each column)
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                double local_sum = 0.0;
                for (int j = 0; j < n; j++)
                {
                    local_sum += matrix[j][i];
                }
                sums[i] = local_sum;
            }
        }
        else if (axis == 1)
        { // Sum along rows (sum each row)
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                double local_sum = 0.0;
                for (int j = 0; j < n; j++)
                {
                    local_sum += matrix[i][j];
                }
                sums[i] = local_sum;
            }
        }
        return sums;
    }

    DMatrix matrixMultiplyOpenmp(DMatrix A, DMatrix B)
    // Multiplies two matrices A and B
    {
        int n = A.size();
        DMatrix C(n, vector<double>(n, 0.0));
#pragma omp parallel for schedule(static)
        for (int i = 0; i < n; i++)
        {
            for (int k = 0; k < n; k++)
            {
                double temp = A[i][k];
                for (int j = 0; j < n; j++)
                {
                    C[i][j] += temp * B[k][j];
                }
            }
        }
        return C;
    }

    DMatrix matrixNormalizeOpenmp(DMatrix matrix, vector<double> v, int axis)
    {
        int n = matrix.size();
        DMatrix C(n, vector<double>(n, 0.0));
        if (axis == 0)
        { // Normalize along columns
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    C[i][j] = matrix[i][j] / v[j];
                }
            }
        }
        else if (axis == 1)
        { // Normalize along rows
#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                double v_i = v[i];
                for (int j = 0; j < n; j++)
                {
                    C[i][j] = matrix[i][j] / v_i;
                }
            }
        }
        return C;
    }

    double getError(DMatrix matrix, int axis)
    // Computes the maximum deviation from 1.0 of the sums along the specified axis
    {
        vector<double> sums = sumAlongOpenmp(matrix, axis);
        double error = 0.0;
        for (double sum : sums)
        {
            error = max(abs(sum - 1.0), error);
        }
        return error;
    }

    SinkhornReturnType sinkhorn_openmp(DMatrix originalMatrix, double tolerance = 1e-3, int maxIterations = 1000, bool slowVersion = true)
    // Performs the Sinkhorn algorithm to balance the matrix using openmp parallelization
    {
        DMatrix matrix = originalMatrix;
        int n = matrix.size();
        vector<double> D1(n, 1.0);
        vector<double> D2(n, 1.0);
        int iter = 0;
        double error = +INFINITY;
        while ((error > tolerance) && (iter < maxIterations))
        {
            vector<double> rowSum = sumAlongOpenmp(matrix, 1);
            D1 = vectorMultiply(D1, rowSum);
            if (slowVersion)
            {
                matrix = matrixMultiplyOpenmp(diagonal(reciprocal(rowSum)), matrix);
            }
            else
            {
                matrix = matrixNormalizeOpenmp(matrix, rowSum, 1);
            }
            vector<double> columnSum = sumAlongOpenmp(matrix, 0);
            D2 = vectorMultiply(columnSum, D2);
            if (slowVersion)
            {
                matrix = matrixMultiplyOpenmp(matrix, diagonal(reciprocal(columnSum)));
            }
            else
            {
                matrix = matrixNormalizeOpenmp(matrix, columnSum, 0);
            }
            double row_error = getError(matrix, 1);
            double column_error = getError(matrix, 0);
            error = max(row_error, column_error);
            iter++;
        }
        SinkhornReturnType result;
        result.original = originalMatrix;
        result.D1 = D1;
        result.D2 = D2;
        result.matrix = matrix;
        result.iterations = iter;
        result.error = error;
        result.size = n;
        return result;
    }
}

#endif