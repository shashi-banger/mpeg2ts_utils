import re
import sys
from operator import itemgetter


if __name__ == "__main__":

    if len (sys.argv) != 3:
        print "USAGE: %s <video pid> <get_pid_pts_fileoffset OUTPUT csv>" %sys.argv[0]

    video_pid = int (sys.argv[1])
    out_line = []
    last_pes_size = 0
    i= 0

    with open (sys.argv[2], "r") as fp:
        for line in fp:
            i = i+1
            m =  re.search ("pts=([0-9]+), diff = [0-9]+, last_pes_size=([0-9]+)", line)
            if m:
                if i != 1:
                    out_line[len(out_line)-1].append (int(last_pes_size))
                last_pes_size = int(m.groups()[1])
                out_line.append ( [ int(m.groups()[0]) ] )

    out_line = out_line[:-1]
    f = sorted (out_line, key=itemgetter(1), reverse=True)

    for l in f:
        print l
