#include <iostream>
#include "../libraries/utils.h"
#include "../libraries/functions.h"
#include "../libraries/json.hpp"
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>

using namespace std;
using namespace utils;
using namespace functions;
using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SinkhornReturnType,
                                   original, D1, D2, matrix, iterations, error, size)

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
  // system("chcp 65001");
  int size = argc > 1 ? stoi(argv[1]) : 500;
  IntMatrix matrix = getRand(size, 1, 100);
  cout << "Solving a square matrix of size " << size << "...\n";
  auto start = chrono::steady_clock::now();
  SinkhornReturnType result = sinkhorn_basic(toDouble(matrix), 1e-10, 10000);
  auto end = chrono::steady_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
  cout << "Time taken: " << duration.count() << " milliseconds\n";
  saveResultToFile(result);
  return 0;
}