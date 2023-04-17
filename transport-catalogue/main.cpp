#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <utility>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    tcat::input::InputReader reader;
    tcat::TransportCatalogue catalogue;
    tcat::output::StatReader stats;
    reader.LoadInputQueries(cin, catalogue);
    stats.LoadOutputQueries(cin, catalogue); 
}