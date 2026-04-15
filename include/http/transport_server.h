#pragma once

#include "../json_reader.h"
#include <cpprest/http_listener.h>

namespace ts
{
struct ServerData
{
    std::unique_ptr<json_reader::JsonReader> json_reader;
    tc::TransportCatalogue catalogue;
    std::unique_ptr<renderer::MapRenderer> renderer;
    std::unique_ptr<tc::TransportRouter> router;
    std::unique_ptr<RequestHandler> request_handler;
};

class TransportServer
{
    struct Listeners
    {
        Listeners()
            : bus_listener(U("http://localhost:8080/bus")), load_listener(U("http://localhost:8080/load")),
              map_listener(U("http://localhost:8080/map")), query_listener(U("http://localhost:8080/query")),
              stop_listener(U("http://localhost:8080/stop")), catalogue_listener(U("http://localhost:8080/catalogue"))
        {
        }

        web::http::experimental::listener::http_listener bus_listener;
        web::http::experimental::listener::http_listener load_listener;
        web::http::experimental::listener::http_listener map_listener;
        web::http::experimental::listener::http_listener query_listener;
        web::http::experimental::listener::http_listener stop_listener;
        web::http::experimental::listener::http_listener catalogue_listener;
    };

  public:
    void Start();
    void OpenListeners();

  private:
    void PatchBus();
    void PostLoad();
    void GetMap();
    void PostQuery();
    void PutStop();
    void DeleteStop();
    void PatchCatalogue();

    ServerData data_;
    Listeners listeners_;
};
} // namespace ts