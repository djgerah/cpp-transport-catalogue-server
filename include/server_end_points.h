#include "../include/server_handlers.h"

void RegisterLoadEndpoints(httplib::Server &svr, ServerState &state);
void RegisterQueryEndpoints(httplib::Server &svr, ServerState &state);
void RegisterMapEndpoints(httplib::Server &svr, ServerState &state);
void RegisterPutEndpoints(httplib::Server &svr, ServerState &state);
void RegisterPatchEndpoints(httplib::Server &svr, ServerState &state);