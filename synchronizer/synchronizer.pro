TEMPLATE   = lib
CONFIG     += static c++17
TARGET     = synchronizer
INCLUDEPATH += $$PWD ../process ../utils
HEADERS    += synchronizer.h mutex_sync.h semaphore_sync.h
SOURCES    += synchronizer.cpp mutex_sync.cpp semaphore_sync.cpp
