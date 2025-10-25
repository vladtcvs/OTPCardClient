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
    totpsecret.cpp \
    totpsecretsmanager.cpp \
    widget_card_params.cpp \
    widget_secret_edit.cpp \
    widget_secret_list_item.cpp \
    widget_secret_new.cpp \
    widget_secrets_list.cpp \
    widget_show_totp.cpp

HEADERS += \
    errordialog.h \
    infc.h \
    iso7816_pcsc.h \
    nfcget.h \
    otpcard.h \
    otpclient.h \
    pinmanager.h \
    totpsecret.h \
    totpsecretsmanager.h \
    widget_card_params.h \
    widget_secret_edit.h \
    widget_secret_list_item.h \
    widget_secret_new.h \
    widget_secrets_list.h \
    widget_show_totp.h

FORMS += \
    errordialog.ui \
    otpclient.ui \
    pinenter.ui \
    widget_card_params.ui \
    widget_secret_edit.ui \
    widget_secret_list_item.ui \
    widget_secret_new.ui \
    widget_secrets_list.ui \
    widget_show_totp.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
