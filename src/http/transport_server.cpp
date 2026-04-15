#include "../../include/http/transport_server.h"
#include "../../include/utils/request_timer.h"
#include "../../include/utils/transport_logger.h"

void ts::TransportServer::Start()
{
    PatchBus();
    PostLoad();
    GetMap();
    PostQuery();
    PutStop();
}

void ts::TransportServer::OpenListeners()
{
    listeners_.load_listener.open().wait();
    listeners_.stop_listener.open().wait();
    listeners_.bus_listener.open().wait();
    listeners_.query_listener.open().wait();
    listeners_.map_listener.open().wait();

    std::cout << "Server running at http://localhost:8080\n";
}

void ts::TransportServer::PostLoad()
{
    listeners_.load_listener.support(web::http::methods::POST, [&](web::http::http_request request) mutable {
        ts::TransportLogger::LogRequest(request);
        std::shared_ptr timer = std::make_shared<ts::RequestTimer>();

        request.extract_json().then(
            [request = std::move(request), this, timer](pplx::task<web::json::value> task) mutable {
                try
                {
                    auto value = task.get();
                    std::stringstream stream;
                    stream << value.serialize();

                    std::istringstream input(stream.str());

                    data_.json_reader = std::make_unique<json_reader::JsonReader>(input);
                    data_.json_reader->FillTransportCatalogue(data_.catalogue);

                    renderer::RenderSettings render_settings =
                        data_.json_reader->FillRenderSettings(data_.json_reader->GetRenderSettings());
                    data_.renderer = std::make_unique<renderer::MapRenderer>(render_settings);

                    tc::RoutingSettings routing_settings =
                        data_.json_reader->FillRoutingSettings(data_.json_reader->GetRoutingSettings());
                    data_.router = std::make_unique<tc::TransportRouter>(routing_settings, data_.catalogue);
                    data_.router->BuildGraph(data_.catalogue);

                    data_.request_handler =
                        std::make_unique<RequestHandler>(data_.catalogue, *data_.renderer, *data_.router);

                    request.reply(web::http::status_codes::OK, "{\"status\":\"ok\"}", U("application/json"));

                    timer->SetStatus(200);
                }
                catch (const std::exception &e)
                {
                    ts::TransportLogger::LogError(e.what());

                    request.reply(web::http::status_codes::BadRequest, e.what());

                    timer->SetStatus(500);
                }
            });
    });
}

void ts::TransportServer::GetMap()
{
    listeners_.map_listener.support(web::http::methods::GET, [this](web::http::http_request request) mutable {
        ts::TransportLogger::LogRequest(request);
        std::shared_ptr timer = std::make_shared<ts::RequestTimer>();

        try
        {
            if (!data_.renderer || !data_.json_reader)
            {
                request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                              U("application/json"));

                timer->SetStatus(400);
                return;
            }

            auto svg = data_.renderer->GetSVG(data_.catalogue.GetAllBuses());
            std::ostringstream output;
            svg.Render(output);

            request.reply(web::http::status_codes::OK, output.str(), U("image/svg+xml"));

            timer->SetStatus(200);
        }
        catch (const std::exception &e)
        {
            ts::TransportLogger::LogError(e.what());

            request.reply(web::http::status_codes::InternalError, e.what());

            timer->SetStatus(500);
        }
    });
}

void ts::TransportServer::PostQuery()
{
    listeners_.query_listener.support(web::http::methods::POST, [this](web::http::http_request request) mutable {
        ts::TransportLogger::LogRequest(request);
        std::shared_ptr timer = std::make_shared<ts::RequestTimer>();

        request.extract_json().then(
            [request = std::move(request), this, timer](pplx::task<web::json::value> task) mutable {
                try
                {
                    if (!data_.request_handler || !data_.json_reader)
                    {
                        request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                      U("application/json"));

                        timer->SetStatus(400);

                        return;
                    }

                    auto value = task.get();
                    std::stringstream stream;
                    stream << value.serialize();

                    std::istringstream input(stream.str());
                    std::ostringstream output;

                    json_reader::JsonReader doc(input);
                    doc.ProcessRequests(doc.GetStatRequests(), data_.catalogue, *data_.request_handler, output);

                    request.reply(web::http::status_codes::OK, output.str(), U("application/json"));

                    timer->SetStatus(200);
                }
                catch (const std::exception &e)
                {
                    ts::TransportLogger::LogError(e.what());

                    request.reply(web::http::status_codes::InternalError, e.what());

                    timer->SetStatus(500);
                }
            });
    });
}

