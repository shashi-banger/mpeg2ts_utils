/**
 * @file pid_remap_app.cpp
 * @author shashidhar@amagi.com
 * @brief The application takes an input ts file and a pid remap string of the following kind
 * "<pid_i1>:<pid_o2>,pid_i2:pid_o2>". Each pid_in and pid_on are in values. The
 * application maps the input pid_i(n) present in the input ts file to pid_o(n). The output
 * of the pplication is another ts file which has pds pid_o(n). The pat and pmt packets are
 * also updated 
 * @version 0.1
 * @date 2021-05-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <fstream>
#include "pid_remap.hpp"


#define USAGE "./pid_remap_app <input_ts_file> <out_ts_file> <remap_string(e.g. \"17:100,256:481,257:492\")>"

int main(int argc, char *argv[]) {
    if(argc < 4) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    std::ifstream  ifs;
    std::ifstream  ifs1;
    std::ofstream  ofs;
    char *inp_filename = argv[1];
    char *out_filename = argv[2];
    unsigned char  ts_pkt[256];
    std::string remap_string = std::string(argv[3]);

    PidRemapper  *pid_remapper;
    pid_remapper = new PidRemapper(remap_string);

    ifs.open(inp_filename, std::ios::in);
    if(!ifs.is_open()) {
        std::cout << "Unable to open " << inp_filename;
        exit(1);
    }
    /**
     * First scan pat and ensure PidRemapper gets it and is ready to do pid remapping
     * 
     */
    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);

        pid_remapper->process(ts_pkt);
        if(pid_remapper->isReadToRemap()) {
            std::cout << "Ready to remap\n";
            break;
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
    /* Pid remapping. This is done in place*/
    while(1) {
        ifs.read((char*)ts_pkt, 188);
        if (ifs.eof()) {
            break;
        }
        /* process call below modifies the pids in ts_pkt */ 
        pid_remapper->process(ts_pkt);
        ofs.write((char*)ts_pkt, 188);
        
    }
    ofs.close();

    return 0;
}