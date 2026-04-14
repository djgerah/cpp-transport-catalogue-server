#include "../../include/json_reader.h"
#include "../../include/json_builder.h"
#include <optional>

namespace json_reader
{
using namespace std::literals;

CommandDescription JsonReader::ParseCommandDescription(const json::Node &request)
{

    json::Dict description;

    for (const auto &item : request.AsDict())
    {
        if (item.first != "type"s && item.first != "name"s)
        {
            description.insert(item);
        }
    }

    return {request.AsDict().at("type"s).AsString(), request.AsDict().at("name"s).AsString(), description};
}

void JsonReader::ParseRequest(const json::Node &request)
{
    json_reader::CommandDescription command_description = ParseCommandDescription(request);

    commands_.push_back(std::move(command_description));
}

std::vector<const tc::Stop *> JsonReader::ParseRoute(const json::Dict &description, tc::TransportCatalogue &catalogue)
{
    std::vector<const tc::Stop *> stop_ptr;

    for (const auto &stop : description.at("stops"s).AsArray())
    {
        stop_ptr.push_back(catalogue.GetStop(stop.AsString()));
    }

    return stop_ptr;
}

tc::Stop JsonReader::MakeStop(const json_reader::CommandDescription &command) const
{

    tc::Stop stop = {command.id,
                     {command.description.at("latitude"s).AsDouble(), command.description.at("longitude"s).AsDouble()},
                     {}};

    return stop;
}

tc::Bus JsonReader::MakeBus(const json_reader::CommandDescription &command, tc::TransportCatalogue &catalogue) const
{

    tc::Bus bus = {command.id, ParseRoute(command.description, catalogue),
                   command.description.at("is_roundtrip"s).AsBool()};

    return bus;
}

void JsonReader::AddDistance(const json_reader::CommandDescription &command, tc::TransportCatalogue &catalogue) const
{
    for (const auto &road_distance : command.description.at("road_distances"s).AsDict())
    {

        catalogue.SetDistance(catalogue.GetStop(command.id), catalogue.GetStop(road_distance.first),
                              road_distance.second.AsInt());
    }
}

/*
 * Обрабатывает поля CommandDescription
 */
void JsonReader::ApplyCommands(tc::TransportCatalogue &catalogue) const
{
    for (const auto &c : commands_)
    {
        // command: автобус или остановка
        if (c.command == "Stop"s)
        {
            // id: номер автобуса или название остановки
            // description: маршрут или координаты
            catalogue.AddStop(MakeStop(c));
        }
    }

    for (const auto &c : commands_)
    {
        // command: автобус или остановка
        if (c.command == "Stop"s && c.description.count("road_distances"s))
        {
            AddDistance(c, catalogue);
        }
    }

    for (const auto &c : commands_)
    {
        // command: автобус или остановка
        if (c.command == "Bus"s)
        {
            // id: номер автобуса или название остановки
            // description: маршрут или координаты
            catalogue.AddBus(MakeBus(c, catalogue));
        }
    }
}

const json::Node &JsonReader::GetRenderSettings() const
{
    return document_.GetRoot().AsDict().at("render_settings"s);
}

const json::Node &JsonReader::GetRoutingSettings() const
{
    return document_.GetRoot().AsDict().at("routing_settings"s);
}

const json::Node &JsonReader::GetBaseRequests() const
{
    return document_.GetRoot().AsDict().at("base_requests");
}

const json::Node &JsonReader::GetStatRequests() const
{
    return document_.GetRoot().AsDict().at("stat_requests"s);
}

void JsonReader::FillTransportCatalogue(tc::TransportCatalogue &catalogue)
{
    auto base_requests = document_.GetRoot().AsDict().at("base_requests"s).AsArray();

    for (const auto &base_request : base_requests)
    {
        ParseRequest(base_request);
    }

    ApplyCommands(catalogue);
}

tc::RoutingSettings JsonReader::FillRoutingSettings(const json::Node &settings) const
{
    return tc::RoutingSettings{settings.AsDict().at("bus_wait_time"s).AsInt(),
                               settings.AsDict().at("bus_velocity"s).AsDouble()};
}

renderer::RenderSettings JsonReader::FillRenderSettings(const json::Node &settings) const
{
    json::Dict request = settings.AsDict();
    renderer::RenderSettings render_settings;

    render_settings.width = request.at("width"s).AsDouble();
    render_settings.height = request.at("height"s).AsDouble();
    render_settings.padding = request.at("padding"s).AsDouble();
    render_settings.stop_radius = request.at("stop_radius"s).AsDouble();
    render_settings.line_width = request.at("line_width"s).AsDouble();
    render_settings.bus_label_font_size = request.at("bus_label_font_size"s).AsInt();
    const json::Array &bus_label_offset = request.at("bus_label_offset"s).AsArray();
    render_settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

    render_settings.stop_label_font_size = request.at("stop_label_font_size"s).AsInt();
    const json::Array &stop_label_offset = request.at("stop_label_offset"s).AsArray();
    render_settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    ProcessColors(request, render_settings);

    return render_settings;
}

svg::Rgb JsonReader::MakeRGB(const json::Array &type) const
{
    return svg::Rgb(type[0].AsInt(), type[1].AsInt(), type[2].AsInt());
}

svg::Rgba JsonReader::MakeRGBA(const json::Array &type) const
{
    return svg::Rgba(type[0].AsInt(), type[1].AsInt(), type[2].AsInt(), type[3].AsDouble());
}

void JsonReader::ProcessColors(const json::Dict &request, renderer::RenderSettings &render_settings) const
{
    if (request.at("underlayer_color"s).IsString())
    {
        render_settings.underlayer_color = request.at("underlayer_color"s).AsString();
    }

    else if (request.at("underlayer_color"s).IsArray())
    {
        const json::Array &type = request.at("underlayer_color"s).AsArray();

        if (type.size() == 3)
        {
            render_settings.underlayer_color = MakeRGB(type);
        }

        else if (type.size() == 4)
        {
            render_settings.underlayer_color = MakeRGBA(type);
        }

        else
        {
            throw std::logic_error("wrong underlayer color type"s);
        }
    }

    else
    {
        throw std::logic_error("wrong underlayer color"s);
    }

    render_settings.underlayer_width = request.at("underlayer_width"s).AsDouble();
    const json::Array &color_palette = request.at("color_palette"s).AsArray();

    for (const auto &color : color_palette)
    {
        if (color.IsString())
        {
            render_settings.color_palette.emplace_back(color.AsString());
        }

        else if (color.IsArray())
        {
            const json::Array &type = color.AsArray();

            if (type.size() == 3)
            {
                render_settings.color_palette.emplace_back(MakeRGB(type));
            }

            else if (type.size() == 4)
            {
                render_settings.color_palette.emplace_back(MakeRGBA(type));
            }

            else
            {
                throw std::logic_error("wrong color palette type"s);
            }
        }

        else
        {
            throw std::logic_error("wrong color palette"s);
        }
    }
}

void JsonReader::ProcessRequests(const json::Node &stat_requests, tc::TransportCatalogue &catalogue,
                                 RequestHandler &request_handler, std::ostream &output) const
{
    json::Array result;

    for (auto &request : stat_requests.AsArray())
    {
        const auto &request_map = request.AsDict();
        const auto &type = request_map.at("type").AsString();

        if (type == "Stop")
        {
            result.push_back(PrintStop(request_map, catalogue, request_handler).AsDict());
        }

        if (type == "Bus")
        {
            result.push_back(PrintBus(request_map, catalogue).AsDict());
        }

        if (type == "Map")
        {
            result.push_back(PrintMap(request_map, request_handler).AsDict());
        }

        if (type == "Route")
        {
            result.push_back(PrintRoute(request_map, catalogue, request_handler).AsDict());
        }
    }

    json::Print(json::Document{result}, output);
}

const json::Node JsonReader::PrintBus(const json::Dict &request, tc::TransportCatalogue &catalogue_) const
{
    json::Node result;

    const std::string &route_number = request.at("name").AsString();
    const tc::Bus *bus = catalogue_.GetBus(route_number);
    const int id = request.at("id").AsInt();

    if (bus)
    {
        const auto &bus_stat = catalogue_.GetBusStat(route_number);

        result = json::Builder{}
                     .StartDict()
                     .Key("request_id")
                     .Value(id)
                     .Key("curvature")
                     .Value(bus_stat->curvature)
                     .Key("route_length")
                     .Value(bus_stat->route_length)
                     .Key("stop_count")
                     .Value(static_cast<int>(bus_stat->total_stops))
                     .Key("unique_stop_count")
                     .Value(static_cast<int>(bus_stat->unique_stops))
                     .EndDict()
                     .Build();
    }

    else
    {
        result = json::Builder{}
                     .StartDict()
                     .Key("request_id")
                     .Value(id)
                     .Key("error_message")
                     .Value("not found")
                     .EndDict()
                     .Build();
    }

    return json::Node{result};
}

const json::Node JsonReader::PrintStop(const json::Dict &request, tc::TransportCatalogue &catalogue_,
                                       RequestHandler &request_handler) const
{
    json::Node result;

    const std::string &stop_name = request.at("name").AsString();
    const tc::Stop *stop = catalogue_.GetStop(stop_name);
    const int id = request.at("id").AsInt();

    if (stop)
    {
        json::Array buses;

        for (const auto &bus : request_handler.GetBusesByStop(stop_name))
        {
            buses.push_back(bus);
        }

        result = json::Builder{}.StartDict().Key("request_id").Value(id).Key("buses").Value(buses).EndDict().Build();
    }

    else
    {
        result = json::Builder{}
                     .StartDict()
                     .Key("request_id")
                     .Value(id)
                     .Key("error_message")
                     .Value("not found")
                     .EndDict()
                     .Build();
    }

    return json::Node{result};
}

const json::Node JsonReader::PrintMap(const json::Dict &request, RequestHandler &request_handler) const
{
    json::Node result;

    const int id = request.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = request_handler.RenderMap();
    map.Render(strm);

    result = json::Builder{}.StartDict().Key("request_id").Value(id).Key("map").Value(strm.str()).EndDict().Build();

    return json::Node{result};
}

const json::Node JsonReader::PrintRoute(const json::Dict &request, tc::TransportCatalogue &catalogue_,
                                        RequestHandler &request_handler) const
{
    json::Node result;

    const int id = request.at("id"s).AsInt();
    const tc::Stop *from = catalogue_.GetStop(request.at("from"s).AsString());
    const tc::Stop *to = catalogue_.GetStop(request.at("to"s).AsString());
    const auto &route = request_handler.GetRoute(from, to);

    if (route)
    {
        json::Array items;
        double total_time = 0.0;
        items.reserve(route.value().edges.size());

        for (auto &id : route.value().edges)
        {
            const graph::Edge<double> edge = request_handler.GetGraph().GetEdge(id);

            if (edge.span_count == 0)
            {
                items.emplace_back(json::Node(json::Builder{}
                                                  .StartDict()
                                                  .Key("type"s)
                                                  .Value("Wait"s)
                                                  .Key("stop_name"s)
                                                  .Value(edge.name)
                                                  .Key("time"s)
                                                  .Value(edge.weight)
                                                  .EndDict()
                                                  .Build()));

                total_time += edge.weight;
            }

            else
            {
                items.emplace_back(json::Node(json::Builder{}
                                                  .StartDict()
                                                  .Key("type"s)
                                                  .Value("Bus"s)
                                                  .Key("bus"s)
                                                  .Value(edge.name)
                                                  .Key("span_count"s)
                                                  .Value(static_cast<int>(edge.span_count))
                                                  .Key("time"s)
                                                  .Value(edge.weight)
                                                  .EndDict()
                                                  .Build()));

                total_time += edge.weight;
            }
        }

        result = json::Builder{}
                     .StartDict()
                     .Key("request_id"s)
                     .Value(id)
                     .Key("total_time"s)
                     .Value(total_time)
                     .Key("items"s)
                     .Value(items)
                     .EndDict()
                     .Build();
    }

    else
    {
        result = json::Builder{}
                     .StartDict()
                     .Key("request_id"s)
                     .Value(id)
                     .Key("error_message"s)
                     .Value("not found"s)
                     .EndDict()
                     .Build();
    }

    return result;
}
} // namespace json_reader