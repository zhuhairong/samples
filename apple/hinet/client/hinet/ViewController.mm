//
//  ViewController.m
//  hinet
//
//  Created by cort xu on 2021/8/22.
//

#import "ViewController.h"
#include "http/http_mgr.hpp"
#include "http/http_request.hpp"
#include "http/http_test.hpp"
#include "http/https_test.hpp"

@interface ViewController ()

@end

@implementation ViewController {
  std::shared_ptr<HttpTest>   http_test_;
  std::shared_ptr<HttpsTest>   https_test_;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.
  
  [self showAllDirs];
}

- (void)showAllDirs {
  NSString *resourceDir = [[NSBundle mainBundle] resourcePath];
  NSLog(@"resourceDir = %@\n", resourceDir);
  
  NSFileManager *fileManager = [NSFileManager defaultManager];
  NSDirectoryEnumerator *enumerator = [fileManager enumeratorAtPath:resourceDir];
  //遍历属性
  NSString *fileName;
  //下面这个方法最为关键 可以给fileName赋值，获得文件名（带文件后缀）。
  while (fileName = [enumerator nextObject]) {
    //跳过子路径
    [enumerator skipDescendants];
    //获取文件属性
    //enumerator.fileAttributes 的后面可以用点语法点出许多许多的属性。
    NSLog(@"%@",enumerator.fileAttributes);
    NSLog(@"%@",enumerator.directoryAttributes);
  }
  NSArray* fileList = [fileManager contentsOfDirectoryAtPath:resourceDir error:nil];
  NSLog(@"fileList = %@\n", fileList);
  //==============================================================================
}

- (IBAction)onClickHttpGet:(id)sender {
}

- (IBAction)onClickHttpPost:(id)sender {
}

- (IBAction)onClickHttpsGet:(id)sender {
}

- (IBAction)onClickHttpsPost:(id)sender {
}

- (IBAction)onClickTest:(id)sender {
  NSString* filePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"/sources/ca.pem"];

  std::string url = "http://cortxu.com/index.html";
  std::map<std::string, std::string> headers;
  auto request = HttpMgr::Instance()->CreateHttpRequest();
  request->Open(url, headers);

//  http_test_ = std::make_shared<HttpTest>();
//  https_test_ = std::make_shared<HttpsTest>();
}

@end
