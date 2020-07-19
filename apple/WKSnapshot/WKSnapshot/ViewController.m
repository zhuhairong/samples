//
//  ViewController.m
//  WKSnapshot
//
//  Created by cort xu on 2021/7/26.
//

#import "ViewController.h"
#import <WebKit/WebKit.h>

#define kScreenWidth [UIScreen mainScreen].bounds.size.width
#define kScreenHeight [UIScreen mainScreen].bounds.size.height


@interface URLSchemeHandler : NSObject <WKURLSchemeHandler>
@property (nonatomic, weak) id<WKURLSchemeHandler> delegate;
@end

@implementation URLSchemeHandler
- (void)webView:(WKWebView *)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
  if (self.delegate && [self.delegate respondsToSelector:@selector(webView:startURLSchemeTask:)]) {
    [self.delegate webView:webView startURLSchemeTask:urlSchemeTask];
  }
}

- (void)webView:(nonnull WKWebView *)webView stopURLSchemeTask:(nonnull id<WKURLSchemeTask>)urlSchemeTask {
  if (self.delegate && [self.delegate respondsToSelector:@selector(webView:stopURLSchemeTask:)]) {
    [self.delegate webView:webView stopURLSchemeTask:urlSchemeTask];
  }
}
@end

@interface ViewController () <WKScriptMessageHandler, WKURLSchemeHandler>
@property (strong, nonatomic) WKWebView*                  webView;
@property (strong, nonatomic) UIProgressView*             progressView;
@end

@implementation ViewController {
  uint32_t requestId;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  
  [self initProgressView];
  
  [self.webView addObserver:self forKeyPath:@"estimatedProgress" options:NSKeyValueObservingOptionNew context:nil];
  
  [self loadBridge];
}

- (void)dealloc {
  [_webView removeObserver:self forKeyPath:@"estimatedProgress"];
}

- (WKWebView*)webView {
  if (_webView) {
    return _webView;
  }
  
  WKWebViewConfiguration *configuration = [[WKWebViewConfiguration alloc] init];
  
  WKPreferences *preferences = [WKPreferences new];
  preferences.javaScriptCanOpenWindowsAutomatically = YES;
  preferences.minimumFontSize = 40.0;
  configuration.preferences = preferences;
  
  URLSchemeHandler* urlSchemeHandler = [[URLSchemeHandler alloc] init];
  urlSchemeHandler.delegate = self;
  [configuration setURLSchemeHandler:urlSchemeHandler forURLScheme:@"hilive"];
  
  _webView = [[WKWebView alloc] initWithFrame:self.view.bounds configuration:configuration];
  
  NSString *urlStr = [[NSBundle mainBundle] pathForResource:@"sources/index.html" ofType:nil];
  NSURL *fileURL = [NSURL fileURLWithPath:urlStr];
  [_webView loadFileURL:fileURL allowingReadAccessToURL:fileURL];
  
  _webView.backgroundColor = UIColor.blueColor;
  _webView.allowsBackForwardNavigationGestures = YES;
  [self.view addSubview:_webView];
  
  return _webView;
}

- (void)initProgressView {
  UIProgressView *progressView = [[UIProgressView alloc] initWithFrame:CGRectMake(0, 0, kScreenWidth, 2)];
  progressView.tintColor = [UIColor redColor];
  progressView.trackTintColor = [UIColor lightGrayColor];
  [self.view addSubview:progressView];
  self.progressView = progressView;
}


- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.webView.configuration.userContentController addScriptMessageHandler:self name:@"Logger"];
  [self.webView.configuration.userContentController addScriptMessageHandler:self name:@"Animate"];
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
  
  [self.webView.configuration.userContentController removeScriptMessageHandlerForName:@"Animate"];
  [self.webView.configuration.userContentController removeScriptMessageHandlerForName:@"Logger"];
}


- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
  if (object == self.webView && [keyPath isEqualToString:@"estimatedProgress"]) {
    CGFloat newprogress = [[change objectForKey:NSKeyValueChangeNewKey] doubleValue];
    if (newprogress == 1) {
      [self.progressView setProgress:1.0 animated:YES];
      dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.7 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        self.progressView.hidden = YES;
        [self.progressView setProgress:0 animated:NO];
      });
      
    }else {
      self.progressView.hidden = NO;
      [self.progressView setProgress:newprogress animated:YES];
    }
  }
}

- (void)loadJsBase:(NSMutableString*)jsStr {
  [jsStr appendString:@"var console = {};"];
  [jsStr appendString:@"console.log = function(msg) {window.webkit.messageHandlers.Logger.postMessage(msg)};"];
  [jsStr appendString:@"var starting = true;"];
}

