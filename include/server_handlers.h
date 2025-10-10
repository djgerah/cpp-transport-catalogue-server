#pragma once

#include "../include/server_state.h"
#include "../third_party/httplib.h"

#include <string>

void HandleLoad(const std::string &body, ServerState &state);
void HandleQuery(const std::string &body, httplib::Response &res, ServerState &state);
void HandleMap(httplib::Response &res, ServerState &state);
void HandlePutStop(const std::string &body, ServerState &state);
void HandlePutBus(const std::string &body, ServerState &state);
void HandlePatch(const std::string &body, ServerState &state);
void HandlePatchBus(const json::Dict &request, ServerState &state);