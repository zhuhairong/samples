//
//  HttpController.m
//  hisvr
//
//  Created by cort xu on 2021/9/7.
//


#import "HttpController.h"
#include <asio.hpp>
#include "http/server.hpp"

@interface HttpController ()

@end

@implementation HttpController {
  std::shared_ptr<http::server::server> server_;
  std::shared_ptr<std::thread>  thread_;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.
  NSLog(@"HttpController");
}

- (IBAction)onClickStart:(id)sender {
  [self startServer];
}

- (IBAction)onClickStop:(id)sender {
  [self stopServer];
}

- (void)startServer {
  NSLog(@"startServer");

  NSString* docDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"/sources/http"];

  server_ = std::make_shared<http::server::server>("0.0.0.0", "80", docDir.UTF8String);
  thread_ = std::make_shared<std::thread>([=] () {
    while (server_->run_one()) {}
  });
}

- (void)stopServer {
  if (!thread_) {
    return;
  }

  server_->stop();
  thread_->join();
  thread_ = nullptr;
  server_ = nullptr;

  NSLog(@"stopServer");
}

@end
