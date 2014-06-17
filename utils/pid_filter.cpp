#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "bitstream/mpeg/ts.h"

#define USAGE "./pid_filter <input_ts_file> <out_ts_file> pid1 pid2 ..."

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
    std::vector<int>  pids_vec;
    int  i;
    unsigned char  ts_pkt[256];
    int  loc_pid;
    int  n_pkts = 0;

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


    for(i = 0; i < (argc - 3); i++) {
        //pids_vec[i] = atoi(argv[3+i]);
        pids_vec.push_back(atoi(argv[3+i]));
    }
    std::cout << "H1" << std::endl;

    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        loc_pid = ts_get_pid(ts_pkt);
        if(std::find(pids_vec.begin(), pids_vec.end(), loc_pid)!=pids_vec.end())
        {
            ofs.write((char*)ts_pkt, 188);
        }
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    std::cout << "Completed" << std::endl;
}
