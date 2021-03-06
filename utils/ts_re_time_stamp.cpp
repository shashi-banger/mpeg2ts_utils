
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

// NOTE: duration_sec is optional. If not present the input ts will not be truncated otherwise
// the input file will be truncated to the given input duration in seconds
#define USAGE "./ts_re_time_stamp <input_ts_file> <out_ts_file> <start_pts(should correspond to first video pts of new ts)> <duration_sec(e..g 4.8, 5.75 etc)>"

#define INVALID_PTS 0xffffffffffffULL

static uint64_t  g_file_first_pts = INVALID_PTS;

typedef struct __pid_info_t {
    uint64_t    inp_first_pts;
}pid_info_t;

int main(int argc, char *argv[])
{
    if(argc < 4) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    std::ifstream  ifs;
    std::ofstream  ofs;
    char *inp_filename = argv[1];
    char *out_filename = argv[2];
    uint64_t   out_start_file_pts = strtoull(argv[3], NULL, 10);
    unsigned char  ts_pkt[256];
    unsigned char  *pes_data;
    int  n_pkts = 0;
    uint64_t       curr_pes_pts;
    uint64_t       new_pes_pts;
    int pid;
    double  output_duration = -1.0;
    int  vid_pid_num;

    ifs.open(inp_filename, std::ios::in);
    if(!ifs.is_open()) {
        std::cout << "Unable to open " << inp_filename;
        exit(1);
    }

    if(argc == 5) {
        // Duration input is present in command line
        output_duration = atof(argv[4]);
    }

    /* Scan input for the first PTS correspnfing to input reference pid */
    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        pid = ts_get_pid(ts_pkt);

        if(ts_get_unitstart(ts_pkt))
        {
            uint8_t  stream_id;
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            stream_id = pes_get_streamid(pes_data);
            if((stream_id >= PES_STREAM_ID_VIDEO_MPEG) &  (stream_id < PES_STREAM_ID_ECM)) 
            {
                vid_pid_num = pid;
                if(pes_has_pts(pes_data))
                {
                    curr_pes_pts = pes_get_pts(pes_data);
                    g_file_first_pts = (g_file_first_pts == INVALID_PTS) ?
                        curr_pes_pts : g_file_first_pts;
                    std::cout << "First pts: " << g_file_first_pts << std::endl;
                    break;
                }
            }
            // else continue
        }
    }
    ifs.close();

    ifs.open(inp_filename, std::ios::in);
    if(!ifs.is_open()) {
        std::cout << "Unable to open " << inp_filename;
        exit(1);
    }

    ofs.open(out_filename, std::ios::out);
    if(!ofs.is_open()) {
        std::cout << "Unable to open " << out_filename;
        exit(1);
    }

    std::cout << "H1" << std::endl;

    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        pid = ts_get_pid(ts_pkt);

        if(ts_get_unitstart(ts_pkt))
        {
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            if(pes_has_pts(pes_data))
            {
                curr_pes_pts = pes_get_pts(pes_data);
                /*g_file_first_pts = (g_file_first_pts == INVALID_PTS) ?
                    curr_pes_pts : g_file_first_pts;*/
                new_pes_pts  = out_start_file_pts +
                                (int64_t)(curr_pes_pts - g_file_first_pts);
                pes_set_pts(pes_data, new_pes_pts);

                if(pid == vid_pid_num) {
                    double dur_sec;
                    dur_sec = ((curr_pes_pts - g_file_first_pts) & 0x1ffffffff)/90000.0;
                    if(output_duration > 0. &&  dur_sec > output_duration) {
                        std::cout << curr_pes_pts << "  " << g_file_first_pts << std::endl;
                        std::cout << "Exiting " << output_duration << "s  " << dur_sec << std::endl;
                        break;
                    }
                }

            }
            if(pes_has_dts(pes_data))
            {
                uint64_t       curr_pes_dts;
                uint64_t       new_pes_dts;
                curr_pes_dts = pes_get_dts(pes_data);
                new_pes_dts  = out_start_file_pts +
                    (int64_t)(curr_pes_dts - g_file_first_pts);
                pes_set_dts(pes_data, new_pes_dts);
            }
        }

        if(ts_has_adaptation(ts_pkt) && ts_get_adaptation(ts_pkt) > 0)
        {
            if(tsaf_has_pcr(ts_pkt))
            {
                uint64_t   pcr_val;
                uint64_t   pcr_base;
                uint64_t   pcr_ext;

                pcr_base = tsaf_get_pcr(ts_pkt);
                pcr_ext  = tsaf_get_pcrext(ts_pkt);
                pcr_val = (pcr_base * 300) + pcr_ext;

                if(g_file_first_pts != INVALID_PTS)
                {
                    /* Get new pcr val */
                    pcr_val = (pcr_val) + (out_start_file_pts - g_file_first_pts) * 300;
                    tsaf_set_pcr(ts_pkt,  ((pcr_val/300) & (uint64_t)0x1ffffffff));
                    tsaf_set_pcrext(ts_pkt,  (pcr_val % 300));
                }
            }
        }

        if(g_file_first_pts != INVALID_PTS)
            ofs.write((char*)ts_pkt, 188);
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    ofs.close();
    std::cout << "Completed" << std::endl;
}

