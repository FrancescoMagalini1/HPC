#include <iostream>
#include "../libraries/utils.h"
#include "../libraries/openmp_functions.h"
#include "../libraries/json.hpp"
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace utils;
using namespace openmp_functions;
using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SinkhornReturnType,
                                   original, D1, D2, matrix, iterations, error)

void saveResultToFile(const SinkhornReturnType &result)
{
    json j = result;
    ofstream file("result.json");
    if (!file.is_open())
    {
        cerr << "Cannot open file for writing\n";
        return;
    }
    file << setw(2) << j << '\n';
    file.close();
    cout << "Saved to result.json\n";
}

int main(int argc, char *argv[])
{
    int size = argc > 1 ? stoi(argv[1]) : 500;
    int threads = argc > 2 ? stoi(argv[2]) : 4;
    omp_set_num_threads(threads);
    IntMatrix matrix = getRand(size, 1, 100);
    cout << "Solving a square matrix of size " << size << " using OpenMP with " << threads << " threads...\n";
    auto start = chrono::steady_clock::now();
    SinkhornReturnType result = sinkhorn_openmp(toDouble(matrix), 1e-10, 10000);
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Time taken: " << duration.count() << " milliseconds\n";
    saveResultToFile(result);
    return 0;
}