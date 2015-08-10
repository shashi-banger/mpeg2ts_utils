
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

#define USAGE "./ts_re_time_stamp <input_ts_file> <out_ts_file> <start_pts>"

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
        ts_get_pid(ts_pkt);

        if(ts_get_unitstart(ts_pkt))
        {
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            if(pes_has_pts(pes_data))
            {
                curr_pes_pts = pes_get_pts(pes_data);
                g_file_first_pts = (g_file_first_pts == INVALID_PTS) ?
                    curr_pes_pts : g_file_first_pts;
                new_pes_pts  = out_start_file_pts +
                                (int64_t)(curr_pes_pts - g_file_first_pts);
                pes_set_pts(pes_data, new_pes_pts);
            }
        }


        if(ts_has_adaptation(ts_pkt))
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
    std::cout << "Completed" << std::endl;
}

