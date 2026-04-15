#include "../include/http/transport_server.h"
#include <iostream>

int main()
{
    ts::TransportServer server;
    server.Start();

    try
    {
        server.OpenListeners();
        std::cout << "Press enter to stop." << std::endl;
        std::string dummy;
        std::getline(std::cin, dummy);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
    }

    return 0;
}