- (void)loadJsSendData:(NSMutableString*)jsStr {
  [jsStr appendString:@"function sendSnapData(data) {"];
  [jsStr appendString:@"var request = new XMLHttpRequest();"];
  [jsStr appendString:@"request.onloadstart = function(e) {console.log('onloadstart');};"];
  [jsStr appendString:@"request.onprogress = function(e) {console.log('onprogress');};"];
  [jsStr appendString:@"request.onabort = function(e) {console.log('onabort');};"];
  [jsStr appendString:@"request.onerror = function(e) {console.log('onerror');};"];
  //  [jsStr appendString:@"request.onload = function(e) {console.log('onload: ' + request.status);};"];
  [jsStr appendString:@"request.ontimeout = function(e) {console.log('ontimeout');};"];
  //  [jsStr appendString:@"request.onloadend = function(e) {console.log('onloadend');};"];
  [jsStr appendFormat:@"request.open('POST', 'hilive://upload/%u', false);", ++ requestId];
  [jsStr appendString:@"request.setRequestHeader('content-type', 'application/arraybuffer');"];
  [jsStr appendString:@"request.send(data);"];
  [jsStr appendString:@"request.end();"];
  [jsStr appendString:@"}"];
}

- (void)loadJsSnapShot:(NSMutableString*)jsStr {
  [jsStr appendString:@"var pixelBuffer = new Uint8Array(1);"];
  [jsStr appendString:@"function snapShot() {"];
  [jsStr appendString:@"if (!starting) { return;}"];
  [jsStr appendString:@"var drawSize = gl.drawingBufferWidth * gl.drawingBufferHeight * 4;"];
  [jsStr appendString:@"if (pixelBuffer.length != drawSize) {"];
  [jsStr appendString:@"pixelBuffer = new Uint8Array(drawSize);"];
  [jsStr appendString:@"console.log('pixelBuffer init');"];
  [jsStr appendString:@"}"];
  [jsStr appendString:@"gl.readPixels(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixelBuffer);"];
  //  [jsStr appendString:@"sendSnapData(pixelBuffer);"];
  [jsStr appendString:@"}"];
}

- (void)loadJsTakeSnap:(NSMutableString*)jsStr {
  [jsStr appendString:@"function takeSnap() {"];
  [jsStr appendString:@"snapShot();"];//59.7fps
  //  [jsStr appendString:@"canvas.toDataURL('image/jpeg');"];//35.5fps
  //  [jsStr appendString:@"canvas.toBlob(function(blob) {}, 'image/jpeg');"];//37fps
  [jsStr appendString:@"window.webkit.messageHandlers.Animate.postMessage('');"];
  [jsStr appendString:@"}"];
}

- (void)loadBridge {
  NSMutableString* jsStr = [[NSMutableString alloc] init];
  
  [self loadJsBase:jsStr];
  [self loadJsSendData:jsStr];
  [self loadJsSnapShot:jsStr];
  [self loadJsTakeSnap:jsStr];
  
  [self.webView evaluateJavaScript:jsStr completionHandler:^(id _Nullable data, NSError * _Nullable error) {
    NSLog(@"loadBridge, error: %@", error);
  }];
}

- (void)statsCount {
  static uint32_t count = 0;
  static uint32_t loopCount = 0;
  static double startStamp = 0;
  static double loopStartStamp = 0;
  double now = [NSDate new].timeIntervalSince1970;
  if (!startStamp) {
    startStamp = now;
    loopStartStamp = now;
    return;
  }
  
  if (!loopStartStamp) {
    loopStartStamp = now;
  }
  
  ++ loopCount;
  ++ count;
  
  if (loopCount % 300 == 0) {
    double duration = now - startStamp;
    double loopDuration = now - loopStartStamp;
    NSLog(@"all: [count: %u duration: %f avg: %f fps: %f] loop: [count: %u duration: %f avg: %f fps: %f]",
          count, duration, duration / count, count / duration, loopCount, loopDuration, loopDuration / loopCount, loopCount / loopDuration);
    loopStartStamp = now;
    loopCount = 0;
  }
}

#pragma mark URLSchemeHandler
- (void)webView:(WKWebView *)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
  NSData* data = [@"helloworld" dataUsingEncoding:NSUTF8StringEncoding];
  NSHTTPURLResponse* response = [[NSHTTPURLResponse alloc] initWithURL:urlSchemeTask.request.URL MIMEType:@"text/plain" expectedContentLength:data.length textEncodingName:nil];
  [urlSchemeTask didReceiveResponse:response];
  [urlSchemeTask didReceiveData:data];
  [urlSchemeTask didFinish];
}

- (void)webView:(nonnull WKWebView *)webView stopURLSchemeTask:(nonnull id<WKURLSchemeTask>)urlSchemeTask {
  NSString *path = [urlSchemeTask.request.URL absoluteString];
  NSLog(@"stopURLSchemeTask, %@", path);
}

#pragma mark - WKScriptMessageHandler
- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message {
  //    message.body  --  Allowed types are NSNumber, NSString, NSDate, NSArray,NSDictionary, and NSNull.
  if ([message.name isEqualToString:@"Logger"]) {
    NSLog(@"name: %@, body: %@", message.name, message.body);
  } else if ([message.name isEqualToString:@"Animate"]) {
    [self statsCount];
  }
}

@end
