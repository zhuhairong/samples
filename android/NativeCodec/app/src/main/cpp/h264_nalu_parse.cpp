//
// Created by cort xu on 2021/8/18.
//

#include "h264_nalu_parse.h"

H264NaluParse::H264NaluParse(void)
{
  reset();
}

H264NaluParse::~H264NaluParse(void)
{

}

UINT Ue(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
  //计算0bit的个数
  UINT nZeroNum = 0;
  while (nStartBit < nLen * 8)
  {
    if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
    {
      break;
    }
    nZeroNum++;
    nStartBit++;
  }
  nStartBit ++;

  //计算结果
  DWORD dwRet = 0;
  for (UINT i=0; i<nZeroNum; i++)
  {
    dwRet <<= 1;
    if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
    {
      dwRet += 1;
    }
    nStartBit++;
  }
  return (1 << nZeroNum) - 1 + dwRet;
}

int Se(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
  int UeVal=Ue(pBuff,nLen,nStartBit);
  double k=UeVal;
  int nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
  if (UeVal % 2==0)
    nValue=-nValue;
  return nValue;
}

DWORD u(UINT BitCount,BYTE * buf,UINT &nStartBit)
{
  DWORD dwRet = 0;
  for (UINT i=0; i<BitCount; i++)
  {
    dwRet <<= 1;
    if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
    {
      dwRet += 1;
    }
    nStartBit++;
  }
  return dwRet;
}

/**
 * H264的NAL起始码防竞争机制
 *
 * @param buf SPS数据内容
 *
 * @无返回值
 */
void de_emulation_prevention(BYTE* buf,unsigned int* buf_size)
{
  int i=0,j=0;
  BYTE* tmp_ptr=NULL;
  unsigned int tmp_buf_size=0;
  int val=0;

  tmp_ptr=buf;
  tmp_buf_size=*buf_size;
  for(i=0;i<(tmp_buf_size-2);i++)
  {
    //check for 0x000003
    val=(tmp_ptr[i]^0x00) +(tmp_ptr[i+1]^0x00)+(tmp_ptr[i+2]^0x03);
    if(val==0)
    {
      //kick out 0x03
      for(j=i+2;j<tmp_buf_size-1;j++)
        tmp_ptr[j]=tmp_ptr[j+1];

      //and so we should devrease bufsize
      (*buf_size)--;
    }
  }
}

