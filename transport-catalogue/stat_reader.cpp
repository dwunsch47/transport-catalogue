#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <set>

#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace tcat {
namespace output {
void StatReader::LoadOutputQueries(istream& input, ostream& output, TransportCatalogue& catalogue) const {
    string number;
    getline(input, number);
    size_t number_of_ops = stoi(number);
    
    size_t i = 0;
    string query_field;
    for (string line; i < number_of_ops && getline(input, line); ++i) {
        auto op_name_begin = line.find_first_not_of(' ');
        auto op_name_close = line.find_first_of(' ');
        string_view op_name = line.substr(op_name_begin, op_name_close - op_name_begin);
        auto query_start = line.find_first_not_of(' ', op_name_close);
        auto colon = line.find_first_of(':', query_start);
        auto query_end = line.find_last_not_of(' ', colon - 1);
        query_field = line.substr(query_start, query_end - query_start + 1);
        if (op_name == "Bus"s) {
            OutputBusInfo(catalogue.GetBusInfo(query_field), output);
        }
        else if (op_name == "Stop"s) {
            OutputStopInfo(catalogue.GetStopInfo(query_field), output);
        }
    }
}
    
void StatReader::OutputBusInfo(const BusInfo& bus_info, ostream& out) const {
    out << "Bus "s << bus_info.name << ": "s;
    if (bus_info.stops == 0) {
        out << "not found"s << endl;
        return;
    }
    out << setprecision(6);
    out << bus_info.stops << " stops on route, "s << bus_info.unique_stops << " unique stops, "s <<
        bus_info.route_length << " route length, "s << bus_info.curvature << " curvature"s << endl;
}
    
void StatReader::OutputStopInfo(const StopInfo& stop_info, ostream& out) const {
    out << "Stop "s << stop_info.name << ": "s;
    if (stop_info.status == StopInfoStatus::NOT_FOUND) {
        out << "not found"s << endl;
    } else if (stop_info.status == StopInfoStatus::NO_BUSES) {
        out << "no buses"s << endl;
    } else {
        out << "buses "s;
        for (const string_view bus : stop_info.buses) {
            out << bus << ' ';
        }
        out << endl;
    }
}
}
}
