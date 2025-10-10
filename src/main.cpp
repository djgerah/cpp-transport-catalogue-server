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
    web::http::experimental::listener::http_listener load_listener(U("http://localhost:8080/load"));
    load_listener.support(web::http::methods::POST, [&](web::http::http_request request) {
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

                web::json::value j = task.get();
                std::stringstream ss;
                ss << j.serialize();

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
        load_listener.open().wait();
        query_listener.open().wait();
        map_listener.open().wait();

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