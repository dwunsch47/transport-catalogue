#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    if (mode == "make_base"sv) {
	tcat::TransportCatalogue catalogue;
    	map_r::MapRenderer map_renderer;
    	io::JsonReader reader(catalogue, map_renderer);

        reader.LoadBaseQueries(std::cin);
    } else if (mode == "process_requests"sv) {
	tcat::TransportCatalogue catalogue;
    	map_r::MapRenderer map_renderer;
    	io::JsonReader reader(catalogue, map_renderer);

        reader.LoadStatQueries(std::cin, std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}