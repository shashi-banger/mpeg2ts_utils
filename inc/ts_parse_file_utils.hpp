
#ifndef __TS_PARSE_FILE_UTILS_HPP__
#define __TS_PARSE_FILE_UTILS_HPP__

#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <string>
//#include "ts_common.h"

#define INVALID_PTS_VAL 0xffffffffffffffffULL

#define AUDIO_AAC   "audio/aac"
#define AUDIO_MP2   "audio/mpeg"
#define AUDIO_AC3   "audio/ac3"
#define VIDEO_H264  "video/h264"
#define VIDEO_MPEG2 "videp/mpeg"


typedef struct tsdemux_pmt_info {
    int                  num_pids;
    struct tsdemux_strm_info{
        int   pid_num;
        std::string  media_type;
    }  strm_info[32];
}tsdemux_pmt_info;

int GetPmtInfo(char *filename, int *pmt_pid_num, tsdemux_pmt_info *parsed_pmt, int  max_streams);
uint64_t  GetFirstPts(char *filename, int pid);
uint64_t  GetLastPts(char *filename, int pid);
uint64_t GetNearestPts(const char *filename, int  pid, uint64_t inp_pts);

/* Estimates the frame duration of the pid passes. typically used to ficgure out
 * whether the frame duration is 40ms(PAL) or around 33ms for NTSC
 */
int  GetFrameDuration(char *filename,  int pid);

#endif //__TS_PARSE_FILE_UTILS_HPP__
