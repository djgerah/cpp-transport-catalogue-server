#include "../../include/transport_router.h"

const double TIME = 6.00;
const int MULTIPLIER = 100;

namespace tc
{
void tc::TransportRouter::AddEdgesGraph(const TransportCatalogue &catalogue)
{
    graph::VertexId vertex_id = 0;
    std::map<const tc::Stop *, graph::VertexId> stop_to_vertex_id_ = {};

    for (const auto &[stop_name, stop_ptr] : catalogue.GetAllStops())
    {
        stop_to_vertex_id_[stop_ptr] = vertex_id;
        graph_.AddEdge(
            {stop_ptr->name, 0, vertex_id, ++vertex_id, static_cast<double>(routing_settings_.bus_wait_time_)});

        ++vertex_id;
    }

    for (auto &[name, bus_ptr] : catalogue.GetAllBuses())
    {

        for (size_t i = 0; i < bus_ptr->stops.size(); ++i)
        {
            size_t span_count = 1;

            for (size_t j = i + 1; j < bus_ptr->stops.size(); ++j)
            {
                int A_to_B = 0;
                int B_to_A = 0;

                for (size_t d = i + 1; d <= j; ++d)
                {

                    A_to_B += catalogue.GetDistance(bus_ptr->stops[d - 1], bus_ptr->stops[d]);

                    B_to_A += catalogue.GetDistance(bus_ptr->stops[d], bus_ptr->stops[d - 1]);
                }

                const Stop *from = bus_ptr->stops[i];
                const Stop *to = bus_ptr->stops[j];

                graph_.AddEdge({bus_ptr->number, span_count, stop_to_vertex_id_.at(from) + 1, stop_to_vertex_id_.at(to),

                                A_to_B / (routing_settings_.bus_velocity_ / TIME * MULTIPLIER)});

                if (!bus_ptr->is_roundtrip)
                {
                    graph_.AddEdge({bus_ptr->number, span_count, stop_to_vertex_id_.at(to) + 1,
                                    stop_to_vertex_id_.at(from),
                                    B_to_A / (routing_settings_.bus_velocity_ / TIME * MULTIPLIER)});
                }

                ++span_count;
            }
        }
    }

    router_ = std::make_unique<graph::Router<double>>(graph_);
    router_->SetVertexId(stop_to_vertex_id_);
}

void tc::TransportRouter::BuildGraph(const TransportCatalogue &catalogue)
{
    graph_ = graph::DirectedWeightedGraph<double>(catalogue.GetAllStops().size() * 2);
    AddEdgesGraph(catalogue);
}

const std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRoute(const tc::Stop *from,
                                                                                const tc::Stop *to) const
{
    return router_->BuildRoute(router_->GetVertexId(from), router_->GetVertexId(to));
}

const graph::DirectedWeightedGraph<double> &TransportRouter::GetRouteGraph() const
{
    return router_->GetGraph();
}
} // namespace tc