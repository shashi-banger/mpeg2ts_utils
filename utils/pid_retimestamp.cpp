#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

#define USAGE "./pid_retimestamp <input_ts_file> <out_ts_file> pid1 offset_ms"

int main(int argc, char *argv[])
{
    if(argc < 5) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    std::ifstream  ifs;
    std::ofstream  ofs;
    char *inp_filename = argv[1];
    char *out_filename = argv[2];
    int  pid_to_change;
    int  i;
    unsigned char  ts_pkt[256];
    unsigned char  *pes_data;
    int  loc_pid;
    int   offset_ms;
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

    pid_to_change = atoi(argv[3]);
    offset_ms     = atoi(argv[4]);

    std::cout << "H1" << std::endl;

    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        loc_pid = ts_get_pid(ts_pkt);
        if(loc_pid == pid_to_change)
        {
            if(ts_get_unitstart(ts_pkt))
            {
                /* If pes present */
                pes_data = ts_payload(ts_pkt);
                if(pes_has_pts(pes_data))
                {
                    curr_pes_pts = pes_get_pts(pes_data);
                    new_pes_pts = curr_pes_pts + (offset_ms * 90);
                    pes_set_pts(pes_data, new_pes_pts);
                    printf("Mod pts %llu %llu\n", curr_pes_pts, new_pes_pts);
                }
            }
        }
        ofs.write((char*)ts_pkt, 188);
        n_pkts++;
    }
    std::cout << "Completed" << std::endl;
}
