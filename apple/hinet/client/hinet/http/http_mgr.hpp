//
//  http_mgr.hpp
//  hinet
//
//  Created by cort xu on 2021/8/24.
//

#ifndef http_mgr_hpp
#define http_mgr_hpp

#include <stdio.h>
#include <iostream>
#include <future>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>

using asio::ip::tcp;

class HttpRequest;

class HttpMgr {
private:
  HttpMgr();
  ~HttpMgr();

public:
  static HttpMgr* Instance();

public:
  std::shared_ptr<HttpRequest> CreateHttpRequest();
  void RemoveHttpRequest(std::shared_ptr<HttpRequest> request);

private:
  void OnThreadRun();
  void CheckStatus();

private:
  std::shared_ptr<std::thread>                        thread_;
  asio::io_context                                    io_context_;
  std::mutex                                          mutex_;
  std::map<uint64_t, std::shared_ptr<HttpRequest>>    http_requests_;
};

#endif /* http_mgr_hpp */
