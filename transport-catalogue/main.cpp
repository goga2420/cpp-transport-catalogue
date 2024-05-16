#include <iostream>
#include <string>
#include <fstream>
#include "json_reader.h"


using namespace std;

int main() {
    catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader catalogue_with_json(catalogue);
    catalogue_with_json.BaseRequest(catalogue, cin, cout);

}
