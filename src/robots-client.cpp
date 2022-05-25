#include <boost/asio.hpp>

#include <stdio.h>
#include <string>
#include <exception>

#include "outbuffer.hpp"
#include "parse_args.hpp"
#include "common.hpp"
#include "client.hpp"

#include "test_client_server.hpp"
#include "test_client_gui.hpp"
#include "test_gui_client.hpp"

using boost::asio::ip::tcp;

int main(int argc, const char *argv[]) {
    std::string player_name;
    EndPoint gui_endpoint, server_endpoint;
    uint16_t port;

    if(!parse_args(argc, argv, gui_endpoint, server_endpoint, port, player_name)) {
        return 1;
    }

    // test_server_serialization(server_endpoint);
    // test_gui_serialization(gui_endpoint);
    // test_gui_deserialization(port);
    Client client(player_name, server_endpoint, gui_endpoint, port);
    client.run();
}
