QT += core gui

# For Qt 5 and above
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Project name and template type
TARGET = Calendarizer
TEMPLATE = app

# Source files
SOURCES += \
    gantt_canvas.cpp \
    main.cpp \
    mainwindow.cpp \
    sync_canvas.cpp \
    synclegend.cpp

    # Add other .cpp files here

# Header files
HEADERS += \
    gantt_canvas.h \
    mainwindow.h \
 \    # Add other .h files here
    sync_canvas.h \
    synclegend.h

# UI form files
FORMS += \
    mainwindow.ui \
    # Add other .ui files here

# Include directories
INCLUDEPATH += \
    . \
    ../../utils \
    ../../process \
    ../../synchronizer \
    ../../scheduler

LIBS += -L$$OUT_PWD/../../utils/debug -lutils
LIBS += -L$$OUT_PWD/../../process/debug -lprocess
LIBS += -L$$OUT_PWD/../../synchronizer/debug -lsynchronizer
LIBS += -L$$OUT_PWD/../../scheduler/debug -lscheduler

# Resource files (if any)
RESOURCES += \
    resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
