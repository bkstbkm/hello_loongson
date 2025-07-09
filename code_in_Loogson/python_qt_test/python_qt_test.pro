QT       += core gui
QT += core gui websockets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# INCLUDEPATH += -I /usr/include/python3.8
# LIBS += -L /usr/lib/python3.8/config-3.8-x86_64-linux-gnu -lpython3.8


INCLUDEPATH += -I /home/toolchain/sysroots/loongarch64-Loongson-linux/usr/include/python3.7m
LIBS += -L /home/toolchain/sysroots/loongarch64-Loongson-linux/usr/lib/python3.7/config-3.7m-loongarch64-linux-gnu -lpython3.7m

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    cv_helloworld.py
