QT += core gui sql widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addproductform.cpp \
    addsupplyform.cpp \
    adminwindow.cpp \
    cashierwindow.cpp \
    clientcartform.cpp \
    clientwindow.cpp \
    database.cpp \
    main.cpp \
    authwindow.cpp \
    salesreceiptform.cpp

HEADERS += \
    addproductform.h \
    addsupplyform.h \
    adminwindow.h \
    authwindow.h \
    cashierwindow.h \
    clientcartform.h \
    clientwindow.h \
    database.h \
    salesreceiptform.h

FORMS += \
    addproductform.ui \
    addsupplyform.ui \
    adminwindow.ui \
    authwindow.ui \
    cashierwindow.ui \
    clientcartform.ui \
    clientwindow.ui \
    salesreceiptform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
