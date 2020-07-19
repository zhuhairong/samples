//
//  http_request.hpp
//  hinet
//
//  Created by cort xu on 2021/9/7.
//

#ifndef http_request_hpp
#define http_request_hpp
#include <stdio.h>
#include <iostream>
#include <future>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>
#import <WebKit/WebKit.h>
#import <Foundation/Foundation.h>

using asio::ip::tcp;

class HttpMgr;

class HttpRequest {
  friend class HttpMgr;

public:
  ~HttpRequest();

private:
  HttpRequest(asio::io_context* io_context);

public:
  bool Open(const std::string& url, const std::map<std::string, std::string> headers);
  void Close();
  
private:
  void Connect(const tcp::resolver::results_type& endpoints);
  void SendHeader();
  void RecvHeader();
  void RecvContent(const asio::error_code& ec);

private:
  void OnResolve(const asio::error_code& ec, asio::ip::tcp::resolver::results_type results);
  void OnConnect(const asio::error_code& ec);

public:
  bool complete() { return complete_; }
  uint64_t id() { return reinterpret_cast<uint64_t>(this); }

private:
  asio::io_context*                   io_context_;
  tcp::resolver                       resolver_;
  tcp::socket                         socket_;
  asio::streambuf                     request_;
  asio::streambuf                     response_;
  bool                                complete_;
  std::string                         host_;
  std::string                         port_;
  std::string                         path_;
  std::map<std::string, std::string>  headers_;
};

#endif /* http_request_hpp */
