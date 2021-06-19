#ifndef __PID_REMAP_HPP__
#define __PID_REMAP_HPP__


#include <stdlib.h>
#include <map>
#include <iostream>
#include <cstring>

class PidRemapper
{
    std::map<int, int> pid_remap_;
    int pmt_pid_;
    bool pat_found_;
    int  output_pmt_pid_;

    int scan_pat(unsigned char *ts_pkt);
    int update_pat(unsigned char *ts_pkt);
    int update_pmt(unsigned char *ts_pkt);
    int update_pkt(unsigned char *ts_pkt);

public:
    /**
     * @brief Construct a new Pid Remapper object
     * 
     * @param remap_data: String of the form "17:280,256:400,257:480" i.e. i
     * nput_pid:output_pid which is comma separated 
     */
    PidRemapper(int output_pmt_pid, std::string remap_data) : 
                pat_found_(false), output_pmt_pid_(output_pmt_pid)
    {
        char *token;
        char *rest = (char*)remap_data.c_str();
        char *t1, *t2;
        int inp_pid, out_pid;
        token = strtok_r(rest, ",", &rest);
        
        while (token) {
            t1 = strtok_r(token, ":", &token);
            inp_pid = atoi(t1);
            t2 = strtok_r(NULL, ":", &token);
            out_pid = atoi(t2);
            pid_remap_[inp_pid] = out_pid;
            std::cout << inp_pid << ":" << out_pid << std::endl;
            token = strtok_r(NULL, ",", &rest);
        }
    }
    bool isReadToRemap() {
        if (pat_found_) {
            return true;
        }
        else {
            return false;
        }
    }

    int process(unsigned char *ts_pkt);
};

#endif //__PID_REMAP_HPP__