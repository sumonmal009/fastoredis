CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(ssd_cli)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src)

SET(HEADERS_SSDB
    src/SSDB.h
    src/SSDB_impl.h

    src/net/link.h

    src/util/bytes.h
)

SET(SOURCES_SSDB
    src/SSDB_impl.cpp

    src/net/link.cpp

    src/util/bytes.cpp
)

ADD_LIBRARY(ssd_cli OBJECT ${HEADERS_SSDB} ${SOURCES_SSDB})

