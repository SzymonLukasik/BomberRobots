#include <boost/asio.hpp>

#include <stdio.h>
#include <string>
#include <exception>

#include "outbuffer.hpp"
#include "parse_args.hpp"
#include "common.hpp"
#include "client.hpp"

using boost::asio::ip::tcp;

int main(int argc, const char *argv[]) {
    std::string player_name;
    EndPoint gui_endpoint, server_endpoint;
    uint16_t port;

    if(!parse_args(argc, argv, gui_endpoint, server_endpoint, port, player_name)) {
        return 1;
    }

    Client client(player_name, server_endpoint, gui_endpoint, port);
    client.run();
}
