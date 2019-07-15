mpeg2ts_utils
=============

Has source to build various mpeg2 ts utilities.

pid_filter: Reads a ts file and will create an output ts file containing only the
pids mentioned on the command line.

    Usage: ./pid_filter   input_ts_file   out_ts_file   pid1   pid2 ...


pid_nullify: Reads a ts file and will create an output ts file in which the pids
mentioned on command line are nullified(i.e. converted to NULL packests).

    Usage: ./pid_nullify   input_ts_file    out_ts_file   pid1   pid2 ...


ts_re_time_stamp: Reads a ts file and changes the pts values of all pes present
in ts such that the first pes in file has a new PTS corresponding to the one
given on command line.

    Usage: ./ts_re_time_stamp   input_ts_file     out_ts_file     start_pts


get_pid_pts_fileoffset: Reads a ts file and creates a csv with each line having
following entry: pid, PTS_in_ms , byte_offset_in_file

    Usage: /get_pid_pts_fileoffset input_ts_file out_csv_file

utc_to_pts_map: Creates csv of utc time and pts. Data can be received from UDP using
socat as follows

     Usage: socat -u udp-recv:5678,setsockopt-int=1:2:1 - | ./bin/utc_to_pts_map 2064
