//
//  http_mgr.cpp
//  hinet
//
//  Created by cort xu on 2021/8/24.
//

#include "http_mgr.hpp"
#include <future>
#include <exception>
#include "http_request.hpp"

HttpMgr::HttpMgr() {
  thread_ = std::make_shared<std::thread>(&HttpMgr::OnThreadRun, this);
}

HttpMgr::~HttpMgr() {
  io_context_.stop();
  thread_->join();
}

HttpMgr* HttpMgr::Instance() {
  static HttpMgr* instance = nullptr;
  static std::once_flag once;
  std::call_once(once, [&] {
    instance = new HttpMgr();
  });
  
  return instance;
}

std::shared_ptr<HttpRequest> HttpMgr::CreateHttpRequest() {
  auto request = std::shared_ptr<HttpRequest>(new HttpRequest(&io_context_));
  std::unique_lock<std::mutex> lock(mutex_);
  http_requests_[request->id()] = request;
  return request;
}

void HttpMgr::RemoveHttpRequest(std::shared_ptr<HttpRequest> request) {
  if (!request) {
    return;
  }
  
  std::unique_lock<std::mutex> lock(mutex_);
  http_requests_.erase(request->id());
}

void HttpMgr::OnThreadRun() {
  printf("%s %d \r\n", __FUNCTION__, __LINE__);

  while (true) {
    asio::error_code err;
    io_context_.run_one(err);
    if (err) {
      std::cout << "run_one: " << err << std::endl;
      break;
    }
  }

  printf("%s %d \r\n", __FUNCTION__, __LINE__);
}

void HttpMgr::CheckStatus() {
  std::unique_lock<std::mutex> lock(mutex_);
  for (auto it = http_requests_.begin(); it != http_requests_.end(); ) {
    if (it->second->complete()) {
      it = http_requests_.erase(it);
    } else {
      ++ it;
    }
  }
}
