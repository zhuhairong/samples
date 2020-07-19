//
//  bmp.cpp
//  mediasdk
//
//  Created by cort xu on 2020/8/25.
//  Copyright © 2020 cortxu. All rights reserved.
//

#include "bmp.h"

#define BmpHdrSize  14
#define BmpInfoSize 40

#pragma pack(2)
struct BmpHdr {
    uint16_t    bh_type = 0x4D42;//文件的类型，固定为BM
    uint32_t    bh_size = 0;//位图文件大小
    uint16_t    bh_reserved1 = 0;//保留
    uint16_t    bh_reserved2 = 0;//保留
    uint32_t    bh_offbits = BmpHdrSize + BmpInfoSize;//文件头到图像数据之间的偏移量，头与数据部分是可选的调色板区域
};

struct BmpInfo {
    uint32_t    bi_size = BmpInfoSize;//BmpInfo 的大小，固定40字节
    uint32_t    bi_width = 0;//
    int32_t     bi_height = 0;//负数：正向，正数：倒向
    uint16_t    bi_planes = 1;//颜色平面数，总是1
    uint16_t    bi_bitcount = 32;//比特数/像素，有1、4、8、16、24、32
    uint32_t    bi_compression = 0;//0:BI_RGB,1:BI_RLE8,2:BI_RLE4,3:BI_BITFIELDS,4:BI_JPEG,5:BI_PNG
    uint32_t    bi_sizeimg = 0;//图像大小，字节为单位；格式为RGB时，可设置为0；
    int32_t     bi_xp_pm = 0;//水平分辨率，像素/米，有符号整数
    int32_t     bi_yp_pm = 0;//垂直分辨率，像素/米，有符号整数
    uint32_t    bi_clr_used = 0;//说明位图实际使用的颜色表示中的索引数，为0则说明使用所有颜色
    uint32_t    bi_clr_important = 0;//图像显示有重要影响的颜色索引数目，如果是0，表示都重要；
};

#pragma pack()

static void FillBmpHdr(const BmpHdr& hdr, uint8_t hdr_buff[]) {
    hdr_buff[0] = (uint8_t)(hdr.bh_type);
    hdr_buff[1] = (uint8_t)(hdr.bh_type >> 8);
    
    hdr_buff[2] = (uint8_t)(hdr.bh_size);
    hdr_buff[3] = (uint8_t)(hdr.bh_size >> 8);
    hdr_buff[4] = (uint8_t)(hdr.bh_size >> 16);
    hdr_buff[5] = (uint8_t)(hdr.bh_size >> 24);

    hdr_buff[10] = (uint8_t)(hdr.bh_offbits);
    hdr_buff[11] = (uint8_t)(hdr.bh_offbits >> 8);
    hdr_buff[12] = (uint8_t)(hdr.bh_offbits >> 16);
    hdr_buff[13] = (uint8_t)(hdr.bh_offbits >> 24);
}

static void FillBmpInfo(const BmpInfo& info, uint8_t info_buff[]) {
    info_buff[0] = (uint8_t)(info.bi_size);
    info_buff[1] = (uint8_t)(info.bi_size >> 8);
    info_buff[2] = (uint8_t)(info.bi_size >> 16);
    info_buff[3] = (uint8_t)(info.bi_size >> 24);

    info_buff[4] = (uint8_t)(info.bi_width);
    info_buff[5] = (uint8_t)(info.bi_width >> 8);
    info_buff[6] = (uint8_t)(info.bi_width >> 16);
    info_buff[7] = (uint8_t)(info.bi_width >> 24);

    info_buff[8] = (uint8_t)(info.bi_height);
    info_buff[9] = (uint8_t)(info.bi_height >> 8);
    info_buff[10] = (uint8_t)(info.bi_height >> 16);
    info_buff[11] = (uint8_t)(info.bi_height >> 24);

    info_buff[12] = (uint8_t)(info.bi_planes);
    info_buff[13] = (uint8_t)(info.bi_planes >> 8);

    info_buff[14] = (uint8_t)(info.bi_bitcount);
    info_buff[15] = (uint8_t)(info.bi_bitcount >> 8);

    info_buff[16] = (uint8_t)(info.bi_compression);
    info_buff[17] = (uint8_t)(info.bi_compression >> 8);
    info_buff[18] = (uint8_t)(info.bi_compression >> 16);
    info_buff[19] = (uint8_t)(info.bi_compression >> 24);
}

bool BmpEncode(const uint8_t* data, const uint32_t size, const uint32_t width, const uint32_t height, const std::string& file_path) {
    bool result = false;
    FILE* fp = NULL;
    
    do {
        BmpHdr bmp_hdr;
        bmp_hdr.bh_size = BmpHdrSize + BmpInfoSize + size;

        BmpInfo bmp_info;
        bmp_info.bi_width = width;
        bmp_info.bi_height = -height;
        bmp_info.bi_bitcount = 32;

        if (!(fp = fopen(file_path.c_str(), "wb"))) {
            break;
        }

        uint8_t hdr_buff[BmpHdrSize] = {0};
        FillBmpHdr(bmp_hdr, hdr_buff);

        uint8_t info_buff[BmpInfoSize] = {0};
        FillBmpInfo(bmp_info, info_buff);
        
        size_t ret = 0;

        if ((ret = fwrite(hdr_buff, BmpHdrSize, 1, fp)) <= 0) {
            break;
        }
        
        if ((ret = fwrite(info_buff, BmpInfoSize, 1, fp)) <= 0) {
            break;
        }

        uint32_t pos = 0;
        while (pos < size) {
            uint32_t lf_size = size - pos;
            uint32_t slice_size = lf_size > 1024 ? 1024 : lf_size;
            if ((ret = fwrite(data + pos, 1, slice_size, fp)) <= 0) {
                goto encode_end;
            }

            pos += ret;
        }
        
        result = true;
    } while (false);
    
encode_end:
    if (fp) {
        fclose(fp);
    }
    
    return result;
}
