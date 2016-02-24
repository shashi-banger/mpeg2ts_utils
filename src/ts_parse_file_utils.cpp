

#include <cstdlib>
#include <cstdio>
#include <map>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"
#include "bitstream/mpeg/psi/pat.h"
#include "bitstream/mpeg/psi/pmt.h"
#include "ts_parse_file_utils.hpp"


static void set_media_type(std::string& media_type, unsigned char s_type)
{
    switch(s_type)
    {
    case 0x02:
        media_type = VIDEO_MPEG2;
        break;

    case 0x1B:
        media_type = VIDEO_H264;
        break;

    case 0x03:
    case 0x04:
        media_type = AUDIO_MP2;
        break;

    case 0x0f:
        media_type = AUDIO_AAC;
        break;

    case 0x81:
    case 0x82:
        media_type = AUDIO_AC3;
        break;

    default:
        printf("UNKNOWN medai type received\n");
        break;
    }
}


uint64_t GetNearestPts(const char *filename, int  pid, uint64_t inp_pts)
{
    int            size;
    int            rd_off;
    unsigned char  *ts_pkt;
    uint64_t       pts = INVALID_PTS_VAL;
    uint64_t       ret_pts_val = INVALID_PTS_VAL;
    int            pts_found = 0;
    FILE           *fp;
    int            read_size = 100*188;
    unsigned int   learn_vid_half_pes_duration = 22*90;
    uint64_t       min_pts;
    uint64_t       min_pts_diff = INVALID_PTS_VAL;


    fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("GetNearestpts: Could not open %s\n", filename);
        return pts;
    }
    unsigned char  *buf = new unsigned char[read_size];
    while(1)
    {
        size = fread(buf, 1, read_size, fp);
        if(size == 0)
        {
            goto GetNearestPts_EXIT;
        }
        rd_off = 0;
        while(rd_off < size)
        {
            ts_pkt = buf + rd_off;
            //printf("pid=%d\n", ts_get_pid(ts_pkt));
            if(ts_get_pid(ts_pkt) == pid)
            {
                if(ts_get_unitstart(ts_pkt))
                {
                    unsigned char   *pes_data;
                    pes_data = ts_payload(ts_pkt);
                    if(pes_has_pts(pes_data))
                    {
                        unsigned int   diff;
                        pts = pes_get_pts(pes_data);
                        diff = (unsigned int)llabs((long long int)(pts - inp_pts));
                        printf("Diff=%d min=%ld\n", diff, min_pts_diff);

                        if(diff < min_pts_diff)
                        {
                            min_pts_diff = diff;
                            min_pts = pts;
                        }
                        if(diff < learn_vid_half_pes_duration)
                        {
                            //printf("Nearest pts %ld %ld", pts, inp_pts);
                            ret_pts_val = pts;
                            pts_found = 1;
                            break;
                        }
                    }
                }
            }
            rd_off += TS_SIZE;
        }
        if(pts_found)
            break;
    }

GetNearestPts_EXIT:
    if(pts_found == 0)
    {
        printf("Pts not found: %ld %ld\n", min_pts_diff, min_pts);
        if(min_pts_diff < 60*90)
        {
            ret_pts_val = min_pts;
        }
    }

    fclose(fp);

    delete[] buf;
    return ret_pts_val;
}

