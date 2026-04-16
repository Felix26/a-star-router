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

        static Parameters::RoadClass getHighwayTagID(const std::string& tagName);
        static uint8_t getHighwayParameterCount() { return static_cast<uint8_t>(Parameters::RoadClass::Unknown) + 1; };

    private:
        static const inline std::unordered_map<std::string, Parameters::RoadClass> highway = {
            // Hochrangiges Straßennetz
            {"motorway", RoadClass::Motorway},
            {"motorway_link", RoadClass::MotorwayLink},
            {"trunk", RoadClass::Trunk},
            {"trunk_link", RoadClass::TrunkLink},
            {"primary", RoadClass::Primary},
            {"primary_link", RoadClass::PrimaryLink},
            {"secondary", RoadClass::Secondary},
            {"secondary_link", RoadClass::SecondaryLink},
            {"tertiary", RoadClass::Tertiary},
            {"tertiary_link", RoadClass::TertiaryLink},

            // Untergeordnetes Straßennetz
            {"unclassified", RoadClass::Unclassified},
            {"residential", RoadClass::Residential},
            {"living_street", RoadClass::LivingStreet},
            {"service", RoadClass::Service},
            {"road", RoadClass::Road},

            // Wege & Aktive Mobilität
            {"pedestrian", RoadClass::Pedestrian},
            {"footway", RoadClass::Footway},
            {"path", RoadClass::Path},
            {"steps", RoadClass::Steps},
            {"cycleway", RoadClass::Cycleway},
            {"track", RoadClass::Track},
            {"bridleway", RoadClass::Bridleway},
            {"corridor", RoadClass::Corridor},

            // Sonderfälle
            {"platform", RoadClass::Platform},
            {"construction", RoadClass::Construction},
            {"proposed", RoadClass::Proposed}
        };
};
