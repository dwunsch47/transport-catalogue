#pragma once

#include <iostream>
#include <string_view>

#include "transport_catalogue.h"

namespace tcat {
namespace input {
class InputReader {
public:
    void LoadInputQueries(std::istream& input, TransportCatalogue& catalogue) const;
private:
    Stop ParseStopCoordinates(std::string_view stop) const;
    StopDistances ParseStopDistances(std::string_view stop) const;
    PreBus ParsePreBus(std::string_view bus) const;
};
}
}