int GetPmtInfo(char *filename, int *pmt_pid_num,
               tsdemux_pmt_info *parsed_pmt, int  max_streams)
{
    int            size;
    int            rd_off;
    unsigned char  *ts_pkt;
    int            pat_found = 0;
    int            pmt_found = 0;
    int            pmt_pid;
    FILE           *fp;
    int            read_size = 100*188;


    fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("GetPmtInfo: Could not open %s file\n", filename);
        return -1;
    }
    unsigned char  *buf = new unsigned char[read_size];

    parsed_pmt->num_pids = 0;

    while(1)
    {
        size = fread(buf, 1, read_size, fp);
        if(size == 0)
        {
            goto GetPmtInfo_Exit;
        }
        rd_off = 0;
        while(rd_off < size)
        {
            ts_pkt = buf + rd_off;
            //printf("GetPmtInfo->pid=%d\n", ts_get_pid(ts_pkt));
            if(ts_get_pid(ts_pkt) == 0 && (pat_found == 0))
            {
                unsigned char    *pat;
                unsigned char    *prog;
                int              i = 0;
                /* PAT found */
                pat = ts_section(ts_pkt);

                for(i = 0; ; i++)
                {
                    prog = pat_get_program(pat, i);
                    if(prog == NULL)
                    {
                        printf("No program PMT found\n");
                        exit(3);
                    }
                    else if(patn_get_program(prog) != 0)
                    {
                        pmt_pid = patn_get_pid(prog);
                        *pmt_pid_num = pmt_pid;
                        break;
                    }
                }

                pat_found = 1;
                printf("pat found %d %x %x\n", pmt_pid, prog[0], prog[1]);
            }

            if(pat_found == 1 && ts_get_pid(ts_pkt) == pmt_pid)
            {
                unsigned char  *pmt;
                unsigned char  *p_pmt_n;
                int            pid_idx = 0;

                pmt = ts_section(ts_pkt);

                while(1)
                {
                    p_pmt_n = pmt_get_es(pmt, pid_idx);
                    if(p_pmt_n == NULL || pid_idx >= max_streams)
                    {
                        break;
                    }
                    parsed_pmt->strm_info[pid_idx].pid_num = pmtn_get_pid(p_pmt_n);
                    set_media_type(parsed_pmt->strm_info[pid_idx].media_type, pmtn_get_streamtype(p_pmt_n));
                    printf("PMT: pid=%d, type=%s\n",parsed_pmt->strm_info[pid_idx].pid_num,
                           parsed_pmt->strm_info[pid_idx].media_type.c_str());
                    pid_idx++;

                }
                parsed_pmt->num_pids = pid_idx;
                pmt_found = 1;
            }
            if((pat_found == 1) && (pmt_found == 1))
            {
                break;
            }
            rd_off += TS_SIZE;
        }
        if((pat_found == 1) && (pmt_found == 1))
        {
            break;
        }
    }

GetPmtInfo_Exit:
    delete[] buf;
    fclose(fp);
    return 0;
}

extern "C" int compare_uint64(const void *a, const void *b)
{
    int diff;
    diff = (int)*((uint64_t*)a) - *((uint64_t*)b);
    return diff;
}

int  GetFrameDuration(char *filename,  int pid)
{
    int            size;
    int            rd_off;
    unsigned char  *ts_pkt;
    int            pts_found = 0;
    FILE           *fp;
    int            read_size = 100*188;
    uint64_t       pts_arr[64];
    int            num_pts_vals = 0;
    int max_count = 0;
    int max_key = 0;
    std::map<int,int>   duration_to_count;
    int  duration;


    fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("GetFirstpts: Could not open %s file\n", filename);
        return max_key;
    }
    unsigned char  *buf = new unsigned char[read_size];
    while(1)
    {
        size = fread(buf, 1, read_size, fp);
        if(size == 0)
        {
            goto GetFrameDuration_Exit;
        }
        rd_off = 0;
        while(rd_off < size)
        {
            ts_pkt = buf + rd_off;
            //printf("pid=%d\n", ts_get_pid(ts_pkt));
            if(ts_get_pid(ts_pkt) == pid)
            {
                if(ts_get_unitstart(ts_pkt))
                {
                    unsigned char   *pes_data;
                    pes_data = ts_payload(ts_pkt);
                    if(pes_has_pts(pes_data))
                    {
                        pts_arr[num_pts_vals] = pes_get_pts(pes_data);
                        num_pts_vals++;
                        if(num_pts_vals > 20)
                        {
                            break;
                        }
                    }
                }
            }
            rd_off += TS_SIZE;
        }
        if(num_pts_vals > 20)
            break;
    }

    qsort(pts_arr, 20, sizeof(uint64_t), compare_uint64);


    for(int i = 1; i < num_pts_vals; i++)
    {
        duration = (pts_arr[i] - pts_arr[i-1])/90;
        if(duration_to_count.find(duration) != duration_to_count.end())
        {
            duration_to_count[duration]++;
        }
        else
        {
            duration_to_count[duration] = 1;
        }
    }



    for(std::map<int, int>::iterator it = duration_to_count.begin(); it != duration_to_count.end(); it++)
    {
        if(it->second > max_count)
        {
            max_count = it->second;
            max_key   = it->first;
        }
    }

