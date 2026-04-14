#include "../../include/map_renderer.h"

using namespace std::literals;

namespace renderer
{
bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

std::vector<svg::Polyline> MapRenderer::RenderRouteLines(const std::map<std::string_view, const tc::Bus *> &buses,
                                                         const SphereProjector &sphere_projector) const
{
    std::vector<svg::Polyline> lines;

    size_t color = 0;

    for (const auto &[bus_number, bus] : buses)
    {
        if (!bus->stops.empty())
        {
            std::vector<const tc::Stop *> route_stops{bus->stops.begin(), bus->stops.end()};

            if (!bus->is_roundtrip)
            {
                route_stops.insert(route_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
            }

            svg::Polyline line;

            for (const auto &stop : route_stops)
            {
                line.AddPoint(sphere_projector(stop->coordinates));
            }

            line.SetStrokeColor(render_settings_.color_palette[color]);

            line.SetFillColor("none");

            line.SetStrokeWidth(render_settings_.line_width);

            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            if (color < render_settings_.color_palette.size() - 1)
            {
                ++color;
            }

            else
            {
                color = 0;
            }

            lines.push_back(line);
        }
    }

    return lines;
}

std::vector<svg::Text> MapRenderer::RenderBusLabel(const std::map<std::string_view, const tc::Bus *> &buses,
                                                   const SphereProjector &sphere_projector) const
{
    svg::Text text;
    svg::Text underlayer;
    std::vector<svg::Text> bus_labels;

    size_t color = 0;

    for (const auto &[bus_number, bus] : buses)
    {

        if (!bus->stops.empty())
        {

            text.SetPosition(sphere_projector(bus->stops.front()->coordinates));

            text.SetOffset(render_settings_.bus_label_offset);

            text.SetFontSize(render_settings_.bus_label_font_size);

            text.SetFontFamily("Verdana"s);

            text.SetFontWeight("bold"s);

            text.SetData(bus->number);

            text.SetFillColor(render_settings_.color_palette[color]);

            underlayer.SetPosition(sphere_projector(bus->stops.front()->coordinates));
            underlayer.SetOffset(render_settings_.bus_label_offset);
            underlayer.SetFontSize(render_settings_.bus_label_font_size);
            underlayer.SetFontFamily("Verdana"s);
            underlayer.SetFontWeight("bold"s);
            underlayer.SetData(bus->number);

            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);

            underlayer.SetStrokeWidth(render_settings_.underlayer_width);

            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            bus_labels.push_back(underlayer);
            bus_labels.push_back(text);

            if (!bus->is_roundtrip && bus->stops.front() != bus->stops.back())
            {
                svg::Text text_2{text};
                svg::Text underlayer_2{underlayer};

                text_2.SetPosition(sphere_projector(bus->stops.back()->coordinates));
                underlayer_2.SetPosition(sphere_projector(bus->stops.back()->coordinates));

                bus_labels.push_back(underlayer_2);
                bus_labels.push_back(text_2);
            }

            if (color < render_settings_.color_palette.size() - 1)
            {
                ++color;
            }

            else
            {
                color = 0;
            }
        }
    }

    return bus_labels;
}

std::vector<svg::Circle> MapRenderer::RenderStopPoints(const std::map<std::string_view, const tc::Stop *> &stops,
                                                       const SphereProjector &sphere_projector) const
{

    svg::Circle circle;
    std::vector<svg::Circle> circles;

    for (const auto &[stop_name, stop] : stops)
    {

        circle.SetCenter(sphere_projector(stop->coordinates));

        circle.SetRadius(render_settings_.stop_radius);

        circle.SetFillColor("white"s);

        circles.push_back(circle);
    }

    return circles;
}

std::vector<svg::Text> MapRenderer::RenderStopLabel(const std::map<std::string_view, const tc::Stop *> &stops,
                                                    const SphereProjector &sphere_projector) const
{

    svg::Text text;
    svg::Text underlayer;
    std::vector<svg::Text> stop_labels;

    for (const auto &[stop_name, stop] : stops)
    {
        text.SetFillColor("black"s);

        text.SetPosition(sphere_projector(stop->coordinates));

        text.SetOffset(render_settings_.stop_label_offset);

        text.SetFontSize(render_settings_.stop_label_font_size);

        text.SetFontFamily("Verdana"s);

        text.SetData(stop->name);

        underlayer.SetPosition(sphere_projector(stop->coordinates));
        underlayer.SetOffset(render_settings_.stop_label_offset);
        underlayer.SetFontSize(render_settings_.stop_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetData(stop->name);

        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);

        underlayer.SetStrokeWidth(render_settings_.underlayer_width);

        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        stop_labels.push_back(underlayer);
        stop_labels.push_back(text);
    }

    return stop_labels;
}

svg::Document MapRenderer::GetSVG(const std::map<std::string_view, const tc::Bus *> &buses) const
{
    svg::Document document;
    std::vector<geo::Coordinates> stop_coordinates;
    std::map<std::string_view, const tc::Stop *> stops;

    for (const auto &[bus_number, bus] : buses)
    {
        for (const auto &stop : bus->stops)
        {
            stop_coordinates.push_back(stop->coordinates);
            stops[stop->name] = stop;
        }
    }

    SphereProjector sphere_projector(stop_coordinates.begin(), stop_coordinates.end(), render_settings_.width,
                                     render_settings_.height, render_settings_.padding);

    for (const auto &line : RenderRouteLines(buses, sphere_projector))
    {
        document.Add(line);
    }

    for (const auto &text : RenderBusLabel(buses, sphere_projector))
    {
        document.Add(text);
    }

    for (const auto &circle : RenderStopPoints(stops, sphere_projector))
    {
        document.Add(circle);
    }

    for (const auto &text : RenderStopLabel(stops, sphere_projector))
    {
        document.Add(text);
    }

    return document;
}
} // namespace renderer