
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(MPEG2TS_UTILS C CXX)

#SET(CMAKE_C_COMPILER arm-none-linux-gnueabi-gcc  )
#SET(CMAKE_CXX_COMPILER arm-none-linux-gnueabi-gcc )
SET(CMAKE_C_COMPILER gcc  )
SET(CMAKE_CXX_COMPILER gcc )
SET(CMAKE_CXX_FLAGS "-g -Wall" )
SET( CMAKE_CXX_FLAGS_RELEASE "-g -Wall")
ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 )

INCLUDE_DIRECTORIES(${MPEG2TS_UTILS_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${MPEG2TS_UTILS_SOURCE_DIR}/bin)

ADD_EXECUTABLE(pid_filter 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/pid_filter.cpp)
TARGET_LINK_LIBRARIES(pid_filter stdc++)

ADD_EXECUTABLE(pid_nullify 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/pid_nullify.cpp)
TARGET_LINK_LIBRARIES(pid_nullify stdc++)

ADD_EXECUTABLE(ts_re_time_stamp 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/ts_re_time_stamp.cpp)
TARGET_LINK_LIBRARIES(ts_re_time_stamp stdc++)

ADD_EXECUTABLE(get_pid_pts_fileoffset 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/get_pid_pts_fileoffset.cpp)
TARGET_LINK_LIBRARIES(get_pid_pts_fileoffset stdc++)

ADD_EXECUTABLE(get_pcr_fileoffset 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/get_pcr_fileoffset.cpp)
TARGET_LINK_LIBRARIES(get_pcr_fileoffset stdc++)

ADD_EXECUTABLE(extract_es 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/extract_es.cpp)
TARGET_LINK_LIBRARIES(extract_es stdc++)

ADD_EXECUTABLE(convert_192_to_188_bytes 
                ${MPEG2TS_UTILS_SOURCE_DIR}/utils/convert_192_to_188_bytes.cpp)
TARGET_LINK_LIBRARIES(convert_192_to_188_bytes stdc++)

