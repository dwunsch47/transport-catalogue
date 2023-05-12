#include <iostream>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_reader.h"

using namespace std;

int main() {
    tcat::TransportCatalogue catalogue;
    map_r::MapRenderer map_renderer;
    io::JsonReader reader;
    reader.LoadBaseQueries(cin, catalogue, map_renderer);
    reader.LoadStatQueries(cout, catalogue, map_renderer);
}