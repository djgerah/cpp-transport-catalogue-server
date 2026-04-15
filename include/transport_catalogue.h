#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace tc
{
struct Hasher
{
    size_t operator()(const Stop *stop) const noexcept
    {
        return ((hasher_db(stop->coordinates.lat) * 37) + (hasher_db(stop->coordinates.lng) * (37 * 37)));
    }

    size_t operator()(const std::pair<const Stop *, const Stop *> pair_stops) const noexcept
    {
        auto hash_1 = static_cast<const void *>(pair_stops.first);
        auto hash_2 = static_cast<const void *>(pair_stops.second);

        return (hasher_ptr(hash_1) * 37) + (hasher_ptr(hash_2) * (37 * 37));
    }

  private:
    std::hash<double> hasher_db;
    std::hash<const void *> hasher_ptr;
};

class TransportCatalogue
{
    using StopMap = std::unordered_map<std::string_view, const Stop *>;
    using BusMap = std::unordered_map<std::string_view, Bus *>;
    using HashedStops = std::unordered_set<const Stop *, Hasher>;
    using HashedDistance = std::unordered_map<std::pair<const Stop *, const Stop *>, int, Hasher>;

  public:
    void AddStop(tc::Stop stop);
    const Stop *GetStop(std::string_view stop_name) const;
    void DeleteStopFromCatalogue(std::string_view stop_name);
    void AddBus(tc::Bus bus);
    Bus *GetBus(std::string_view bus_name) const;
    void InsertBusStop(std::string_view bus_name, std::string_view stop_name, size_t pos);
    void DeleteStopFromBus(std::string_view bus_name, std::string_view stop_name);
    std::unordered_set<const Stop *, Hasher> GetUniqueStops(std::string_view bus_number) const;
    const std::map<std::string_view, const Stop *> GetAllStops() const;
    const std::map<std::string_view, const Bus *> GetAllBuses() const;
    void SetDistance(const Stop *from, const Stop *to, const int distance);
    int GetDistance(const Stop *from, const Stop *to) const;
    std::pair<int, double> GetRouteLength(const tc::Bus *bus) const;
    std::optional<tc::BusStat> GetBusStat(const std::string_view bus_number) const;

  private:
    void EraseStop(const Stop *stop);
    void EraseDistance(const Stop *stop);

    std::deque<std::unique_ptr<Stop>> stops_;
    StopMap stopname_to_stop_;
    std::deque<Bus> buses_;
    BusMap busname_to_bus_;
    HashedDistance distance_;
};
} // namespace tc