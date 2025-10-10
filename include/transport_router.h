#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace tc
{
struct RoutingSettings
{
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;
};

class TransportRouter
{
  public:
    TransportRouter(const RoutingSettings &routing_settings, const TransportCatalogue &catalogue)
        : routing_settings_(routing_settings)
    {
        BuildGraph(catalogue);
    }

    const std::optional<graph::Router<double>::RouteInfo> GetRoute(const tc::Stop *stop_from,
                                                                   const tc::Stop *stop_to) const;
    const graph::DirectedWeightedGraph<double> &GetRouteGraph() const;

  private:
    void AddEdgesGraph(const TransportCatalogue &catalogue);
    void BuildGraph(const TransportCatalogue &catalogue);

    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    RoutingSettings routing_settings_;
};
} // namespace tc