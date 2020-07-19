//
//  UIFont+UltronRTE.m
//  MMMain
//
//  Created by 郑钦洪 on 2019/10/8.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import "UIFont+UltronRTE.h"

@implementation UIFont (UltronRTE)
+ (UIFont *)safePingFangSCLightFontOfSize:(CGFloat)fontSize {
    UIFont *font = [UIFont systemFontOfSize:fontSize weight:UIFontWeightLight];
    return font;
}

+ (UIFont *)safePingFangSCRegularFontOfSize:(CGFloat)fontSize {
    UIFont *font = [UIFont systemFontOfSize:fontSize weight:UIFontWeightRegular];
    return font;
}

+ (UIFont *)safePingFangSCMediumFontOfSize:(CGFloat)fontSize {
    UIFont *font = [UIFont systemFontOfSize:fontSize weight:UIFontWeightMedium];
    return font;
}
@end
