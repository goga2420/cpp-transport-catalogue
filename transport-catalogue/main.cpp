#include <iostream>
#include <string>
#include <fstream>
#include "json_reader.h"


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

        
    std::ifstream in("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/input.json");
    std::ofstream out("/Users/georgijzukov/Desktop/Transport_Catalogue V2/Transport_Catalogue V2/out.json");
    
    catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader catalogue_with_json;
    catalogue_with_json.BaseRequest(catalogue, in, out);

}
