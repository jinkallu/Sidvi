QT       += core sql
TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += main.cpp \
    sidview.cpp \
    sidviewDict.cpp \
    viewserver.cpp \
    viewserverDict.cpp

QTPLUGIN += QSQLMYSQL

INCLUDEPATH += /opt/root/include/root/
LIBS += $(shell /opt/root/bin/root-config --libs)
QMAKE_CXXFLAGS += $(shell /opt/root/bin/root-config --ldflags)


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/release/ -lCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/debug/ -lCore
else:unix: LIBS += -L$$PWD/../../../../opt/root/lib/root/ -lCore

INCLUDEPATH += $$PWD/../../../../opt/root/lib/root
DEPENDPATH += $$PWD/../../../../opt/root/lib/root

HEADERS += \
    sidview.h \
    sidviewLinkDef.h \
    viewserver.h \
    viewserverLinkDef.h \
    sidviewDict.h \
    viewserverDict.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/release/ -lGui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/debug/ -lGui
else:unix: LIBS += -L$$PWD/../../../../opt/root/lib/root/ -lGui

INCLUDEPATH += $$PWD/../../../../opt/root/lib/root
DEPENDPATH += $$PWD/../../../../opt/root/lib/root

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/release/ -lGraf
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/debug/ -lGraf
else:unix: LIBS += -L$$PWD/../../../../opt/root/lib/root/ -lGraf

INCLUDEPATH += $$PWD/../../../../opt/root/lib/root
DEPENDPATH += $$PWD/../../../../opt/root/lib/root

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/release/ -lNet
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../opt/root/lib/root/debug/ -lNet
else:unix: LIBS += -L$$PWD/../../../../opt/root/lib/root/ -lNet

INCLUDEPATH += $$PWD/../../../../opt/root/lib/root
DEPENDPATH += $$PWD/../../../../opt/root/lib/root

unix:!macx: LIBS += -L$$PWD/../../../../../opt/root/lib/root/ -lCore

INCLUDEPATH += $$PWD/../../../../../opt/root/lib/root
DEPENDPATH += $$PWD/../../../../../opt/root/lib/root
