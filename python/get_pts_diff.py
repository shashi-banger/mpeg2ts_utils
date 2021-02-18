import sys
import re

pid_pts_file = sys.argv[1]
scan_pid    = int(sys.argv[2])

prev_pts = 0
m_l = []

with open(pid_pts_file, "r") as fp:
    for line in fp:
        m = re.search ("^%d,([0-9]+),.*" %scan_pid, line)
        if m:
            #print "diff=%d" %(int(m.groups()[0]) - prev_pts)
            m_l.append(int(m.groups()[0]))
            prev_pts = int(m.groups()[0])

m_l.sort()
i=0
for p in m_l:
    if i==0:
        i+=1
        continue
    print "%d, %d" %(p-m_l[i-1], p)
    i+=1
