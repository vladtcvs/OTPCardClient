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
    pinmanager.cpp \
    secret_edit.cpp \
    secret_list_item.cpp \
    secret_new.cpp \
    secrets_list.cpp \
    show_totp.cpp \
    totpsecret.cpp \
    totpsecretsmanager.cpp

HEADERS += \
    errordialog.h \
    infc.h \
    iso7816_pcsc.h \
    nfcget.h \
    otpcard.h \
    otpclient.h \
    pinmanager.h \
    secret_edit.h \
    secret_list_item.h \
    secret_new.h \
    secrets_list.h \
    show_totp.h \
    totpsecret.h \
    totpsecretsmanager.h

FORMS += \
    errordialog.ui \
    otpclient.ui \
    pinenter.ui \
    secret_edit.ui \
    secret_list_item.ui \
    secret_new.ui \
    secrets_list.ui \
    show_totp.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
