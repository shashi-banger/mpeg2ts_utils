#include<stdio.h>
#include "bitstream/mpeg/ts.h"
#include "bitstream/mpeg/pes.h"
#include "bitstream/mpeg/psi.h"


int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char  ts_pkt[512];
    int   pid;
    unsigned char *payload;
    unsigned char *section;
    bool        flag;

    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("Could not open file %s\n", argv[1]);
        exit(1);
    }

    fread(ts_pkt, 1, 188, fp);
    pid = ts_get_pid(ts_pkt);
    printf("pid = %d\n", pid);

    section = ts_payload(ts_pkt) + 1;

    printf("%x %x %x %x\n", section[0], section[1],
                         section[2], section[3]);

    flag = psi_validate(section);

    if(flag)
    {
        printf("Valid psi\n");
    }
    else
    {
        printf("invalid psi_data\n");
    }

    flag = psi_check_crc(section);
    if(flag)
    {
        printf("valid CRC\n");
    }
    else
    {
        printf("Invalid CRD\n");
    }
    
    


}