#include <boost/asio.hpp>
#include "../messages/server.hpp"
#include "../parse_args.hpp"
#include "../buffers/outbuffer.hpp"

using boost::asio::ip::tcp;

template <typename T>
void write_to_socket(tcp::socket &socket, T &val) {
    boost::system::error_code error;
    OutBuffer buffer;
    buffer << val; 
    boost::asio::write(socket, boost::asio::buffer(buffer.get_data(), buffer.get_size()), error);
}

void write_client_message_to_socket(tcp::socket &socket, ClientMessage variant) {
    write_to_socket(socket, variant);
}

void test_server_serialization(EndPoint &server_endpoint) {
    try { 
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(server_endpoint.get_ip(), server_endpoint.get_port());
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        
        write_client_message_to_socket(socket, Join("Szymon"));
        write_client_message_to_socket(socket, PlaceBomb());
        write_client_message_to_socket(socket, PlaceBlock());
        write_client_message_to_socket(socket, Move(Direction::Right));
    } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    }
}