


#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"

#define USAGE "./get_pid_pts_fileoffset <input_ts_file> <pid> <out_es>"

#define INVALID_PTS 0xffffffffffffULL

typedef struct __pid_info_t {
    uint64_t    inp_first_pts;
}pid_info_t;

int main(int argc, char *argv[])
{
    if(argc < 4) {
        std::cout << USAGE << std::endl;
        exit(1);
    }
    int            es_pid = atoi(argv[2]);
    std::ifstream  ifs;
    std::ofstream  ofs;
    char *inp_filename = argv[1];
    char *out_filename = argv[3];
    std::map<int, pid_info_t>  pids_map;
    unsigned char  ts_pkt[256];
    unsigned char  *pes_data;
    unsigned char  *payload;
    int  inp_pid;
    int  n_pkts = 0;
    pid_info_t     curr_pid_info;
    uint64_t       curr_pes_pts = 0;
    uint64_t       prev_pes_pts = 0;
    int            got_pes_start = 0;
    int            cur_es_size = 0;

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

    while(!ifs.eof()) {
        ifs.read((char*)ts_pkt, 188);
        inp_pid = ts_get_pid(ts_pkt);

        if(pids_map.find(inp_pid) == pids_map.end())
        {
            /* Pid not found in map */
            curr_pid_info.inp_first_pts = INVALID_PTS;
            pids_map[inp_pid] = curr_pid_info;
        }

        if(ts_get_unitstart(ts_pkt) && (inp_pid == es_pid))
        {
            /* If pes present */
            pes_data = ts_payload(ts_pkt);
            payload = pes_payload(pes_data);

            got_pes_start = 1;
            prev_pes_pts = curr_pes_pts;
            curr_pes_pts = pes_get_pts(pes_data);

            printf("pts=%llu, diff = %d, last_pes_size=%d\n", curr_pes_pts/90, (int)((curr_pes_pts - prev_pes_pts)/90),  cur_es_size);

            /*printf("======Got pes start %x %x %x %x %x %x %x %x\n", pes_data[0], pes_data[1], pes_data[2], pes_data[3], pes_data[4], pes_data[5],
                                                            pes_data[6], pes_data[7]);
            printf("======Got pes start %x %x %x %x %x %x %x %x\n", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5],
                                                            payload[6], payload[7]); */
            if(pes_get_dataalignment(pes_data))
            {
                printf("##########Got data alignmednt\n");
            }
            ofs.write((char*)payload, (188 - (payload - ts_pkt)));
            cur_es_size = 0;
            cur_es_size += 188 - (int)(payload - ts_pkt);
#if 0
            if(pes_has_pts(pes_data))
            {
                curr_pes_pts = pes_get_pts(pes_data);
                ofs << inp_pid << "," << curr_pes_pts/90 << "," << ifs.tellg() << std::endl;
            }
            else
            {
                ofs << inp_pid << "," << INVALID_PTS/90 << "," << ifs.tellg() << std::endl;
            }
#endif //0
        }
        else if(inp_pid == es_pid)
        {
            if(got_pes_start == 1)
            {
                payload = ts_payload(ts_pkt);
                /*printf("Got ts packet %x %x %x %x %x %x %x %x\n", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5],
                                                            payload[6], payload[7]);*/
                if(payload[0] == 0xb && payload[1] == 0x77)
                {
                    printf("GOT AC3 START\n");
                }
                ofs.write((char*)payload, (188 - (payload - ts_pkt)));
                cur_es_size += 188 - (int)(payload - ts_pkt);
            }
        }
        n_pkts++;
        //std::cout << n_pkts <<std::endl;
    }
    std::cout << "Completed" << std::endl;
}



