#include "../../include/http/server_data.h"

void PatchBus(Listeners &listeners, ServerData &data)
{
    listeners.bus_listener.support(web::http::methods::PATCH, [&](web::http::http_request request) mutable {
        request.extract_json().then([request = std::move(request), &data](pplx::task<web::json::value> task) mutable {
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
                    auto bus = data.catalogue.GetBus(bus_name);

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
                            data.catalogue.UpdateBusStops(bus_name, stops[i].AsString(), pos + i);
                    }

                    if (dict.count("is_roundtrip"))
                        bus->is_roundtrip = dict.at("is_roundtrip").AsBool();
                }

                data.router->BuildGraph(data.catalogue);

                request.reply(web::http::status_codes::OK, "{\"status\":\"bus updated\"}", U("application/json"));
            }
            catch (const std::exception &e)
            {
                request.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });
}