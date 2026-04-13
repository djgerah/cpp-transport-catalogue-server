#include "../../include/http/server_data.h"

void GetMap(Listeners &listeners, ServerData &data)
{
    listeners.map_listener.support(web::http::methods::GET, [&data](web::http::http_request request) mutable {
        try
        {
            if (!data.renderer || !data.json_reader)
            {
                request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                              U("application/json"));
                return;
            }

            auto svg_doc = data.renderer->GetSVG(data.catalogue.GetAllBuses());
            std::ostringstream out;
            svg_doc.Render(out);

            request.reply(web::http::status_codes::OK, out.str(), U("image/svg+xml"));
        }
        catch (const std::exception &e)
        {
            request.reply(web::http::status_codes::InternalError, e.what());
        }
    });
}