bool H264NaluParse::sps(BYTE * buf,unsigned int nLen,int &width,int &height,int &fps) {
  UINT StartBit=0;
  fps=0;
  de_emulation_prevention(buf,&nLen);

  int forbidden_zero_bit=u(1,buf,StartBit);
  int nal_ref_idc=u(2,buf,StartBit);
  int nal_unit_type=u(5,buf,StartBit);
  if(nal_unit_type==7)
  {
    int profile_idc=u(8,buf,StartBit);
    int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
    int constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
    int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
    int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
    int reserved_zero_4bits=u(4,buf,StartBit);
    int level_idc=u(8,buf,StartBit);

    int seq_parameter_set_id=Ue(buf,nLen,StartBit);

    if( profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 144 )
    {
      int chroma_format_idc=Ue(buf,nLen,StartBit);
      if( chroma_format_idc == 3 )
        int residual_colour_transform_flag=u(1,buf,StartBit);
      int bit_depth_luma_minus8=Ue(buf,nLen,StartBit);
      int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit);
      int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);
      int seq_scaling_matrix_present_flag=u(1,buf,StartBit);

      int seq_scaling_list_present_flag[8];
      if( seq_scaling_matrix_present_flag )
      {
        for( int i = 0; i < 8; i++ ) {
          seq_scaling_list_present_flag[i]=u(1,buf,StartBit);
        }
      }
    }
    int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);
    int pic_order_cnt_type=Ue(buf,nLen,StartBit);
    if( pic_order_cnt_type == 0 )
      int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);
    else if( pic_order_cnt_type == 1 )
    {
      int delta_pic_order_always_zero_flag=u(1,buf,StartBit);
      int offset_for_non_ref_pic=Se(buf,nLen,StartBit);
      int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);
      int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

      int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
      for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
        offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
      delete [] offset_for_ref_frame;
    }
    int num_ref_frames=Ue(buf,nLen,StartBit);
    int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);
    int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
    int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);

    width=(pic_width_in_mbs_minus1+1)*16;
    height=(pic_height_in_map_units_minus1+1)*16;

    int frame_mbs_only_flag=u(1,buf,StartBit);
    if(!frame_mbs_only_flag)
      int mb_adaptive_frame_field_flag=u(1,buf,StartBit);

    int direct_8x8_inference_flag=u(1,buf,StartBit);
    int frame_cropping_flag=u(1,buf,StartBit);
    if(frame_cropping_flag)
    {
      int frame_crop_left_offset=Ue(buf,nLen,StartBit);
      int frame_crop_right_offset=Ue(buf,nLen,StartBit);
      int frame_crop_top_offset=Ue(buf,nLen,StartBit);
      int frame_crop_bottom_offset=Ue(buf,nLen,StartBit);
    }
    int vui_parameter_present_flag=u(1,buf,StartBit);
    if(vui_parameter_present_flag)
    {
      int aspect_ratio_info_present_flag=u(1,buf,StartBit);
      if(aspect_ratio_info_present_flag)
      {
        int aspect_ratio_idc=u(8,buf,StartBit);
        if(aspect_ratio_idc==255)
        {
          int sar_width=u(16,buf,StartBit);
          int sar_height=u(16,buf,StartBit);
        }
      }
      int overscan_info_present_flag=u(1,buf,StartBit);
      if(overscan_info_present_flag)
        int overscan_appropriate_flagu=u(1,buf,StartBit);
      int video_signal_type_present_flag=u(1,buf,StartBit);
      if(video_signal_type_present_flag)
      {
        int video_format=u(3,buf,StartBit);
        int video_full_range_flag=u(1,buf,StartBit);
        int colour_description_present_flag=u(1,buf,StartBit);
        if(colour_description_present_flag)
        {
          int colour_primaries=u(8,buf,StartBit);
          int transfer_characteristics=u(8,buf,StartBit);
          int matrix_coefficients=u(8,buf,StartBit);
        }
      }
      int chroma_loc_info_present_flag=u(1,buf,StartBit);
      if(chroma_loc_info_present_flag)
      {
        int chroma_sample_loc_type_top_field=Ue(buf,nLen,StartBit);
        int chroma_sample_loc_type_bottom_field=Ue(buf,nLen,StartBit);
      }
      int timing_info_present_flag=u(1,buf,StartBit);

      if(timing_info_present_flag)
      {
        int num_units_in_tick=u(32,buf,StartBit);
        int time_scale=u(32,buf,StartBit);
        fps=time_scale/num_units_in_tick;
        int fixed_frame_rate_flag=u(1,buf,StartBit);
        if(fixed_frame_rate_flag)
        {
          fps=fps/2;
        }
      }
    }
    return true;
  }
  else
    return false;
}

void H264NaluParse::reset()
{
  m_strPending.clear();
  m_foundSei = false;
  m_foundSps = false;
  m_foundPps = false;
  m_markSize = 0;
  m_preMarkSize = 0;
  m_headCnt = 0;
}

bool H264NaluParse::parse(std::list<NaluUnit>& naluList, FILE* fp) {
  if (!fp) {
    return false;
  }

  const uint32_t kSize = 102400;
  char buff[kSize] = {0};

  int ret = fread(buff, 1, kSize, fp);
  if (ret <= 0) {
    return false;
  }

  return parse(naluList, buff, ret);
}

bool H264NaluParse::parse(std::list<NaluUnit>& naluList, char* data, uint32_t size) {
  return parseNaluFromBuf(naluList, data, size);
}

