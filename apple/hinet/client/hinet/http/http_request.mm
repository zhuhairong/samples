//
//  http_request.cpp
//  hinet
//
//  Created by cort xu on 2021/9/7.
//

#include "http_request.hpp"


HttpRequest::HttpRequest(asio::io_context* io_context) : io_context_(io_context), resolver_(*io_context), socket_(*io_context) {
  
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::Open(const std::string& url, const std::map<std::string, std::string> headers) {
  NSURL* nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
  host_.assign(nsurl.host.UTF8String);
  path_.assign(nsurl.path.UTF8String);
  if (nsurl.port == 0) {
    port_ = "80";
  } else {
    port_.assign([NSString stringWithFormat:@"%d", nsurl.port].UTF8String);
  }
  headers_ = headers;
  
  tcp::resolver::query query(host_, "http");
  resolver_.async_resolve(query, std::bind(&HttpRequest::OnResolve, this, std::placeholders::_1, std::placeholders::_2));
//  io_context_->dispatch([&] () {});
  return true;
}

void HttpRequest::Close() {
  
}

void HttpRequest::Connect(const tcp::resolver::results_type& endpoints) {
  std::ostream request_stream(&request_);
  request_stream << "GET " << path_ << " HTTP/1.0\r\n";
  request_stream << "Host: " << host_ << "\r\n";
  
  for (auto it = headers_.begin(); it != headers_.end(); ++ it) {
    request_stream << it->first << ": " << it->second << "\r\n";
  }
  
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";
  
  asio::async_connect(socket_, endpoints, std::bind(&HttpRequest::OnConnect, this, std::placeholders::_1));
  
  //  asio::async_connect(socket_, endpoints, [&] (const asio::error_code& ec) {
  //    if (ec) {
  //      std::cout << "async_connect: " << ec << std::endl;
  //      return;
  //    }
  //
  //    SendHeader();
  //  });
}

void HttpRequest::SendHeader() {
  //  asio::async_write(socket_, request_, [&] (const asio::error_code& ec) {
  //    if (ec) {
  //      std::cout << "async_write: " << ec << std::endl;
  //      return;
  //    }
  //
  //    RecvHeader();
  //  });
}

void HttpRequest::RecvHeader() {
  //  asio::async_read_until(socket_, response_, "\r\n\r\n", [&] (const asio::error_code& ec) {
  //    if (ec) {
  //      std::cout << "RecvHeader: " << ec << std::endl;
  //      return;
  //    }
  //
  //    std::istream response_stream(&response_);
  //    std::string header;
  //    while (std::getline(response_stream, header) && header != "\r")
  //      std::cout << header << "\n";
  //    std::cout << "\n";
  //
  //    // Write whatever content we already have to output.
  //    if (response_.size() > 0)
  //      std::cout << &response_;
  //
  //    asio::async_read(socket_, response_, asio::transfer_at_least(1), std::bind(&HttpRequest::RecvContent, this, std::placeholders::_1));
  //  });
}

void HttpRequest::RecvContent(const asio::error_code& ec) {
  //  if (ec) {
  //    std::cout << "RecvContent: " << ec << std::endl;
  //    return;
  //  }
  //
  //  std::cout << &response_;
  //
  //  asio::async_read(socket_, response_, asio::transfer_at_least(1), std::bind(&HttpRequest::RecvContent, this, std::placeholders::_1));
}

void HttpRequest::OnResolve(const asio::error_code& ec, asio::ip::tcp::resolver::results_type results) {
    if (ec) {
      std::cout << "async_resolve: " << ec << std::endl;
      return;
    }
    
    Connect(results);
}

void HttpRequest::OnConnect(const asio::error_code& ec) {
  if (ec) {
    std::cout << "OnConnect: " << ec << std::endl;
    return;
  }
}