GetFrameDuration_Exit:
    delete[] buf;
    fclose(fp);
    return max_key;
}

uint64_t  GetFirstPts(char *filename, int pid)
{
    int            size;
    int            rd_off;
    unsigned char  *ts_pkt;
    uint64_t       pts = INVALID_PTS_VAL;
    int            pts_found = 0;
    FILE           *fp;
    int            read_size = 100*188;


    fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("GetFirstpts: Could not open %s file\n", filename);
        return pts;
    }
    unsigned char  *buf = new unsigned char[read_size];
    while(1)
    {
        size = fread(buf, 1, read_size, fp);
        if(size == 0)
        {
            goto GetFirstPts_Exit;
        }
        rd_off = 0;
        while(rd_off < size)
        {
            ts_pkt = buf + rd_off;
            //printf("pid=%d\n", ts_get_pid(ts_pkt));
            if(ts_get_pid(ts_pkt) == pid)
            {
                if(ts_get_unitstart(ts_pkt))
                {
                    unsigned char   *pes_data;
                    pes_data = ts_payload(ts_pkt);
                    if(pes_has_pts(pes_data))
                    {
                        pts = pes_get_pts(pes_data);
                        pts_found = 1;
                        printf("First Pts = %ld\n", pts);
                        break;
                    }
                }
            }
            rd_off += TS_SIZE;
        }
        if(pts_found)
            break;
    }

GetFirstPts_Exit:
    delete[] buf;
    fclose(fp);
    return pts;
}



uint64_t  GetLastPts(char *filename, int pid)
{
    int            size;
    int            rd_off;
    unsigned char  *ts_pkt;
    uint64_t       pts = INVALID_PTS_VAL;
    int            pts_found = 0;
    FILE           *fp;
    int            read_size = 100*188;
    int            file_size;
    int            num_pts_vals_to_check = 10;
    uint64_t       max_pts = 0;


    fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("GetLastpts: Could not open file %s\n", filename);
        return pts;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    unsigned char  *buf = new unsigned char[read_size];
    while(1)
    {
        if(file_size > read_size)
        {
            fseek(fp, file_size - read_size, SEEK_SET);
            size = fread(buf, 1, read_size, fp);
            if(size != read_size)
            {
                printf("GetLastpts error: This should not have happened\n");
                break;
            }
            file_size -= read_size;
        }
        else
        {
            break;
        }

        rd_off = size - TS_SIZE;
        while(rd_off > 0)
        {
            ts_pkt = buf + rd_off;
            /*printf("GetLast pid=%d %d %d %x %x %x %x\n", ts_get_pid(ts_pkt), pid, file_size+rd_off,
                   ts_pkt[0], ts_pkt[1], ts_pkt[2], ts_pkt[3]);*/
            if(ts_get_pid(ts_pkt) == pid)
            {
                if(ts_get_unitstart(ts_pkt))
                {
                    unsigned char   *pes_data;
                    pes_data = ts_payload(ts_pkt);
                    if(pes_has_pts(pes_data))
                    {
                        pts = pes_get_pts(pes_data);
                        if(pts > max_pts)
                        {
                            max_pts = pts;
                        }
                        num_pts_vals_to_check--;
                        if(num_pts_vals_to_check == 0)
                        {
                            printf("Last Pts = %ld\n", max_pts);
                            break;
                        }
                    }
                }
            }
            rd_off -= TS_SIZE;
        }
        if(pts_found)
            break;
    }
    delete[] buf;
    fclose(fp);
    return max_pts;
}
