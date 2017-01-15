

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

#define USAGE "./get_pid_pts_fileoffset <input_ts_file> <out_csv_file>"

#define INVALID_PTS 0xffffffffffffULL

typedef struct __pid_info_t {
    uint64_t    inp_first_pts;
}pid_info_t;

typedef struct _pcr_derive_info {
    uint64_t    prev_pcr;
    uint64_t    prev_prev_pcr;
    uint64_t    bytes_bet_last_2_pcrs;
    uint64_t    bytes_since_last_pcr;
}pcr_derive_info;

static void check_and_note_pcr_info(pcr_derive_info  *pcr_info,  unsigned char *ts_pkt)
{
    uint64_t  pcr_ext;

    if(ts_has_adaptation(ts_pkt) && ts_get_adaptation(ts_pkt) > 0)
    {

        if(tsaf_has_pcr(ts_pkt))
        {
            pcr_info->prev_prev_pcr = pcr_info->prev_pcr;
            pcr_info->prev_pcr      = tsaf_get_pcr(ts_pkt);
            pcr_ext = tsaf_get_pcrext(ts_pkt);
            pcr_info->prev_pcr = (300 * pcr_info->prev_pcr) + pcr_ext;
            /*printf("%d %lld %lld %d\n", inp_pid, curr_pcr/(300 * 90),
              (int64_t)ifs.tellg()/188, ts_get_unitstart(ts_pkt));*/
            pcr_info->bytes_bet_last_2_pcrs = pcr_info->bytes_since_last_pcr;
            pcr_info->bytes_since_last_pcr = 0;
        }
        else
        {
            pcr_info->bytes_since_last_pcr += TS_SIZE;
        }
    }
    else
    {
        pcr_info->bytes_since_last_pcr += TS_SIZE;
    }
}

static uint64_t  get_cur_pcr(pcr_derive_info  *pcr_info)
{
    uint64_t   ret_val;
    if(pcr_info->prev_pcr == (uint64_t)-1)
    {
        ret_val = -1;
    }
    else if(pcr_info->prev_prev_pcr)
    {
        ret_val = pcr_info->prev_pcr;
    }
    else
    {
        ret_val = (uint64_t)(pcr_info->prev_pcr +
                             ((((double)(pcr_info->prev_pcr - pcr_info->prev_prev_pcr))/
                               pcr_info->bytes_bet_last_2_pcrs) * pcr_info->bytes_since_last_pcr) );
    }
    return ret_val;
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    std::ifstream  ifs;
    std::ofstream  ofs;
    char *inp_filename = argv[1];
    char *out_filename = argv[2];
    std::map<int, pid_info_t>  pids_map;
    unsigned char  ts_pkt[256];
    unsigned char  *pes_data;
    int  inp_pid;
    int  n_pkts = 0;
    pid_info_t     curr_pid_info;
    uint64_t       curr_pes_pts;
    uint64_t       curr_pes_dts;
    pcr_derive_info   pcr_info;
    uint64_t          cur_pcr;
    uint64_t          cur_pos;

    pcr_info.prev_pcr = -1;
    pcr_info.prev_prev_pcr = -1;


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
        cur_pos = ifs.tellg();
        ifs.read((char*)ts_pkt, 188);
        inp_pid = ts_get_pid(ts_pkt);

        check_and_note_pcr_info(&pcr_info, ts_pkt);
        if(pids_map.find(inp_pid) == pids_map.end())
        {
            /* Pid not found in map */
            curr_pid_info.inp_first_pts = INVALID_PTS;
            pids_map[inp_pid] = curr_pid_info;
        }

        if(ts_get_unitstart(ts_pkt))
        {
            curr_pes_pts = INVALID_PTS;
            curr_pes_dts = INVALID_PTS;
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            if(pes_has_pts(pes_data))
            {
                unsigned char *pes_pld;
                curr_pes_pts = pes_get_pts(pes_data);
                pes_pld = pes_payload(pes_data);
                //ofs << std::hex << pes_pld[0] << " " << pes_pld[1] << " " << pes_pld[2] << " " << std::endl;
                //ofs << pes_pld[0] << " " << pes_pld[1] << " " << pes_pld[2] << " " << std::endl;
            }
            if(pes_has_dts(pes_data))
            {
                curr_pes_dts = pes_get_dts(pes_data);
            }
            cur_pcr = get_cur_pcr(&pcr_info);
            if(cur_pcr != (uint64_t)(-1))
            {
                ofs << inp_pid << "," << cur_pcr/(300 * 90) << "," << curr_pes_pts/90 << "," <<
                    curr_pes_dts/90 <<
                    "," << cur_pos << std::endl;
            }
        }
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    std::cout << "Completed" << std::endl;
}