bool H264NaluParse::parseNaluFromBuf(std::list<NaluUnit>& naluList, char* data, uint32_t size) {
  m_strPending.append(data, size);
  bool bNewHead = false;
  int nMark = m_strPending.size();
  char* pStart = (char*)m_strPending.c_str();
  char* pPos = pStart;
  while(nMark > 4)
  {
    if (pPos[0] == 0x00 &&
        pPos[1] == 0x00 &&
        pPos[2] == 0x00 &&
        pPos[3] == 0x01)
    {
      m_markSize = 4;
      bNewHead = true;
      ++ m_headCnt;
    }
    else if (pPos[0] == 0x00 &&
             pPos[1] == 0x00 &&
             pPos[2] == 0x01)
    {
      m_markSize = 3;
      bNewHead = true;
      ++ m_headCnt;
    }
    else
    {
      bNewHead = false;
      m_preMarkSize = m_markSize;
    }
    if (bNewHead)
    {
      int naluSize = pPos - pStart;
      if (m_headCnt > 1)
      {
        if (naluSize > 0)
        {
          NaluUnit nalu;
          nalu.data = std::string(pStart, naluSize);
          nalu.mark_size = m_preMarkSize;
          nalu.nal_unit_type = nalu.data[m_preMarkSize] & 0x1f;
          if (nalu.nal_unit_type == kNaluTypeSEI) {
            m_foundSei = true;
          } else if (nalu.nal_unit_type == kNaluTypeSPS) {
            m_foundSps = true;
          } else if (nalu.nal_unit_type == kNaluTypePPS) {
            m_foundPps = true;
          }
          naluList.push_back(nalu);
          m_strPending = m_strPending.substr(naluSize);
          pStart = (char*)m_strPending.c_str();
        }
        pPos = pStart + m_markSize;
        nMark = m_strPending.size() - m_markSize;
        continue;
      }

      if (m_headCnt == 1)
      {
        if (naluSize != 0)
        {
          int nGap = pPos - pStart;
          printf("invalid header %u!\r\n", nGap);
          m_strPending = m_strPending.substr(nGap);
          pStart = (char*)m_strPending.c_str();
        }
        pPos = pStart + m_markSize;
        nMark = m_strPending.size() - m_markSize;
        continue;
      }
    }
    else
    {
      ++ pPos;
      -- nMark;
    }
  }
  return naluList.size() > 0 || !m_strPending.empty();
}

bool H264NaluParse::parseNaluFromNalu(std::list<NaluUnit>& naluList, char* data, uint32_t size)
{
  bool bNewHead = false;
  uint32_t nMark = size;
  uint32_t nHeadCnt = 0;
  uint32_t nMarkSize = 0;
  uint32_t nPreMarkSize = 0;
  char* pStart = data;
  char* pPos = pStart;
  while(nMark > 4)
  {
    if (pPos[0] == 0x00 &&
        pPos[1] == 0x00 &&
        pPos[2] == 0x00 &&
        pPos[3] == 0x01)
    {
      nMarkSize = 4;
      bNewHead = true;
      ++ nHeadCnt;
    }
    else if (pPos[0] == 0x00 &&
             pPos[1] == 0x00 &&
             pPos[2] == 0x01)
    {
      nMarkSize = 3;
      bNewHead = true;
      ++ nHeadCnt;
    }
    else
    {
      bNewHead = false;
      nPreMarkSize = nMarkSize;
    }
    if (bNewHead)
    {
      int naluSize = pPos - pStart;
      if (nHeadCnt > 1)
      {
        if (naluSize > 0)
        {
          NaluUnit nalu;
          nalu.data = std::string(pStart, naluSize);
          nalu.mark_size = nPreMarkSize;
          nalu.nal_unit_type = nalu.data[nPreMarkSize] & 0x1f;
          if (nalu.nal_unit_type == kNaluTypeSEI) {
              m_foundSei = true;
          } else if (nalu.nal_unit_type == kNaluTypeSPS) {
              m_foundSps = true;
          } else if (nalu.nal_unit_type == kNaluTypePPS) {
              m_foundPps = true;
          }
          naluList.push_back(nalu);
          pStart = pPos;
          nMark = size - (pPos - data);
        }
        pPos = pStart + nMarkSize;
        nMark -= nMarkSize;
        continue;
      }

      if (nHeadCnt == 1)
      {
        if (naluSize != 0)
        {
          int nGap = pPos - pStart;
          printf("H264NaluParse invalid header %u!\r\n", nGap);
          pStart = pPos;
          nMark = size - (pPos - data);
        }
        pPos = pStart + nMarkSize;
        nMark -= nMarkSize;
        continue;
      }
    }
    else
    {
      ++ pPos;
      -- nMark;
    }
  }

  NaluUnit nalu;
  nalu.data = std::string(pStart, size - (pStart - data));
  nalu.mark_size = nPreMarkSize;
  nalu.nal_unit_type = nalu.data[nPreMarkSize] & 0x1f;
  if (nalu.nal_unit_type == kNaluTypeSEI) {
    m_foundSei = true;
  } else if (nalu.nal_unit_type == kNaluTypeSPS) {
    m_foundSps = true;
  } else if (nalu.nal_unit_type == kNaluTypePPS) {
    m_foundPps = true;
  }
  naluList.push_back(nalu);
  return naluList.size() > 0;
}