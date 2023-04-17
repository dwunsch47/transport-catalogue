#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <tuple>
#include <algorithm>

#include "input_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace tcat {
namespace input {
void InputReader::LoadInputQueries(istream& input, TransportCatalogue& catalogue) const {
    vector<Stop> stops;
    vector<PreBus> buses;
    vector<StopDistances> stop_distances;
    string number;
    size_t number_of_ops;
    
    getline(input, number);
    number_of_ops = stoi(number);
    
    string cleaned_line;
    int i = 0;
    for (string line; i < number_of_ops && getline(input, line); ++i) {
        auto opener = line.find_first_not_of(' ');
        auto divider = line.find_first_of(' ', opener);
        auto name_start = line.find_first_not_of(' ', divider);
        cleaned_line = line.substr(name_start);
        if (line[opener] == 'S') {
            stops.push_back(ParseStopCoordinates(cleaned_line));
            stop_distances.push_back(ParseStopDistances(cleaned_line));
        } else if (line[opener] == 'B') {
            buses.push_back(ParsePreBus(cleaned_line));
        }
    }
    for (const auto& stop : stops) {
        catalogue.AddStop(stop);
    }
    for (const auto& stop_to_distances : stop_distances) {
        catalogue.AddDistanceBetweenStops(stop_to_distances);
    }
    for (const auto& pre_bus : buses) {
        catalogue.AddBus(pre_bus);
    }
}

Stop InputReader::ParseStopCoordinates(std::string_view stop) const {
    Stop complete_stop;
    auto colon = stop.find_first_of(':');
    auto name_end = stop.find_last_not_of(' ', colon - 1);
    complete_stop.name = string(stop.substr(0, name_end + 1));
    auto latitude_start = stop.find_first_not_of(' ', colon + 1);
    auto comma = stop.find_first_of(',', latitude_start);
    auto latitude_end = stop.find_last_not_of(' ', comma - 1);
    complete_stop.latitude = stod(string(stop.substr(latitude_start, latitude_end - latitude_start + 1)));
    auto longtitude_start = stop.find_first_not_of(' ', comma + 1);
    auto past_longtitude_comma = stop.find_first_of(',', longtitude_start);
    complete_stop.longtitude = stod(string(stop.substr(longtitude_start, past_longtitude_comma - latitude_start)));
    return complete_stop;
}
    
StopDistances InputReader::ParseStopDistances(std::string_view stop) const {
    StopDistances stop_with_dist;
    string stop_name;
    size_t dist = 0;
    auto colon = stop.find_first_of(':');
    auto stop_name_end = stop.find_last_not_of(' ', colon - 1);
    stop_with_dist.name = string(stop.substr(0, stop_name_end + 1));
    auto comma = stop.find_first_of(','); // after lat
    comma = stop.find_first_of(',', comma + 1); // after lng
    while (comma != string::npos) {
        auto next_comma = stop.find_first_of(',', comma + 1);
        auto number_start = stop.find_first_not_of(' ', comma + 1);
        auto number_end = stop.find_first_of('m', number_start);
        dist = stoi(string(stop.substr(number_start, number_end - number_start)));
        auto name_start = stop.find_first_not_of(' ', number_end + 1);
        name_start += 2; // to skip 'to'
        name_start = stop.find_first_not_of(' ', name_start);
        auto name_end = min(stop.find_last_not_of(' '), (next_comma - 1));
        stop_name = string(stop.substr(name_start, name_end - name_start + 1));
        stop_with_dist.stop_to_distance.emplace(stop_name, dist);
        comma = next_comma;
    }
    return stop_with_dist;
}

PreBus InputReader::ParsePreBus(std::string_view bus) const {
    PreBus pre_bus;
    vector<string> pre_stops;
    auto colon = bus.find_first_of(':');
    auto name_end = bus.find_last_not_of(' ', colon - 1);
    pre_bus.name = string(bus.substr(0, name_end + 1));
    auto divider = min(bus.find_first_of('-', colon), bus.find_first_of('>', colon));
    pre_bus.type = (bus[divider] == '>' ? "circular"s : "usual"s);
    auto prev_divider = colon;
    while (divider != string::npos) {
        auto stop_start = bus.find_first_not_of(' ', prev_divider + 1);
        auto stop_end = bus.find_last_not_of(' ', divider - 1);
        pre_stops.push_back(string(bus.substr(stop_start, stop_end - stop_start + 1)));
        prev_divider = divider;
        divider = bus.find_first_of(bus[prev_divider], prev_divider + 1);
    }
    auto last_stop_start = bus.find_first_not_of(' ', prev_divider + 1);
    pre_stops.push_back(string(bus.substr(last_stop_start)));
    if (pre_bus.type == "usual"s)  {
        vector<string> pre(pre_stops.begin(), pre_stops.end() - 1);
        reverse(pre.begin(), pre.end());
        for (const auto& stop : pre) {
            pre_stops.push_back(stop);
        }
    }
    pre_bus.stops = move(pre_stops);
    return pre_bus;
}
}
}