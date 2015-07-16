
#include <cstdlib>
#include <cstdio>

#define MAX_BUF_SIZE   600*192

int main(int argc, char *argv[])
{
    FILE   *fp_in;
    FILE   *fp_out;
    int    rd_bytes;
    char   buf[MAX_BUF_SIZE];
    int    num_pkts;
    char   *loc_buf;
    int     num_bytes_written = 0;

    if(argc < 2)
    {
        printf("Usage: ./convert_192_to_188_bytes ,in_file> out_ts_file\n");
        std::exit(1);
    }

    printf("%s %s\n", argv[1], argv[2]);
    fp_in = fopen(argv[1], "rb");
    fp_out = fopen(argv[2], "wb");

    while(1)
    {
        rd_bytes = fread(buf, 1, MAX_BUF_SIZE, fp_in);
        if(rd_bytes == 0) break;
        num_pkts = rd_bytes/192;
        loc_buf = buf;
        for(int i = 0; i < num_pkts; i++)
        {
            fwrite(loc_buf + 4, 1, 188, fp_out);
            num_bytes_written += 188;
            loc_buf += 192;
        }
    }
    printf("num_bytes_written=%d\n", num_bytes_written);
}
