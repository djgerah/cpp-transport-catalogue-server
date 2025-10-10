#include "../include/server_handlers.h"
#include "../include/json_reader.h"
#include <cstddef>

void HandleLoad(const std::string &body, ServerState &state)
{
    std::istringstream input(body);
    state.catalogue = std::make_unique<tc::TransportCatalogue>();
    state.json_reader = std::make_unique<json_reader::JsonReader>(input);
    state.json_reader->FillTransportCatalogue(*state.catalogue);

    renderer::RenderSettings settings = state.json_reader->FillRenderSettings(state.json_reader->GetRenderSettings());
    state.renderer = std::make_unique<renderer::MapRenderer>(settings);

    auto routing_settings = state.json_reader->FillRoutingSettings(state.json_reader->GetRoutingSettings());
    state.router = std::make_unique<tc::TransportRouter>(routing_settings, *state.catalogue);

    state.request_handler = std::make_unique<RequestHandler>(*state.catalogue, *state.renderer, *state.router);
}

void HandleQuery(const std::string &body, httplib::Response &res, ServerState &state)
{
    if (!state.request_handler)
    {
        res.status = 400;
        res.set_content("{\"error\":\"catalogue not loaded\"}", "application/json");
        return;
    }

    try
    {
        std::istringstream input(body);
        std::ostringstream output;

        json_reader::JsonReader document(input);
        document.ProcessRequests(document.GetStatRequests(), *state.catalogue, *state.request_handler, output);

        res.set_content(output.str(), "application/json");
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
    }
}

void HandleMap(httplib::Response &res, ServerState &state)
{
    if (!state.renderer || !state.catalogue)
    {
        res.status = 400;
        res.set_content("{\"error\":\"catalogue not loaded\"}", "application/json");
        return;
    }

    try
    {
        auto svg_doc = state.renderer->GetSVG(state.catalogue->GetAllBuses());
        std::ostringstream out;
        svg_doc.Render(out);
        res.set_content(out.str(), "image/svg+xml");
    }

    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
    }
}

void HandlePutStop(const std::string &body, ServerState &state)
{
    if (!state.catalogue || !state.json_reader)
        return;

    std::istringstream input(body);
    json::Document doc = json::Load(input);
    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

    for (const auto &request : base_requests)
    {
        state.json_reader->ParseRequest(request);
    }

    state.json_reader->ApplyCommands(*state.catalogue);
}

void HandlePutBus(const std::string &body, ServerState &state)
{
    if (!state.catalogue || !state.json_reader)
        return;

    std::istringstream input(body);
    json::Document doc = json::Load(input);
    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

    for (const auto &request : base_requests)
    {
        state.json_reader->ParseRequest(request);
    }

    state.json_reader->ApplyCommands(*state.catalogue);
}

void HandlePatch(const std::string &body, ServerState &state)
{
    if (!state.catalogue || !state.json_reader)
    {
        return;
    }

    std::istringstream input(body);
    json::Document doc = json::Load(input);
    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

    for (const auto &req : base_requests)
    {
        const auto &request = req.AsDict();
        const auto type = request.at("type").AsString();

        if (type == "Bus")
        {
            HandlePatchBus(request, state);
        }
    }
}

void HandlePatchBus(const json::Dict &request, ServerState &state)
{
    const auto bus_name = request.at("name").AsString();
    auto bus = state.catalogue->GetBus(bus_name);

    if (!bus)
    {
        return;
    }

    if (request.count("stops"))
    {
        size_t pos = 0;

        if (request.count("position"))
        {
            pos = static_cast<size_t>(request.at("position").AsInt());
        }

        const auto &stops = request.at("stops").AsArray();
        for (size_t i = 0; i < stops.size(); i++)
        {
            state.catalogue->UpdateBusStops(bus_name, stops[i].AsString(), pos + i);
        }
    }

    if (request.count("is_roundtrip"))
    {
        bus->is_roundtrip = request.at("is_roundtrip").AsBool();
    }
}