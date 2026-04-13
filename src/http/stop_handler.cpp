#include "../../include/http/server_data.h"

void PutStop(Listeners &listeners, ServerData &data)
{
    listeners.stop_listener.support(web::http::methods::PUT, [&](web::http::http_request req) mutable {
        req.extract_json().then([req = std::move(req), &data](pplx::task<web::json::value> task) mutable {
            try
            {
                if (!data.json_reader)
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
                    data.json_reader->ParseRequest(r);
                data.json_reader->ApplyCommands(data.catalogue);
                req.reply(web::http::status_codes::OK, "{\"status\":\"stop added\"}", U("application/json"));
            }
            catch (const std::exception &e)
            {
                req.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });
}