void ts::TransportServer::PutStop()
{
    listeners_.stop_listener.support(web::http::methods::PUT, [&](web::http::http_request request) mutable {
        ts::TransportLogger::LogRequest(request);
        std::shared_ptr timer = std::make_shared<ts::RequestTimer>();

        request.extract_json().then(
            [request = std::move(request), this, timer](pplx::task<web::json::value> task) mutable {
                try
                {
                    if (!data_.json_reader)
                    {
                        request.reply(web::http::status_codes::BadRequest, "{\"error\":\"catalogue not loaded\"}",
                                      U("application/json"));

                        timer->SetStatus(400);

                        return;
                    }

                    auto value = task.get();
                    std::stringstream stream;
                    stream << value.serialize();

                    std::istringstream input(stream.str());

                    json::Document doc = json::Load(input);

                    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

                    for (const auto &r : base_requests)
                    {
                        data_.json_reader->ParseRequest(r);
                    }

                    data_.json_reader->ApplyCommands(data_.catalogue);
                    request.reply(web::http::status_codes::OK, "{\"status\":\"stop added\"}", U("application/json"));

                    timer->SetStatus(200);
                }
                catch (const std::exception &e)
                {
                    ts::TransportLogger::LogError(e.what());

                    request.reply(web::http::status_codes::InternalError, e.what());

                    timer->SetStatus(500);
                }
            });
    });
}

void ts::TransportServer::PatchBus()
{
    listeners_.bus_listener.support(web::http::methods::PATCH, [&](web::http::http_request request) mutable {
        ts::TransportLogger::LogRequest(request);
        std::shared_ptr timer = std::make_shared<ts::RequestTimer>();

        request.extract_json().then(
            [request = std::move(request), this, timer](pplx::task<web::json::value> task) mutable {
                try
                {
                    auto value = task.get();
                    std::stringstream stream;
                    stream << value.serialize();

                    std::istringstream input(stream.str());

                    json::Document doc = json::Load(input);

                    const auto &base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();

                    for (const auto &r : base_requests)
                    {
                        const auto &dict = r.AsDict();
                        auto bus_name = dict.at("name").AsString();
                        auto bus = data_.catalogue.GetBus(bus_name);

                        if (!bus)
                        {
                            request.reply(web::http::status_codes::NotFound, "{\"error\":\"bus not found\"}",
                                          U("application/json"));

                            timer->SetStatus(400);

                            return;
                        }

                        if (dict.count("stops"))
                        {
                            size_t pos = dict.count("position") ? static_cast<size_t>(dict.at("position").AsInt()) : 0;
                            const auto &stops = dict.at("stops").AsArray();

                            for (size_t i = 0; i < stops.size(); ++i)
                            {
                                data_.catalogue.UpdateBusStops(bus_name, stops[i].AsString(), pos + i);
                            }
                        }

                        if (dict.count("is_roundtrip"))
                            bus->is_roundtrip = dict.at("is_roundtrip").AsBool();
                    }

                    data_.router->BuildGraph(data_.catalogue);

                    request.reply(web::http::status_codes::OK, "{\"status\":\"bus updated\"}", U("application/json"));

                    timer->SetStatus(200);
                }
                catch (const std::exception &e)
                {
                    ts::TransportLogger::LogError(e.what());

                    request.reply(web::http::status_codes::InternalError, e.what());

                    timer->SetStatus(500);
                }
            });
    });
}