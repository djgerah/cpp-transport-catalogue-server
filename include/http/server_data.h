#pragma once

#include "../json_reader.h"
#include <cpprest/http_listener.h>

struct ServerData
{
    tc::TransportCatalogue catalogue;
    std::unique_ptr<renderer::MapRenderer> renderer;
    std::unique_ptr<tc::TransportRouter> router;
    std::unique_ptr<RequestHandler> request_handler;
    std::unique_ptr<json_reader::JsonReader> json_reader;
};

struct Listeners
{
    Listeners()
        : bus_listener(U("http://localhost:8080/bus")), load_listener(U("http://localhost:8080/load")),
          map_listener(U("http://localhost:8080/map")), query_listener(U("http://localhost:8080/query")),
          stop_listener(U("http://localhost:8080/stop"))
    {
    }

    web::http::experimental::listener::http_listener bus_listener;
    web::http::experimental::listener::http_listener load_listener;
    web::http::experimental::listener::http_listener map_listener;
    web::http::experimental::listener::http_listener query_listener;
    web::http::experimental::listener::http_listener stop_listener;
};

void PatchBus(Listeners &listeners, ServerData &data);
void PostLoad(Listeners &listeners, ServerData &data);
void GetMap(Listeners &listeners, ServerData &data);
void PostQuery(Listeners &listeners, ServerData &data);
void PutStop(Listeners &listeners, ServerData &data);