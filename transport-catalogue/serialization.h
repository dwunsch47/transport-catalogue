#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "transport_catalogue.pb.h"
#include "svg.h"

#include <string>
#include <memory>

namespace serialization {
    
class Serializer {
public:
    explicit Serializer(tcat::TransportCatalogue& catalogue, std::shared_ptr<router::TransportRouter> tr, map_r::MapRenderer& map_renderer);
    
    void SerializeToFile(const std::string& file);
    
    void DeserializeFromFile(const std::string& file);
    
private:
    tcat::TransportCatalogue& tc_;
    std::shared_ptr<router::TransportRouter> tr_ptr_;
    map_r::MapRenderer& mr_;
    
    proto_serialization::TransportCatalogue proto_tc_;
    
    void SerializeStops();
    void SerializeBuses();
    void SerializeDistances();
    void SerializeRenderSettings();
    //void SerializeRouterSettings();
    
    void DeserializeCatalogue();
    void DeserializeRenderSettings();
    //void DeserializeRouterSettings();
    
    proto_serialization::Color SerializeColor(const svg::Color& color) const;
    svg::Color DeserializeColor(const proto_serialization::Color& proto_color) const;
};
    
} // namespace serialization