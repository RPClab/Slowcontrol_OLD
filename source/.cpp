#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }
    std::string command{""};
    for(std::size_t i=2;i!=argc;++i)
    {
        command+=std::string(argv[i])+"*";
    }
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1],"5555");
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);
    boost::array<char, 128> buf;
    boost::array<char, 128> buf2;
    boost::system::error_code error;
    //Read
    size_t len = socket.read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof) ; // Connection closed cleanly by peer.
    else if (error) throw boost::system::system_error(error); // Some other error.
    std::cout.write(buf.data(), len);
    //Send Command
    size_t len2=  socket.write_some(boost::asio::buffer(command), error);
    if (error == boost::asio::error::eof); // Connection closed cleanly by peer.
    else if (error) throw boost::system::system_error(error); // Some other error.
    //Read return
    size_t len3 = socket.read_some(boost::asio::buffer(buf2), error);
    if (error == boost::asio::error::eof) ; // Connection closed cleanly by peer.
    else if (error) throw boost::system::system_error(error); // Some other error.
    std::cout.write(buf2.data(),len3);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
