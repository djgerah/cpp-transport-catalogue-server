#include "../include/json_reader.h"

#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <iostream>

int main()
{
    tc::TransportCatalogue catalogue;
    std::unique_ptr<renderer::MapRenderer> renderer;
    std::unique_ptr<tc::TransportRouter> router;
    std::unique_ptr<RequestHandler> request_handler;
    std::unique_ptr<json_reader::JsonReader> json_reader;

    // --- Load endpoint ---
    web::http::experimental::listener::http_listener listener(U("http://localhost:8080"));
    listener.support([&](web::http::http_request request) {
        auto path = web::uri::decode(request.relative_uri().path());
        auto method = request.method();

        if (method == web::http::methods::POST && path == U("/load"))
        {
            request.extract_json().then([&](pplx::task<web::json::value> task) {
                try
                {
                    web::json::value j = task.get();
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

                    request.reply(web::http::status_codes::OK, "{\"status\":\"ok\"}", U("application/json"));
                }
                catch (const std::exception &e)
                {
                    request.reply(web::http::status_codes::BadRequest, e.what());
                }
            });
        }
    });

    // --- PUT /stop endpoint ---
    web::http::experimental::listener::http_listener stop_listener(U("http://localhost:8080/stop"));
    stop_listener.support(web::http::methods::PUT, [&](web::http::http_request request) {
        request.extract_json().then([&](pplx::task<web::json::value> task) {
            try
            {
                if (!json_reader)
                {
                    request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                  U("application/json"));
                    return;
                }

                web::json::value j = task.get();
                std::stringstream ss;
                ss << j.serialize();

                json::Document doc = json::Load(ss);
                const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

                for (const auto &req : base_requests)
                {
                    json_reader->ParseRequest(req);
                }

                json_reader->ApplyCommands(catalogue);

                request.reply(web::http::status_codes::OK, "{\"status\":\"stop added\"}", U("application/json"));
            }

            catch (const std::exception &e)
            {
                request.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });

    // --- PATCH /bus endpoint ---
    web::http::experimental::listener::http_listener patch_listener(U("http://localhost:8080/bus"));
    patch_listener.support(web::http::methods::PATCH, [&](web::http::http_request request) {
        request.extract_json().then([&](pplx::task<web::json::value> task) {
            try
            {
                if (!json_reader)
                {
                    request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                  U("application/json"));
                    return;
                }

                web::json::value result = task.get();
                std::stringstream ss;
                ss << result.serialize();

                json::Document doc = json::Load(ss);
                const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

                for (const auto &req : base_requests)
                {
                    const auto &request_dict = req.AsDict();
                    const auto bus_name = request_dict.at("name").AsString();
                    auto bus = catalogue.GetBus(bus_name);

                    if (!bus)
                    {
                        request.reply(web::http::status_codes::NotFound, "{\"error\":\"bus not found\"}",
                                      U("application/json"));
                        return;
                    }

                    if (request_dict.count("stops"))
                    {
                        size_t pos = 0;

                        if (request_dict.count("position"))
                        {
                            pos = static_cast<size_t>(request_dict.at("position").AsInt());
                        }

                        const auto &stops = request_dict.at("stops").AsArray();

                        for (size_t i = 0; i < stops.size(); i++)
                        {
                            catalogue.UpdateBusStops(bus_name, stops[i].AsString(), pos + i);
                        }
                    }

                    if (request_dict.count("is_roundtrip"))
                    {
                        bus->is_roundtrip = request_dict.at("is_roundtrip").AsBool();
                    }
                }

                request.reply(web::http::status_codes::OK, "{\"status\":\"bus updated\"}", U("application/json"));
            }

            catch (const std::exception &e)
            {
                request.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });

    // --- Query endpoint ---
    web::http::experimental::listener::http_listener query_listener(U("http://localhost:8080/query"));
    query_listener.support(web::http::methods::POST, [&](web::http::http_request request) {
        request.extract_json().then([&](pplx::task<web::json::value> task) {
            try
            {
                if (!request_handler)
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

    // --- Map endpoint ---
    web::http::experimental::listener::http_listener map_listener(U("http://localhost:8080/map"));
    map_listener.support(web::http::methods::GET, [&](web::http::http_request request) {
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

    try
    {
        listener.open().wait();
        query_listener.open().wait();
        map_listener.open().wait();
        stop_listener.open().wait();
        patch_listener.open().wait();

        std::cout << "Server running at http://localhost:8080\n";

        std::string line;
        std::getline(std::cin, line);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
    }

    return 0;
}