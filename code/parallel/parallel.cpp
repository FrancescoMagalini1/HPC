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

int main()
{

    return 0;
}