//
//  test_http.hpp
//  test_cpphttplib_by_app
//
//  Created by yu.zuo on 2019/8/27.
//  Copyright © 2019 yu.zuo. All rights reserved.
//

#ifndef test_http_hpp
#define test_http_hpp

#include <stdio.h>
void test_https();
void test_https(const char* cert_file_name);
void test_http2();
void test_ssl();
void test_file_op();
void test_file_op_on_c();
void test_file_op_on_c2(const char* filename);
#endif /* test_http_hpp */
