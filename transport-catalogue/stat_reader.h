#pragma once

#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "input_reader.h"

namespace tcat {
namespace output {
class StatReader {
public:
    void LoadOutputQueries(std::istream& input, std::ostream& output, TransportCatalogue& catalogue) const;
private:
    void OutputBusInfo(const BusInfo& bus_info, std::ostream& out) const;
    void OutputStopInfo(const StopInfo& stop_info, std::ostream& out) const;
};
}
}