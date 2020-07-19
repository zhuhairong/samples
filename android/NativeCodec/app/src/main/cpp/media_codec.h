//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_MEDIA_CODEC_H
#define NATIVECODEC_MEDIA_CODEC_H
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#define MEDIACODEC_BITRATE_MODE_CQ  0 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ
#define MEDIACODEC_BITRATE_MODE_VBR 1 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR
#define MEDIACODEC_BITRATE_MODE_CBR 2 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR

#define MEDIACODEC_TIMEOUT_USEC 100000//us

class MediaCodec {
public:
    MediaCodec();
    ~MediaCodec();

protected:

};


#endif //NATIVECODEC_MEDIA_CODEC_H
