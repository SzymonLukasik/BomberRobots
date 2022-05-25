#include <boost/asio.hpp>
#include "../messages/server.hpp"
#include "../parse_args.hpp"
#include "../buffers/outbuffer.hpp"

using boost::asio::ip::tcp;

void test_server_serialization(EndPoint &server_endpoint) {
    try { 
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(server_endpoint.get_ip(), server_endpoint.get_port());
        tcp::socket socket(io_context);
        std::cout << endpoints.begin()->endpoint() << '\n';
        boost::asio::connect(socket, endpoints);

        OutBuffer buffer;
        buffer 
            << ClientMessage(Join("Szymon"))
            << ClientMessage(PlaceBomb())
            << ClientMessage(PlaceBlock())
            << ClientMessage(Move(Direction::Right));
        socket << buffer;
    } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    }
}