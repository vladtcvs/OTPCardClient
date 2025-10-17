QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lpcsclite
CONFIG += c++17
INCLUDEPATH += /usr/include/PCSC

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    errordialog.cpp \
    iso7816_pcsc.cpp \
    main.cpp \
    nfcget.cpp \
    otpcard.cpp \
    otpclient.cpp \
    secret_list_item.cpp

HEADERS += \
    errordialog.h \
    infc.h \
    iso7816_pcsc.h \
    nfcget.h \
    otpcard.h \
    otpclient.h \
    secret_list_item.h

FORMS += \
    errordialog.ui \
    otpclient.ui \
    pinenter.ui \
    secret_list_item.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
