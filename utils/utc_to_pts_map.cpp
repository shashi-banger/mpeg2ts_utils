#include <unistd.h>
#include <cstdio>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

static int64_t get_pts_from_buf(unsigned char *buf, int len, int pid_num)
{
    int curr_len = 0;
    unsigned char *ts_pkt;
    unsigned char *pes_data;
    int cur_pid;
    int64_t ret_pts = -1;
    while(curr_len < len)
    {
        ts_pkt = buf + curr_len;
        cur_pid = ts_get_pid(ts_pkt);
        if(cur_pid == pid_num)
        {
            if(ts_get_unitstart(ts_pkt))
            {
                pes_data = ts_payload(ts_pkt);
                if(pes_has_pts(pes_data))
                {
                    ret_pts = pes_get_pts(pes_data);
                }
            }
        }
        curr_len += 188;
    }
    return ret_pts;
}

int main(int argc, char *argv[])
{
    //int fd = open(stdin, O_RDONLY);
    int fd = 0; //stdin
    uint8_t buf[188*100];
    int nb;
    int64_t  pts;
    int      pid = atoi(argv[1]);
    struct timeval tv;

    printf("utc,pts\n");
    while(1)
    {
        nb = read(fd, buf, 188*7);
        pts = get_pts_from_buf(buf, nb, pid);
        if(pts != -1)
        {
            gettimeofday(&tv, NULL);
            printf("%ld.%06ld, %f\n", tv.tv_sec, tv.tv_usec, pts/90000.0);
        }
    }
    return 0;
}
