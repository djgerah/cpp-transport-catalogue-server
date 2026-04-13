#include "../../include/http/server_data.h"

void PostLoad(Listeners &listeners, ServerData &data)
{
    listeners.load_listener.support(web::http::methods::POST, [&](web::http::http_request req) mutable {
        req.extract_json().then([req = std::move(req), &data](pplx::task<web::json::value> task) mutable {
            try
            {
                auto j = task.get();
                std::stringstream ss;
                ss << j.serialize();

                data.json_reader = std::make_unique<json_reader::JsonReader>(ss);
                data.json_reader->FillTransportCatalogue(data.catalogue);

                renderer::RenderSettings render_settings =
                    data.json_reader->FillRenderSettings(data.json_reader->GetRenderSettings());
                data.renderer = std::make_unique<renderer::MapRenderer>(render_settings);

                tc::RoutingSettings routing_settings =
                    data.json_reader->FillRoutingSettings(data.json_reader->GetRoutingSettings());
                data.router = std::make_unique<tc::TransportRouter>(routing_settings, data.catalogue);
                data.router->BuildGraph(data.catalogue);

                data.request_handler = std::make_unique<RequestHandler>(data.catalogue, *data.renderer, *data.router);

                req.reply(web::http::status_codes::OK, "{\"status\":\"ok\"}", U("application/json"));
            }
            catch (const std::exception &e)
            {
                req.reply(web::http::status_codes::BadRequest, e.what());
            }
        });
    });
}