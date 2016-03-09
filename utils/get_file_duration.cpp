
#include <cstdio>
#include <cstring>
#include "ts_parse_file_utils.hpp"


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage: ./get_file_duration inp.ts\n");
        exit(1);
    }
    int  vid_pid;
    int  pmt_pid_num;
    uint64_t  first_pts;
    uint64_t  last_pts;
    int total_duration = 0;

    tsdemux_pmt_info pmt_info;
    GetPmtInfo(argv[1], &pmt_pid_num,
               &pmt_info, 32);

    for(int i = 0; i < pmt_info.num_pids; i++)
    {
        if(strncmp(pmt_info.strm_info[i].media_type.c_str(),
                   "video", strlen("video")) == 0)
        {
            vid_pid = pmt_info.strm_info[i].pid_num;
            break;
        }
    }

    first_pts = GetFirstPts(argv[1], vid_pid);
    last_pts = GetLastPts(argv[1], vid_pid);
    total_duration = (last_pts - first_pts)/90;
    total_duration += GetFrameDuration(argv[1], vid_pid);
    printf("Duration=%d\n", total_duration);

}
