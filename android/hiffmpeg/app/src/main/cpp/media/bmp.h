//
//  bmp.hpp
//  mediasdk
//
//  Created by cort xu on 2020/8/25.
//  Copyright © 2020 cortxu. All rights reserved.
//

#pragma once
#include <stdint.h>
#include <string>


bool BmpEncode(const uint8_t* data, const uint32_t size, const uint32_t width, const uint32_t height, const std::string& file_path);