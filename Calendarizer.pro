TEMPLATE = subdirs
CONFIG   += ordered

SUBDIRS += \
    process \
    utils \
    scheduler \
    synchronizer \
    visualization/Calendarizer

utils.depends                      = process
scheduler.depends                  = process utils
synchronizer.depends               = process utils
visualization/Calendarizer.depends = process utils scheduler synchronizer
