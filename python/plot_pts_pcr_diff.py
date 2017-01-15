import matplotlib.pyplot as plt
import re
import sys
import numpy as np

# Output of ts_parse application to be fed to this script which will
# result in e.g.
# ./out/bin/PC/ts_parse player3_config.cfg /home2/sb_media/Bloomberg_locals/EA002096_with_PETP.ts 482 481 o.aac o.h264 >EA002096_withpetp.txt
#

f = open(sys.argv[1])

pids = []


for s in sys.argv[2:]:
    pids.append(int(s))

pid_data = {int(p) : {'x' : [], 'y' : []} for p in pids}
print pid_data
v = []
a = []

for l in f:
    w = l.split(',')
    l_pid = int(w[0])
    if l_pid in pids:
        diff = int(w[2]) - int(w[1])
        pid_data[l_pid]['x'].append(int(w[4]))
        pid_data[l_pid]['y'].append(int(diff))

#plt.plot(x,y, label='video pts', x1, y1, label='audio pts')

lab = []
for pid in pids:
   print pid
   s = plt.plot(pid_data[pid]['x'], pid_data[pid]['y'], label='%d pts' % pid)
   lab.append(s)

plt.grid(True)
plt.xlabel('File offset in bytes')
plt.ylabel('PTS-PCR Difference')
plt.legend()
plt.show()
