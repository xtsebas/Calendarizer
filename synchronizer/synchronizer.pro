TEMPLATE   = lib
CONFIG     += staticlib c++17
TARGET     = synchronizer

HEADERS    += synchronizer.h synchronizer_peterson.h mutex_sync.h semaphore_sync.h
SOURCES    += synchronizer_peterson.cpp mutex_sync.cpp semaphore_sync.cpp
