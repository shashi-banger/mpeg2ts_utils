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
print(pid_data)

v = []
a = []

for l in f:
    w = l.split(',')
    l_pid = int(w[0])
    if l_pid in pids:
        pid_data[l_pid]['x'].append(int(w[4]))
        pid_data[l_pid]['y'].append(int(w[2]))
#plt.plot(x,y, label='video pts', x1, y1, label='audio pts')

pts = {}
pts_diff = {}
max_diff = 0
min_diff = 100000000000000000000000
for p in pids:
    pts[p] = np.array(pid_data[p]['y'])
    pts[p].sort()
    pts_diff[p] = pts[p][1:pts[p].size] - pts[p][0:pts[p].size-1]
    if max(pts_diff[p]) > max_diff:
        max_diff = max(pts_diff[p])
    if min(pts_diff[p]) < min_diff:
        min_diff = min(pts_diff[p])
    print(pts_diff[p])



plt.ylim([(min_diff - 100),(max_diff +200)])
lab = []
for pid in pids:
   print(pid)
   #s = plt.plot(np.array(pid_data[pid]['y'][1:])/1000., pts_diff[pid], label='%d pts' % pid)
   s = plt.plot(pts[pid][1:]/1000., pts_diff[pid], label='%d pts' % pid)
   lab.append(s)

plt.grid(True)
plt.xlabel('Pts val')
plt.ylabel('PTS Diff')
plt.legend()
plt.show()
