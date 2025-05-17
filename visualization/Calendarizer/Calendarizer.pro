QT += core gui

# For Qt 5 and above
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Project name and template type
TARGET = Calendarizer
TEMPLATE = app

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    # Add other .cpp files here

# Header files
HEADERS += \
    mainwindow.h \
    # Add other .h files here

# UI form files
FORMS += \
    mainwindow.ui \
    # Add other .ui files here

# Include directories
INCLUDEPATH += \
    . \
    # Add other include paths if needed

# Resource files (if any)
RESOURCES += \
    resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target