#pragma once

#include "../include/json_reader.h"

struct ServerState
{
    std::unique_ptr<tc::TransportCatalogue> catalogue;
    std::unique_ptr<renderer::MapRenderer> renderer;
    std::unique_ptr<tc::TransportRouter> router;
    std::unique_ptr<RequestHandler> request_handler;
    std::unique_ptr<json_reader::JsonReader> json_reader;
};