#ifndef FUNCTIONS_H
#define FUNCTIONS_H
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

namespace functions
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

    DMatrix newDiagonal(int n)
    // Creates an n x n identity matrix
    {
        DMatrix diagonalMatrix(n, vector<double>(n, 0.0));
        for (int i = 0; i < n; i++)
        {
            diagonalMatrix[i][i] = 1.0;
        }
        return diagonalMatrix;
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

    DMatrix matrixMultiply(DMatrix A, DMatrix B)
    // Multiplies two matrices A and B
    {
        int n = A.size();
        DMatrix C(n, vector<double>(n, 0.0));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                for (int k = 0; k < n; k++)
                {
                    C[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return C;
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

    vector<double> sumAlong(DMatrix matrix, int axis)
    // Sums the elements of the matrix along the specified axis (0 for columns, 1 for rows)
    {
        int n = matrix.size();
        vector<double> sums(n, 0.0);

        if (axis == 0)
        { // Sum along columns
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    sums[j] += matrix[i][j];
                }
            }
        }
        else if (axis == 1)
        { // Sum along rows
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    sums[i] += matrix[i][j];
                }
            }
        }
        return sums;
    }

    double getError(DMatrix matrix, int axis)
    // Computes the maximum deviation from 1.0 of the sums along the specified axis
    {
        vector<double> sums = sumAlong(matrix, axis);
        double error = 0.0;
        for (double sum : sums)
        {
            error = max(abs(sum - 1.0), error);
        }
        return error;
    }

    SinkhornReturnType sinkhorn_basic(DMatrix originalMatrix, double tolerance = 1e-3, int maxIterations = 1000)
    // Performs the Sinkhorn algorithm to balance the matrix
    {
        DMatrix matrix = originalMatrix;
        int n = matrix.size();
        vector<double> D1(n, 1.0);
        vector<double> D2(n, 1.0);
        int iter = 0;
        double error = 1000;
        while ((error > tolerance) && (iter < maxIterations))
        {
            vector<double> rowSum = sumAlong(matrix, 1);
            D1 = vectorMultiply(D1, rowSum);
            matrix = matrixMultiply(diagonal(reciprocal(rowSum)), matrix);
            vector<double> columnSum = sumAlong(matrix, 0);
            D2 = vectorMultiply(columnSum, D2);
            matrix = matrixMultiply(matrix, diagonal(reciprocal(columnSum)));
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