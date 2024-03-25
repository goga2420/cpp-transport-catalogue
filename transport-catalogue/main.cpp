#include <iostream>
#include <string>
#include <fstream>
#include "json_reader.hpp"


using namespace std;

int main() {
//    input::InputReader inputReader;
//    inputReader.ReadInputAndGetStats();
    
//    reader::JsonReader json;
//    json.ReadJson();
//    ReadJson();
    std::ifstream file("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/input.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open JSON file." << std::endl;
            return 1;
        }

        // Читаем содержимое файла JSON в строку
        //std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::ifstream in("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/input.json");
    std::ofstream out("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/out.json");
    catalogue::TransportCatalogue catalogue;
    BaseRequest(catalogue, in, out);

}
