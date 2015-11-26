

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

#define USAGE "./get_pcr_fileoffset <input_ts_file> <out_csv_file>"

#define INVALID_PTS 0xffffffffffffULL

typedef struct __pid_info_t {
    uint64_t    inp_first_pts;
}pid_info_t;

typedef enum {
    MP2TS_H264_UNKNOWN_FRAME = -1,
    MP2TS_H264_IDR_FRAME = 1,
    MP2TS_H264_P_FRAME,
    MP2TS_H264_B_FRAME,
}mp2ts_h264_frame_type_enum;

static int   mpeg2ts_h264_get_frame_type(unsigned char *h264_frame, int len)
{
    unsigned int   val;
    int            offset = 0;
    int            frame_type = MP2TS_H264_UNKNOWN_FRAME;

    val  = h264_frame[0] << 16;
    val |= h264_frame[1] << 8;
    val |= h264_frame[2];

    offset += 3;
    while(offset < len && (frame_type == MP2TS_H264_UNKNOWN_FRAME))
    {
        while(!((val & 0x00ffffff) == 0x000001))
        {
            val <<= 8;
            val |= h264_frame[offset];
            offset++;
        }
        if(h264_frame[offset] == 0x1)
        {
            frame_type = MP2TS_H264_B_FRAME;
        }
        else if(h264_frame[offset] == 0x21 || h264_frame[offset] == 0x41 ||
            h264_frame[offset] == 0x61)
        {
            frame_type = MP2TS_H264_P_FRAME;
        }
        else if(h264_frame[offset] == 0x25 || h264_frame[offset] == 0x45 ||
                h264_frame[offset] == 0x65)
        {
            frame_type = MP2TS_H264_IDR_FRAME;
        }
        else
        {
            /* Continue searching for next NAL */
            val <<= 8;
            val |= h264_frame[offset];
            offset++;
        }
    }
    return frame_type;
}

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    std::ifstream  ifs;
    char *inp_filename = argv[1];
    unsigned char  ts_pkt[256];
    int  n_pkts = 0;
    uint64_t       curr_pcr;
    uint64_t       pcr_ext;
    int            inp_pid;

    ifs.open(inp_filename, std::ios::in);
    if(!ifs.is_open()) {
        std::cout << "Unable to open " << inp_filename;
        exit(1);
    }

    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        inp_pid = ts_get_pid(ts_pkt);

        if(ts_has_adaptation(ts_pkt) && ts_get_adaptation(ts_pkt) > 0)
        {
            if(tsaf_has_pcr(ts_pkt))
            {
                curr_pcr = tsaf_get_pcr(ts_pkt);
                pcr_ext = tsaf_get_pcrext(ts_pkt);
                curr_pcr = (300 * curr_pcr) + pcr_ext;
                printf("%d %lld %lld %d\n", inp_pid, curr_pcr/(300 * 90), (int64_t)ifs.tellg(), ts_get_unitstart(ts_pkt));
            }
        }
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    std::cout << "Completed" << std::endl;
}


