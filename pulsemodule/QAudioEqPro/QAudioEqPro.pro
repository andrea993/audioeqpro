TEMPLATE = app

QT += qml quick
CONFIG += c++11
LIBS += -lpulse -ldl
QMAKE_CXXFLAGS = -Wall -Wextra
QMAKE_CXXFLAGS_RELEASE = -O2
QMAKE_CXXFLAGS_DEBUG = -g -Wshadow

SOURCES += main.cpp \
           q_pamod.cpp \
    q_pulseutil.cpp \
    qmlutils.cpp \
    moduleobject.cpp

HEADERS += q_pamod.h \
    q_pulseutil.h \
    qmlutils.h \
    moduleobject.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
