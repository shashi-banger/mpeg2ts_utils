

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
        inp_pid = ts_get_pid(ts_pkt);

        if(pids_map.find(inp_pid) == pids_map.end())
        {
            /* Pid not found in map */
            curr_pid_info.inp_first_pts = INVALID_PTS;
            pids_map[inp_pid] = curr_pid_info;
        }

        if(ts_get_unitstart(ts_pkt))
        {
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            if(pes_has_pts(pes_data))
            {
                curr_pes_pts = pes_get_pts(pes_data);
                ofs << inp_pid << "," << curr_pes_pts/90 << "," << ifs.tellg() << std::endl;
            }
        }
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    std::cout << "Completed" << std::endl;
}


