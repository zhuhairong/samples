//
//  ViewController.m
//  font-test
//
//  Created by cort xu on 2020/7/29.
//  Copyright © 2020 cort xu. All rights reserved.
//

#import "ViewController.h"
#import <CoreText/CoreText.h>
#include <unordered_map>
#include <vector>
#include <string>
#import "UIColor+MWEExtension.h"
#import "UIFont+UltronRTE.h"

@interface ViewController ()
@property (weak, nonatomic) IBOutlet UILabel *lbTest0;
@property (weak, nonatomic) IBOutlet UILabel *lbTest1;
@property (weak, nonatomic) IBOutlet UILabel *lbTest2;
@property (weak, nonatomic) IBOutlet UILabel *lbTest3;
@property (weak, nonatomic) IBOutlet UILabel *lbTest4;
@property (weak, nonatomic) IBOutlet UILabel *lbTest5;
@property (weak, nonatomic) IBOutlet UILabel *lbTest6;
@property (weak, nonatomic) IBOutlet UILabel *lbTest7;
@property (weak, nonatomic) IBOutlet UILabel *lbTest8;
@property (weak, nonatomic) IBOutlet UILabel *lbTest9;
@property (weak, nonatomic) IBOutlet UILabel *lbTest10;
@property (nonatomic, strong) UIView* bgView;
@property (nonatomic, strong) UIView* rdView;
@property (nonatomic, strong) UITextField* tfView;
@property (nonatomic, strong) UIImageView* imgView;
@end

@implementation ViewController


static std::vector<CGFloat> fontWeightConfig = {
//    -1.00, -0.70, -0.50, -0.23, -0.00, 0.20, 0.30, 0.40, 0.60, 0.80, 1.00
    -1.00, UIFontWeightThin, UIFontWeightUltraLight, UIFontWeightLight, UIFontWeightRegular,
    UIFontWeightMedium, UIFontWeightSemibold, UIFontWeightBold, UIFontWeightHeavy, UIFontWeightBlack, 1.00
};

- (void)viewDidLoad {
    [super viewDidLoad];

    _bgView = [[UIView alloc] initWithFrame:CGRectMake(50, 100, 100, 30)];
    _bgView.backgroundColor = [UIColor colorWithHexString:@"FA5151"];
    _bgView.layer.cornerRadius = _bgView.frame.size.height / 2;
    [self.view addSubview:_bgView];

    _rdView = [[UIView alloc] initWithFrame:CGRectMake(50, 50, 10, 10)];
    _rdView.backgroundColor = [UIColor colorWithHexString:@"F8374E"];
    _rdView.layer.cornerRadius = _rdView.frame.size.width / 2;
    [self.view addSubview:_rdView];
    
    _tfView = [[UITextField alloc] initWithFrame:CGRectMake(100, 50, 100, 50)];
    _tfView.font = [UIFont safePingFangSCRegularFontOfSize:14];
    _tfView.text = @"抽取永久皮肤";
    _tfView.textColor = [UIColor colorWithHexString:@"FD4A47"];
    _tfView.textAlignment = NSTextAlignmentCenter;
    [_tfView sizeToFit];
    [self.view addSubview:_tfView];
    
    _imgView = [[UIImageView alloc] initWithFrame:CGRectMake(50, 150, 10, 10)];
    _imgView.backgroundColor = [UIColor colorWithHexString:@"FA5151"];
//    _imgView.layer.cornerRadius = _bgView.frame.size.height / 2;
    [self.view addSubview:_imgView];
    
    // Do any additional setup after loading the view.
}

- (IBAction)onClickSysFont:(id)sender {
    int size = 20;
    for (int i = 0; i < (int)fontWeightConfig.size(); ++ i) {
        CGFloat fontWeigth = fontWeightConfig[i];
        UIFont* font = [UIFont systemFontOfSize:size weight:fontWeigth];
        UILabel* lb = (UILabel*)[self.view viewWithTag:(100 + i)];
        if (!font || !lb) {
            continue;
        }
        
        lb.font = font;
    }
}

- (IBAction)onClickCusFont:(id)sender {
    int size = 20;
    for (int i = 0; i < (int)fontWeightConfig.size(); ++ i) {
        CGFloat fontWeigth = fontWeightConfig[i];
        
        UIFontDescriptorSymbolicTraits symbolic = 0;
        
        symbolic |= UIFontDescriptorTraitBold;
        symbolic |= UIFontDescriptorTraitItalic;

        UIFontDescriptor* fontDescriptor = [UIFontDescriptor fontDescriptorWithFontAttributes:@{
            UIFontDescriptorSizeAttribute:@(size),
       //     UIFontDescriptorFamilyAttribute: @"Arial",//
            UIFontDescriptorFamilyAttribute: @"Helvetica Neue",
      //      UIFontDescriptorFamilyAttribute: @".AppleSystemUIFont",
            UIFontDescriptorTraitsAttribute: @{
                UIFontWeightTrait: @(fontWeigth),
          //      UIFontSymbolicTrait: @(symbolic),
            },
        }];
        
        UIFont* font = [UIFont fontWithDescriptor:fontDescriptor size:size];
        
        UILabel* lb = (UILabel*)[self.view viewWithTag:(100 + i)];
        if (!font || !lb) {
            continue;
        }
        
        lb.font = font;
    }
}

@end
