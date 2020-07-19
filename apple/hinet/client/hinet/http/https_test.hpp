//
//  https_test.hpp
//  hinet
//
//  Created by cort xu on 2021/9/8.
//

#ifndef https_test_hpp
#define https_test_hpp
#include <stdio.h>
#include <iostream>
#include <future>
#include <map>
#include <asio.hpp>
#include <asio/ssl.hpp>

class HttpsTest {
public:
  HttpsTest();
  ~HttpsTest();

private:
  void OnThread();

private:
  std::shared_ptr<std::thread>    thread_;
};

#endif /* https_test_hpp */
