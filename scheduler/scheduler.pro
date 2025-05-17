TEMPLATE    = lib
CONFIG      += static c++17
TARGET      = scheduler
INCLUDEPATH += $$PWD ../process ../utils
HEADERS     += scheduler.h \
               SJF/sjf_scheduler.h \
               RoundRobin/rr_scheduler.h \
               FIFO/fifo_scheduler.h \
               Priority/priority_scheduler.h \
               SRTF/srtf_scheduler.h
SOURCES     += scheduler.cpp \
               SJF/sjf_scheduler.cpp \
               RoundRobin/rr_scheduler.cpp \
               FIFO/fifo_scheduler.cpp \
               Priority/priority_scheduler.cpp \
               SRTF/srtf_scheduler.cpp
