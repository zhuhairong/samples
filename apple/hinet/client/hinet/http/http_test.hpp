//
//  http_test.hpp
//  hinet
//
//  Created by cort xu on 2021/9/8.
//

#ifndef http_test_hpp
#define http_test_hpp
#include <stdio.h>
#include <iostream>
#include <future>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>

class HttpTest {
public:
  HttpTest();
  ~HttpTest();

private:
  void OnThread();

private:
  std::shared_ptr<std::thread>    thread_;
};

#endif /* http_test_hpp */
