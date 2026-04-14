#include "../../include/request_handler.h"

using namespace std::literals;

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const
{
    return catalogue_.GetStop(stop_name)->buses;
}

const std::optional<graph::Router<double>::RouteInfo> RequestHandler::GetRoute(const tc::Stop *stop_from,
                                                                               const tc::Stop *stop_to) const
{
    return router_.GetRoute(stop_from, stop_to);
}

const graph::DirectedWeightedGraph<double> &RequestHandler::GetGraph() const
{
    return router_.GetRouteGraph();
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.GetSVG(catalogue_.GetAllBuses());
}