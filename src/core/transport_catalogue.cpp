#include <stdexcept>

#include "../../include/transport_catalogue.h"

namespace tc
{
void TransportCatalogue::AddStop(tc::Stop stop)
{
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

const Stop *TransportCatalogue::GetStop(std::string_view stop_name) const
{
    if (stopname_to_stop_.find(stop_name) == stopname_to_stop_.end())
    {
        return nullptr;
    }

    else
    {
        return stopname_to_stop_.at(stop_name);
    }
}

void TransportCatalogue::AddBus(tc::Bus bus)
{
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().number] = &buses_.back();

    for (const auto &bus_stop : bus.stops)
    {
        for (auto &stop : stops_)
        {
            if (stop.name == bus_stop->name)
            {
                stop.buses.insert(bus.number);
            }
        }
    }
}

void TransportCatalogue::UpdateBusStops(std::string_view bus_name, std::string_view stop_name, size_t pos)
{
    auto bus = GetBus(bus_name);

    if (!bus)
        return;

    if (pos > bus->stops.size())
    {
        pos = bus->stops.size();
    }

    const auto stop = GetStop(stop_name);

    if (stop)
    {
        bus->stops.insert(bus->stops.begin() + pos, stop);
    }
}

Bus *TransportCatalogue::GetBus(std::string_view bus_name) const
{
    if (busname_to_bus_.find(bus_name) == busname_to_bus_.end())
    {
        return nullptr;
    }

    else
    {
        return busname_to_bus_.at(bus_name);
    }
}

std::unordered_set<const Stop *, Hasher> TransportCatalogue::GetUniqueStops(std::string_view bus_number) const
{
    std::unordered_set<const Stop *, Hasher> unique_stops;

    for (const auto &stop : busname_to_bus_.at(bus_number)->stops)
    {
        if (stopname_to_stop_.count(stop->name))
        {
            unique_stops.insert(stopname_to_stop_.at(stop->name));
        }
    }

    return unique_stops;
}

const std::map<std::string_view, const Stop *> TransportCatalogue::GetAllStops() const
{
    std::map<std::string_view, const Stop *> all_stops;

    for (const auto &stop : stopname_to_stop_)
    {
        all_stops.emplace(stop);
    }

    return all_stops;
}

const std::map<std::string_view, const Bus *> TransportCatalogue::GetAllBuses() const
{
    std::map<std::string_view, const Bus *> all_buses;

    for (const auto &bus : busname_to_bus_)
    {
        all_buses.emplace(bus);
    }

    return all_buses;
}

void TransportCatalogue::SetDistance(const Stop *from, const Stop *to, const int distance)
{
    dist_btw_stops[{from, to}] = distance;
}

int TransportCatalogue::GetDistance(const Stop *from, const Stop *to) const
{
    if (dist_btw_stops.count({from, to}))
    {
        return dist_btw_stops.at({from, to});
    }

    else if (dist_btw_stops.count({to, from}))
    {
        return dist_btw_stops.at({to, from});
    }

    else
    {
        return 0;
    }
}

std::pair<int, double> TransportCatalogue::GetRouteLength(const tc::Bus *bus) const
{
    int route_length = 0;
    double geo_length = 0.0;

    for (size_t i = 0; i < bus->stops.size() - 1; ++i)
    {
        auto from = bus->stops[i];
        auto to = bus->stops[i + 1];

        if (bus->is_roundtrip)
        {
            route_length += GetDistance(from, to);
            geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }

        else
        {
            route_length += GetDistance(from, to) + GetDistance(to, from);
            geo_length += geo::ComputeDistance(from->coordinates, to->coordinates) * 2;
        }
    }

    return {route_length, geo_length};
}

std::optional<tc::BusStat> TransportCatalogue::GetBusStat(const std::string_view bus_number) const
{
    tc::BusStat bus_stat = {};
    const tc::Bus *bus = GetBus(bus_number);

    if (!bus)
    {
        throw std::invalid_argument("bus not found");
    }

    if (bus->is_roundtrip)
    {
        bus_stat.total_stops = bus->stops.size();
    }

    else
    {
        bus_stat.total_stops = bus->stops.size() * 2 - 1;
    }

    auto distance = GetRouteLength(bus);
    bus_stat.unique_stops = GetUniqueStops(bus_number).size();
    bus_stat.route_length = distance.first;
    bus_stat.curvature = distance.first / distance.second;

    return bus_stat;
}
} // namespace tc