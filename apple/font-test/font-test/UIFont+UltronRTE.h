//
//  UIFont+UltronRTE.h
//  MMMain
//
//  Created by 郑钦洪 on 2019/10/8.
//  Copyright © 2019 Tencent. All rights reserved.
//

#import <UIKit/UIFont.h>

NS_ASSUME_NONNULL_BEGIN

@interface UIFont (UltronRTE)
+ (UIFont *)safePingFangSCLightFontOfSize:(CGFloat)fontSize;
+ (UIFont *)safePingFangSCRegularFontOfSize:(CGFloat)fontSize;
+ (UIFont *)safePingFangSCMediumFontOfSize:(CGFloat)fontSize;
@end

NS_ASSUME_NONNULL_END
