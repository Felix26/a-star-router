#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>

class Parameters
{
    public:
        enum class RoadClass : uint8_t
        {
            // Hochrangiges Netz
            Motorway = 0,
            MotorwayLink = 1,
            Trunk = 2,
            TrunkLink = 3,
            Primary = 4,
            PrimaryLink = 5,
            Secondary = 6,
            SecondaryLink = 7,
            Tertiary = 8,
            TertiaryLink = 9,

            // Untergeordnetes Netz
            Unclassified = 10,
            Residential = 11,
            LivingStreet = 12,
            Service = 13,
            Road = 14,          // Unbekannte Straße

            // Wege & Aktive Mobilität
            Pedestrian = 15,
            Footway = 16,
            Path = 17,
            Steps = 18,
            Cycleway = 19,
            Track = 20,
            Bridleway = 21,
            Corridor = 22,

            // Sonderfälle
            Platform = 23,
            Construction = 24,
            Proposed = 25,
            
            // Fallback
            Unknown = 26
        };

        static uint8_t getHighwayTagID(const std::string& tagName);
        
    private:
        static const inline std::unordered_map<std::string, uint8_t> highway = {
            // Hochrangiges Straßennetz
            {"motorway", static_cast<uint8_t>(RoadClass::Motorway)},
            {"motorway_link", static_cast<uint8_t>(RoadClass::MotorwayLink)},
            {"trunk", static_cast<uint8_t>(RoadClass::Trunk)},
            {"trunk_link", static_cast<uint8_t>(RoadClass::TrunkLink)},
            {"primary", static_cast<uint8_t>(RoadClass::Primary)},
            {"primary_link", static_cast<uint8_t>(RoadClass::PrimaryLink)},
            {"secondary", static_cast<uint8_t>(RoadClass::Secondary)},
            {"secondary_link", static_cast<uint8_t>(RoadClass::SecondaryLink)},
            {"tertiary", static_cast<uint8_t>(RoadClass::Tertiary)},
            {"tertiary_link", static_cast<uint8_t>(RoadClass::TertiaryLink)},

            // Untergeordnetes Straßennetz
            {"unclassified", static_cast<uint8_t>(RoadClass::Unclassified)},
            {"residential", static_cast<uint8_t>(RoadClass::Residential)},
            {"living_street", static_cast<uint8_t>(RoadClass::LivingStreet)},
            {"service", static_cast<uint8_t>(RoadClass::Service)},
            {"road", static_cast<uint8_t>(RoadClass::Road)},

            // Wege & Aktive Mobilität
            {"pedestrian", static_cast<uint8_t>(RoadClass::Pedestrian)},
            {"footway", static_cast<uint8_t>(RoadClass::Footway)},
            {"path", static_cast<uint8_t>(RoadClass::Path)},
            {"steps", static_cast<uint8_t>(RoadClass::Steps)},
            {"cycleway", static_cast<uint8_t>(RoadClass::Cycleway)},
            {"track", static_cast<uint8_t>(RoadClass::Track)},
            {"bridleway", static_cast<uint8_t>(RoadClass::Bridleway)},
            {"corridor", static_cast<uint8_t>(RoadClass::Corridor)},

            // Sonderfälle
            {"platform", static_cast<uint8_t>(RoadClass::Platform)},
            {"construction", static_cast<uint8_t>(RoadClass::Construction)},
            {"proposed", static_cast<uint8_t>(RoadClass::Proposed)}
        };
};
