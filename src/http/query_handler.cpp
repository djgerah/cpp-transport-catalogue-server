#include "../../include/http/server_data.h"

void PostQuery(Listeners &listeners, ServerData &data)
{
    listeners.query_listener.support(web::http::methods::POST, [&data](web::http::http_request request) mutable {
        request.extract_json().then([request = std::move(request), &data](pplx::task<web::json::value> task) mutable {
            try
            {
                if (!data.request_handler || !data.json_reader)
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
                doc.ProcessRequests(doc.GetStatRequests(), data.catalogue, *data.request_handler, output);

                request.reply(web::http::status_codes::OK, output.str(), U("application/json"));
            }
            catch (const std::exception &e)
            {
                request.reply(web::http::status_codes::InternalError, e.what());
            }
        });
    });
}