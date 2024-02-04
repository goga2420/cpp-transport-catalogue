#include <iostream>
#include <string>
#include <fstream>
#include "input_reader.h"


using namespace std;

int main() {
    input::InputReader inputReader;
    inputReader.ReadInputAndGetStats();
}

