//
//  UIColor+HexColors.h
//
//  Created by JustinYan on 14/12/25.
//  Copyright (c) 2014年 Tencent. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIColor (HexColors)

+ (UIColor *)colorWithHexString:(NSString *)hexString;
+ (UIColor *)colorWithHexString:(NSString *)hexString alpha:(CGFloat)alpha;
+ (NSString *)hexValuesFromUIColor:(UIColor *)color;
+ (UIColor *)randomColor;

@end
