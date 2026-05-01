#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>
#include <array>

class Parameters
{
    public:
        Parameters() = default;

        const std::unordered_map<std::string, std::string> &getParameters() const { return parameters; }

        void setParameter(const std::string &key, const std::string &value);

        std::string getParameter(const std::string &key) const;
    private:
        // Parameter maps, with entries like "highway" -> "primary"; "surface" -> "asphalt"
        std::unordered_map<std::string, std::string> parameters;
};
