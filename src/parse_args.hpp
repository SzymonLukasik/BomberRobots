#ifndef __PARSE_ARGS_H__
#define __PARSE_ARGS_H__

#include <boost/program_options.hpp>
#include <iostream>

#include "common.hpp"

namespace program_options = boost::program_options;

class EndPoint {
public:
    EndPoint() = default;

    EndPoint(std::string str) {
        size_t pos = str.find_last_of(":");
        ip = str.substr(0, pos);
        port = str.substr(pos + 1);

        if (ip[0] == '[' && ip[ip.size() - 1] == ']') {
            ip = ip.substr(1);
            ip = ip.substr(0, ip.size() - 1);
        }
    }
    std::string get_ip() { return ip; }

    std::string get_port() { return port; }

private:
    std::string ip;
    std::string port;

    friend std::ostream &operator<<(std::ostream &stream, const EndPoint &endpoint) {
        stream << "EndPoint: { ip: " << endpoint.ip << ", port: " << endpoint.port << " }";
        return stream; 
    }
};

bool parse_args(int argc, const char *argv[],
                EndPoint &gui_endpoint, EndPoint &server_endpoint,
                uint16_t &port, std::string &player_name) {
    
    std::string server_addr_str, gui_addr_str;
    try {
        program_options::options_description desc("Allowed options");
        desc.add_options()
            ("gui-address,d", program_options::value<std::string>(&gui_addr_str)->required(), 
            "<(host name):(port) | (IPv4):(port) | (IPv6):(port)>")
            ("help,h", "produce help message")
            ("player-name,n", program_options::value<std::string>(&player_name)->required(), "<String>")
            ("port,p", program_options::value<uint16_t>(&port)->required(), "<u16> - client listens to GUI on that port")
            ("server-address,s", program_options::value<std::string>(&server_addr_str)->required(), 
            "<(host name):(port) | (IPv4):(port) | (IPv6):(port)>")
        ;

        program_options::variables_map  options;
        program_options::store(program_options::parse_command_line(argc, argv, desc), options);

        if (options.count("help")) {
            std::cout << desc << "\n";
            return false;
        }

        program_options::notify(options);

    } catch (boost::program_options::required_option &required_option) {
        std::cerr << required_option.what() << '\n';
        return false;
    }

    gui_endpoint = EndPoint(gui_addr_str);
    server_endpoint = EndPoint(server_addr_str);

    return true;
}

#endif // __PARSE_ARGS_H__