#include "../include/server_end_points.h"

void RunServer(httplib::Server &svr, const std::string &host, int port)
{
    std::cout << "Server running at http://" << host << ":" << port << "\n";
    if (!svr.listen(host.c_str(), port))
    {
        std::cerr << "Failed to start server on " << host << ":" << port << "\n";
        std::exit(1);
    }
}

int main()
{
    httplib::Server svr;
    ServerState state;

    RegisterLoadEndpoints(svr, state);
    RegisterQueryEndpoints(svr, state);
    RegisterMapEndpoints(svr, state);
    RegisterPutEndpoints(svr, state);
    RegisterPatchEndpoints(svr, state);

    RunServer(svr, "0.0.0.0", 8080);
}