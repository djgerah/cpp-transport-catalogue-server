#include "../include/http/server_data.h"
#include <iostream>

int main()
{
    ServerData data;
    Listeners listeners;

    PatchBus(listeners, data);
    PostLoad(listeners, data);
    GetMap(listeners, data);
    PostQuery(listeners, data);
    PutStop(listeners, data);

    try
    {
        listeners.load_listener.open().wait();
        listeners.stop_listener.open().wait();
        listeners.bus_listener.open().wait();
        listeners.query_listener.open().wait();
        listeners.map_listener.open().wait();
        std::cout << "Server running at http://localhost:8080\n";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
    }

    return 0;
}