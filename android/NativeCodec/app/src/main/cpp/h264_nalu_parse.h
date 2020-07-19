//
// Created by cort xu on 2021/8/18.
//

#ifndef NATIVECODEC_H264_NALU_PARSE_H
#define NATIVECODEC_H264_NALU_PARSE_H
#include <string>
#include <list>


typedef unsigned int uint32_t;
typedef  unsigned int UINT;
typedef  unsigned char BYTE;
typedef  unsigned long DWORD;


enum VideoFrameType
{
    kVideoUnknowFrame = 0xFF,   // 8bits
    kVideoIFrame = 0,
    kVideoPFrame,
    kVideoBFrame,
    kVideoPFrameSEI = 3,
    kVideoIDRFrame,
    kVideoSPSFrame,
    kVideoPPSFrame,
    kVideoHeaderFrame,
    kVideoEncodedDataFrame
};

enum NaluType {
    kNaluTypeSLICE           = 1,
    kNaluTypeDPA             = 2,
    kNaluTypeDPB             = 3,
    kNaluTypeDPC             = 4,
    kNaluTypeIDR_SLICE       = 5,
    kNaluTypeSEI             = 6,
    kNaluTypeSPS             = 7,
    kNaluTypePPS             = 8,
    kNaluTypeAUD             = 9,
    kNaluTypeEND_SEQUENCE    = 10,
    kNaluTypeEND_STREAM      = 11,
    kNaluTypeFILLER_DATA     = 12,
    kNaluTypeSPS_EXT         = 13,
    kNaluTypeAUXILIARY_SLICE = 19,
    kNaluTypeFF_IGNORE       = 0xff0f001
};

typedef struct _NaluUnit
{
    int nal_unit_type;
    int mark_size;

    std::string data;
    _NaluUnit()
            : nal_unit_type(0)
            , mark_size(0)
    {

    }
}NaluUnit;

class H264NaluParse
{
public:
    H264NaluParse(void);
    virtual ~H264NaluParse(void);

public:
    bool foundSei() const { return m_foundSei; }
    bool foundSps() const { return m_foundSps; }
    bool foundPps() const { return m_foundPps; }

public:
    static bool sps(BYTE * buf,unsigned int nLen,int &width,int &height,int &fps);

public:
    void reset();
    bool parse(std::list<NaluUnit>& naluList, FILE* fp);
    bool parse(std::list<NaluUnit>& naluList, char* data, uint32_t size);

private:
    bool parseNaluFromBuf(std::list<NaluUnit>& naluList, char* data, uint32_t size);
    bool parseNaluFromNalu(std::list<NaluUnit>& naluList, char* data, uint32_t size);

private:
    std::string m_strPending;
    bool m_foundSei;
    bool m_foundSps;
    bool m_foundPps;
    int m_markSize;
    int m_preMarkSize;
    int m_headCnt;
};


#endif //NATIVECODEC_H264_NALU_PARSE_H
