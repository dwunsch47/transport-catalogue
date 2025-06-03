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
  <details>
    <summary>Example of base request</summary>
    
    ````json
  {
    "serialization_settings": {
      "file": "name of the serializaiton file"
    },
    "routing_settings": {
      "bus_wait_time": 2, // how long bus will wait at stops, int32
      "bus_velocity": 30, // double
    },
    "render_settings": {
      "width": 1200, // width of the map, double
      "height": 500, // height of the map, double
      "padding": 50, // double
      "stop_radius": 5, // double
      "line_width": 14, // double
      "bus_label_font_size": 20, // int32
      "bus_label_offset": [
        7,
        15
      ], // "Point" type, two doubles
      "stop_label_font_size": 18, //int32
      "stop_label_offset": [
        7,
        -3
      ], // "Point" type, two doubles
      "underlayer_color": [
              255,
              255,
              255,
              0.85
      ], // "RGB" or "RGBA" type, three unint32 and possible double 
      "underlayer_width": 3, //double
      "color_palette": [
          "green",
          [
              255,
              160,
              0
          ],
          "red"
      ] // repeated "Color" type, can be RGB/RGBA and/or string
    },
    "base_requests": [
      {
        "type": "Bus", // can be "Bus" or "Stop", string
        "name": "14", // name of the stop
        "stops": [
            "some",
            "stops"
        ], // repeated string
        "is_roundtrip": true // is buses route circular, bool 
      },
      {
        "type": "Stop",
        "name": "Mira",
        "latitude": 43.601202, // double
        "longitude": 39.733879, // double
        "road_distances": {} // int32
      },
      {
        "type": "Stop",
        "name": "Druzhba",
        "latitude": 43.590041, // double
        "longitude": 39.732886, // double
        "road_distances": {} // int32
      }
      {
        "type": "Stop",
        "name": "Radost",
        "latitude": 43.6012021, // double
        "longitude": 43.601202, // double
        "road_distances": {} // int32
      }
    ]
  }
  ````
    
  </details>
  
   - Constructs `JsonReader`, which takes newly constructed empty `tcat::TransportCatalogue` and `map_r::MapRenderer` via non-const references
   - `JsonReader.LoadBaseQueries()` takes non-const ref of desired INPUT stream (`std::cin`, for example), reads it and parses all information needed for `tcat::TransportCatalogue` and `map_r::MapRenderer`, along with serialization settings
   - After `tcat::TransportCatalogue` and `map_r::MapRenderer` have done all necessary calculations, all their data is serialized and saved
  
  ### `process_requests`
  <details>
    <summary>Example of process request</summary>
    
    ````json
      {
        "serialization_settings": {
          "file": "name of the serializaiton file"
        },
        "stat_requests": [
          {
            "id": 2342341342, // unique request id, int32
            "type": "Bus", // type of request, can be "Bus", "Stop", "Route" or "Map", string
            "name": "14", // name of the requested type, string
          },
          {
            "id": 508658276,
            "type": "Stop",
            "name": "Mira"
          },
          {
            "id": 54524142534,
            "type": "Route",
            "from": "Mira", // first stop in the route to be constructed, string
            "to": "Druzhba" // last stop int the route, string
          },
          {
            "id": 1242352342,
            "type": "Map"
          }
        ]
      }
    ````
  
  </details>
    
    - Constructs `JsonReader`, which takes newly constructed empty `tcat::TransportCatalogue` and `map_r::MapRenderer` via non-const references
    - `JsonReader.LoadStatQueries()` takes non-const ref of desired INPUT stream (`std::cin`, for example), takes non-const ref of desired OUPUT stream (`std::cout`, for example), parses de-serialization settings and stat requests from INPUT stream
    - After all data is de-serialized, requested stats are output into desired OUTPUT stream in JSON format and (if requested) map in SVG format


## Usage:
- Build Protobuf
- Make new folder `build` inside project, got to it and run this command:
  ````
  cmake .. -DCMAKE_PREFIX_PATH=/path/to/built/protobuf
  cmake --build .
  ````
