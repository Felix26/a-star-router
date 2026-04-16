#include "weights.hpp"

double HighwayWeights::getHighwayPenalty(Tags &tags)
{
    return penalties[static_cast<size_t>(tags.highway)];
}

double HighwayWeights::getHighwayPenalty(size_t index)
{
    return penalties[std::min(index, static_cast<size_t>(Parameters::highwayParameterCount - 1))];
}
