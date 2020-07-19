//
//  http_test.cpp
//  hinet
//
//  Created by cort xu on 2021/9/8.
//

#include "http_test.hpp"

using asio::ip::tcp;

class HttpClient
{
public:
  HttpClient(asio::io_context& io_context,
      const std::string& server, const std::string& path)
    : resolver_(io_context), socket_(io_context), host_(server), path_(path) {
  }
  
  void request() {
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);
    request_stream << "GET " << path_ << " HTTP/1.0\r\n";
    request_stream << "Host: " << host_ << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query(host_, "http");
//    resolver_.async_resolve(server, "http",
    resolver_.async_resolve(query,
        std::bind(&HttpClient::handle_resolve, this,
          std::placeholders::_1,
          std::placeholders::_2));
  }

private:
  void handle_resolve(const asio::error_code& err,
      const tcp::resolver::results_type& endpoints)
  {
    if (!err)
    {
      // Attempt a connection to each endpoint in the list until we
      // successfully establish a connection.
      asio::async_connect(socket_, endpoints,
          std::bind(&HttpClient::handle_connect, this,
            std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_connect(const asio::error_code& err)
  {
    if (!err)
    {
      // The connection was successful. Send the request.
      asio::async_write(socket_, request_,
          std::bind(&HttpClient::handle_write_request, this,
            std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_write_request(const asio::error_code& err)
  {
    if (!err)
    {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      asio::async_read_until(socket_, response_, "\r\n",
          std::bind(&HttpClient::handle_read_status_line, this,
            std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const asio::error_code& err)
  {
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      asio::async_read_until(socket_, response_, "\r\n\r\n",
          std::bind(&HttpClient::handle_read_headers, this,
            std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_headers(const asio::error_code& err)
  {
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        std::cout << header << "\n";
      std::cout << "\n";

      // Write whatever content we already have to output.
      if (response_.size() > 0)
        std::cout << &response_;

      // Start reading remaining data until EOF.
      asio::async_read(socket_, response_,
          asio::transfer_at_least(1),
          std::bind(&HttpClient::handle_read_content, this,
            std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_content(const asio::error_code& err)
  {
    if (!err)
    {
      // Write all of the data that has been read so far.
      std::cout << &response_;

      // Continue reading remaining data until EOF.
      asio::async_read(socket_, response_,
          asio::transfer_at_least(1),
          std::bind(&HttpClient::handle_read_content, this,
            std::placeholders::_1));
    }
    else if (err != asio::error::eof)
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  std::string     host_;
  std::string     path_;
  tcp::resolver   resolver_;
  tcp::socket     socket_;
  asio::streambuf request_;
  asio::streambuf response_;
};

HttpTest::HttpTest() : thread_(new std::thread(&HttpTest::OnThread, this)) {
}

HttpTest::~HttpTest() {
  thread_->join();
}

void HttpTest::OnThread() {
  try {
    std::string host = "www.cortxu.com";
    std::string path = "/";

    asio::io_context io_context;
    HttpClient c(io_context, host, path);
    c.request();
    io_context.run();
  } catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << "\n";
  }

}
