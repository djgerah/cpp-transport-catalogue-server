#include "../include/server_end_points.h"

void RegisterLoadEndpoints(httplib::Server &svr, ServerState &state)
{
    svr.Post("/load", [&state](const httplib::Request &req, httplib::Response &res) {
        try
        {
            HandleLoad(req.body, state);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
        }
    });
}

void RegisterQueryEndpoints(httplib::Server &svr, ServerState &state)
{
    svr.Post("/query",
             [&state](const httplib::Request &req, httplib::Response &res) { HandleQuery(req.body, res, state); });
}

void RegisterMapEndpoints(httplib::Server &svr, ServerState &state)
{
    svr.Get("/map", [&state](const httplib::Request &, httplib::Response &res) { HandleMap(res, state); });
}

void RegisterPutEndpoints(httplib::Server &svr, ServerState &state)
{

    svr.Put("/stop", [&state](const httplib::Request &req, httplib::Response &res) {
        try
        {
            HandlePutStop(req.body, state);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }

        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
        }
    });

    svr.Put("/bus", [&state](const httplib::Request &req, httplib::Response &res) {
        try
        {
            HandlePutBus(req.body, state);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }

        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
        }
    });
}

void RegisterPatchEndpoints(httplib::Server &svr, ServerState &state)
{
    svr.Patch("/patch", [&state](const httplib::Request &req, httplib::Response &res) {
        try
        {
            HandlePatch(req.body, state);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content(std::string("{\"error\":\"") + e.what() + "\"}", "application/json");
        }
    });
}