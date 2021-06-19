#include "pid_remap.hpp"
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"
#include "bitstream/mpeg/psi/pat.h"
#include "bitstream/mpeg/psi/psi.h"
#include "bitstream/mpeg/psi/pmt.h"

int PidRemapper::scan_pat(unsigned char *ts_pkt) {
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
            pmt_pid_ = patn_get_pid(prog);
            break;
        }
    }

    if (i > 1) {
        std::cout << "Multiprogram ts stream is not supported for remapping" << std::endl;
        exit(1);
    }
    
    pat_found_ = true;
    printf("pat found pmt_pid=%d\n", pmt_pid_);
    return 0;
}

int PidRemapper::update_pat(unsigned char *ts_pkt) {
    unsigned char    *prog;
    unsigned char *pat;
    //int pmt_pid;
    int i;
    
    
    pat = ts_section(ts_pkt);
    
    for(i = 0; ; i++)
    {
        prog = pat_get_program(pat, i);
        if(patn_get_program(prog) != 0)
        {
            //pmt_pid = patn_get_pid(prog);
            patn_set_pid(prog, (uint16_t)output_pmt_pid_);
            break;
        }
    }
    psi_set_crc(pat);
    return 0;
}

int PidRemapper::update_pmt(unsigned char *ts_pkt) {
    unsigned char  *pmt;
    unsigned char  *p_pmt_n;
    int            pid_idx = 0;
    int            pid_num;
    int            pcr_pid_num;

    pid_num = ts_get_pid(ts_pkt);
    
    // Modifify the ts packet pid to match the output_pmt_pid_
    ts_set_pid(ts_pkt, (uint16_t)output_pmt_pid_);

    pmt = ts_section(ts_pkt);
    pcr_pid_num = pmt_get_pcrpid(pmt);
    if (pid_remap_.find(pcr_pid_num) != pid_remap_.end()) {
        pmt_set_pcrpid(pmt, (uint16_t)pid_remap_[pcr_pid_num]);
    }

    while(1)
    {
        p_pmt_n = pmt_get_es(pmt, pid_idx);
        // max_streams in a program limited to 16
        if(p_pmt_n == NULL || pid_idx >= 16)
        {
            break;
        }
        pid_num = (int)pmtn_get_pid(p_pmt_n);
        
        if (pid_remap_.find(pid_num) != pid_remap_.end()) {
            pmtn_set_pid(p_pmt_n, (uint16_t)pid_remap_[pid_num]);
        }
        pid_idx++;
    }
    psi_set_crc(pmt);
    return 0;
}

int PidRemapper::update_pkt(unsigned char *ts_pkt) {
    int pid_num;

    pid_num = ts_get_pid(ts_pkt);

    if (pid_remap_.find(pid_num) != pid_remap_.end()) {
        ts_set_pid(ts_pkt, (uint16_t)pid_remap_[pid_num]);
    }
    return 0;
}

int PidRemapper::process(unsigned char *ts_pkt) {
    int curr_pid = (int)ts_get_pid(ts_pkt);
    
    if (!pat_found_) {
        if (curr_pid == 0) {
            scan_pat(ts_pkt);
        }
    }
    else {
        if (curr_pid == 0) {
            update_pat(ts_pkt);
        }
        else if (curr_pid == pmt_pid_) {
            update_pmt(ts_pkt);
        }
        else {
            update_pkt(ts_pkt);
        }
    }
    return 0;
}