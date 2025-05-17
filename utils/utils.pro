TEMPLATE   = lib
CONFIG     += static c++17
TARGET     = utils
INCLUDEPATH += $$PWD ../process
HEADERS    += file_loader.h metrics.h
SOURCES    += file_loader.cpp metrics.cpp
