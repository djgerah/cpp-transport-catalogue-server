#include "../include/json_reader.h"
#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <iostream>
#include <memory>

int main()
{
    tc::TransportCatalogue catalogue;
    std::unique_ptr<renderer::MapRenderer> renderer;
    std::unique_ptr<tc::TransportRouter> router;
    std::unique_ptr<RequestHandler> request_handler;
    std::unique_ptr<json_reader::JsonReader> json_reader;

    // --- POST /load ---
    web::http::experimental::listener::http_listener load_listener(U("http://localhost:8080/load"));
    load_listener.support(web::http::methods::POST, [&](web::http::http_request req) mutable {
        req.extract_json().then([req = std::move(req), &catalogue, &renderer, &router, &request_handler,
                                 &json_reader](pplx::task<web::json::value> task) mutable {
            try
            {
                auto j = task.get();
                std::stringstream ss;
                ss << j.serialize();

                json_reader = std::make_unique<json_reader::JsonReader>(ss);
                json_reader->FillTransportCatalogue(catalogue);

                renderer::RenderSettings render_settings =
                    json_reader->FillRenderSettings(json_reader->GetRenderSettings());
                renderer = std::make_unique<renderer::MapRenderer>(render_settings);

                tc::RoutingSettings routing_settings =
                    json_reader->FillRoutingSettings(json_reader->GetRoutingSettings());
                router = std::make_unique<tc::TransportRouter>(routing_settings, catalogue);
                router->BuildGraph(catalogue);

                request_handler = std::make_unique<RequestHandler>(catalogue, *renderer, *router);

                req.reply(web::http::status_codes::OK, "{\"status\":\"ok\"}", U("application/json"));
            }
            catch (const std::exception &e)
            {
                req.reply(web::http::status_codes::BadRequest, e.what());
            }
        });
    });

    // --- PUT /stop ---
    web::http::experimental::listener::http_listener stop_listener(U("http://localhost:8080/stop"));
    stop_listener.support(web::http::methods::PUT, [&](web::http::http_request req) mutable {
        req.extract_json().then(
            [req = std::move(req), &catalogue, &json_reader](pplx::task<web::json::value> task) mutable {
                try
                {
                    if (!json_reader)
                    {
                        req.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                  U("application/json"));
                        return;
                    }

                    auto j = task.get();
                    std::stringstream ss;
                    ss << j.serialize();
                    json::Document doc = json::Load(ss);

                    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();
                    for (const auto &r : base_requests)
                        json_reader->ParseRequest(r);
                    json_reader->ApplyCommands(catalogue);
                    req.reply(web::http::status_codes::OK, "{\"status\":\"stop added\"}", U("application/json"));
                }
                catch (const std::exception &e)
                {
                    req.reply(web::http::status_codes::InternalError, e.what());
                }
            });
    });

    // --- PATCH /bus ---
    web::http::experimental::listener::http_listener patch_listener(U("http://localhost:8080/bus"));
    patch_listener.support(web::http::methods::PATCH, [&](web::http::http_request request) mutable {
        request.extract_json().then(
            [request = std::move(request), &catalogue, &router](pplx::task<web::json::value> task) mutable {
                try
                {
                    web::json::value result = task.get();
                    std::stringstream ss;
                    ss << result.serialize();
                    json::Document doc = json::Load(ss);

                    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

                    for (const auto &r : base_requests)
                    {
                        const auto &dict = r.AsDict();
                        auto bus_name = dict.at("name").AsString();
                        auto bus = catalogue.GetBus(bus_name);

                        if (!bus)
                        {
                            request.reply(web::http::status_codes::NotFound, "{\"error\":\"bus not found\"}",
                                          U("application/json"));
                            return;
                        }

                        if (dict.count("stops"))
                        {
                            size_t pos = dict.count("position") ? static_cast<size_t>(dict.at("position").AsInt()) : 0;
                            const auto &stops = dict.at("stops").AsArray();
                            for (size_t i = 0; i < stops.size(); ++i)
                                catalogue.UpdateBusStops(bus_name, stops[i].AsString(), pos + i);
                        }

                        if (dict.count("is_roundtrip"))
                            bus->is_roundtrip = dict.at("is_roundtrip").AsBool();
                    }

                    router->BuildGraph(catalogue);

                    request.reply(web::http::status_codes::OK, "{\"status\":\"bus updated\"}", U("application/json"));
                }
                catch (const std::exception &e)
                {
                    request.reply(web::http::status_codes::InternalError, e.what());
                }
            });
    });

    // --- POST /query ---
    web::http::experimental::listener::http_listener query_listener(U("http://localhost:8080/query"));
    query_listener.support(web::http::methods::POST, [&](web::http::http_request request) mutable {
        request.extract_json().then([request = std::move(request), &catalogue, &json_reader,
                                     &request_handler](pplx::task<web::json::value> task) mutable {
            try
            {
                if (!request_handler || !json_reader)
                {
                    request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                  U("application/json"));
                    return;
                }

                web::json::value result = task.get();
                std::stringstream ss;
                ss << result.serialize();

                std::istringstream input(ss.str());
                std::ostringstream output;

                json_reader::JsonReader doc(input);
                doc.ProcessRequests(doc.GetStatRequests(), catalogue, *request_handler, output);

                request.reply(web::http::status_codes::OK, output.str(), U("application/json"));
            }
            catch (const std::exception &e)
            {
                request.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });

    // --- GET /map ---
    web::http::experimental::listener::http_listener map_listener(U("http://localhost:8080/map"));
    map_listener.support(web::http::methods::GET, [&](web::http::http_request request) mutable {
        try
        {
            if (!renderer || !json_reader)
            {
                request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                              U("application/json"));
                return;
            }

            auto svg_doc = renderer->GetSVG(catalogue.GetAllBuses());
            std::ostringstream out;
            svg_doc.Render(out);

            request.reply(web::http::status_codes::OK, out.str(), U("image/svg+xml"));
        }
        catch (const std::exception &e)
        {
            request.reply(web::http::status_codes::InternalError, e.what());
        }
    });

    // --- Open listeners ---
    try
    {
        load_listener.open().wait();
        stop_listener.open().wait();
        patch_listener.open().wait();
        query_listener.open().wait();
        map_listener.open().wait();
        std::cout << "Server running at http://localhost:8080\n";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
    }

    return 0;
}