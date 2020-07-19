//
//  UIColor+HexColors.m
//
//  Created by JustinYan on 14/12/25.
//  Copyright (c) 2014年 Tencent. All rights reserved.
//

#import "UIColor+MWEExtension.h"

@implementation UIColor (HexColors)

+ (UIColor *)colorWithHexString:(NSString *)hexString {
    return [self colorWithHexString:hexString alpha:1.0f];
}

+ (UIColor *)colorWithHexString:(NSString *)hexString alpha:(CGFloat)alpha {
    if ([hexString hasPrefix:@"#"]) {
        hexString = [hexString substringFromIndex:1];
    }

    if ([hexString length] != 6) {
        return nil;
    }

    // Brutal and not-very elegant test for non hex-numeric characters
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"[^a-fA-F|0-9]" options:0 error:NULL];
    NSUInteger match = [regex numberOfMatchesInString:hexString options:NSMatchingReportCompletion range:NSMakeRange(0, [hexString length])];

    if (match != 0) {
        return nil;
    }

    NSRange rRange = NSMakeRange(0, 2);
    NSString *rComponent = [hexString substringWithRange:rRange];
    NSUInteger rVal = 0;
    NSScanner *rScanner = [NSScanner scannerWithString:rComponent];
    [rScanner scanHexInt:(unsigned int *)&rVal];
    float rRetVal = (float)rVal / 254;

    NSRange gRange = NSMakeRange(2, 2);
    NSString *gComponent = [hexString substringWithRange:gRange];
    NSUInteger gVal = 0;
    NSScanner *gScanner = [NSScanner scannerWithString:gComponent];
    [gScanner scanHexInt:(unsigned int *)&gVal];
    float gRetVal = (float)gVal / 254;

    NSRange bRange = NSMakeRange(4, 2);
    NSString *bComponent = [hexString substringWithRange:bRange];
    NSUInteger bVal = 0;
    NSScanner *bScanner = [NSScanner scannerWithString:bComponent];
    [bScanner scanHexInt:(unsigned int *)&bVal];
    float bRetVal = (float)bVal / 254;

    return [UIColor colorWithRed:rRetVal green:gRetVal blue:bRetVal alpha:alpha];
}

+ (NSString *)hexValuesFromUIColor:(UIColor *)color {
    if (!color) {
        return nil;
    }

    if (color == [UIColor whiteColor]) {
        // Special case, as white doesn't fall into the RGB color space
        return @"ffffff";
    }

    CGFloat red;
    CGFloat blue;
    CGFloat green;
    CGFloat alpha;

    [color getRed:&red green:&green blue:&blue alpha:&alpha];

    int redDec = (int)(red * 255);
    int greenDec = (int)(green * 255);
    int blueDec = (int)(blue * 255);

    NSString *returnString = [NSString stringWithFormat:@"%02x%02x%02x", (unsigned int)redDec, (unsigned int)greenDec, (unsigned int)blueDec];

    return returnString;
}

+ (UIColor *)randomColor {
    CGFloat hue = (arc4random() % 256 / 256.0); //  0.0 to 1.0
    CGFloat saturation = (arc4random() % 128 / 256.0) + 0.5; //  0.5 to 1.0, away from white
    CGFloat brightness = (arc4random() % 128 / 256.0) + 0.5; //  0.5 to 1.0, away from black
    UIColor *color = [UIColor colorWithHue:hue saturation:saturation brightness:brightness alpha:1];
    return color;
}

@end
