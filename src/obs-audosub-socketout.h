// thread example
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <string>

#include <boost/asio.hpp>
#include <boost/array.hpp>


boost::asio::io_service ioSvc;
boost::asio::ip::tcp::socket serverConn(ioSvc);

void init_conn()
{
    std::string host = "127.0.0.1";
    int port = 8964;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

    while (1) {

        boost::array<char, 128> buf;
        try
        {
            std::cout << "Connecting to " +  host + ":" + std::to_string(port) << std::endl;
            serverConn.connect(endpoint);
            std::string message = "!Client Reconnected";
            std::copy(message.begin(),message.end(),buf.begin());
            boost::system::error_code error;
            serverConn.write_some(boost::asio::buffer(buf, message.size()), error);
            serverConn.wait(serverConn.wait_error);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            serverConn.close();
        }
        sleep(1);
    };
}

void send_str(std::string message) {
    if (!serverConn.is_open()) {
        return;
    };
	boost::array<char, 128> buf;
        std::copy(message.begin(),message.end(),buf.begin());
	boost::system::error_code error;

	serverConn.write_some(boost::asio::buffer(buf, message.size()), error);
}

// int main()
// {
//     std::thread first (init_conn);     // spawn new thread that calls foo()
//     while (true){
//         sleep(1);
//         std::thread second (send_str,"qwq");  // spawn new thread that calls bar(0)
//         second.detach();
//     }

//     std::cout << "main, foo and bar now execute concurrently...\n";

//     // synchronize threads:
//     first.join();                // pauses until first finishes

//     std::cout << "foo and bar completed.\n";
//     serverConn.close();

//     return 0;
// }
