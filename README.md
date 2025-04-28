# Transport Catalogue
Graph-based transport catalogue which supports adding stops and buses, creating routes and outputing these routes as SVG image. Also supports serializing and saving routes, buses and stops

## Features
- Built-in JSON parser and formatter
- Built-in SVG formatter
- Google's ProtoBuf for serialization
- CMake for building 

## Design:
- There are two modes:
  - `make_base`, which parses all needed information (such as all the stops, buses, routes, map and serialization settings), builds graph and serializes all needed data for future use
  - `process_requests`, which parses stats requests and de-serialization settings
 
  ### `make_base`
   - Constructs `JsonReader`, which takes newly constructed empty `tcat::TransportCatalogue` and `map_r::MapRenderer` via non-const references
   - `JsonReader.LoadBaseQueries()` takes non-const ref of desired INPUT stream (`std::cin`, for example), reads it and parses all information needed for `tcat::TransportCatalogue` and `map_r::MapRenderer`, along with serialization settings
   - After `tcat::TransportCatalogue` and `map_r::MapRenderer` have done all necessary calculations, all their data is serialized and saved

  ### `process_requests`
    - Constructs `JsonReader`, which takes newly constructed empty `tcat::TransportCatalogue` and `map_r::MapRenderer` via non-const references
    - `JsonReader.LoadStatQueries()` takes non-const ref of desired INPUT stream (`std::cin`, for example), takes non-const ref of desired OUPUT stream (`std::cout`, for example), parses de-serialization settings and stat requests from INPUT stream
    - After all data is de-serialized, requested stats are output into desired OUTPUT stream in JSON format and (if requested) map in SVG format
