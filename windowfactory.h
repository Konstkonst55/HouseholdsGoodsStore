// windowfactory.h - добавляем новую функцию
#ifndef WINDOWFACTORY_H
#define WINDOWFACTORY_H

#include <QWidget>
#include "authwindow.h"
#include "adminwindow.h"
#include "cashierwindow.h"
#include "clientwindow.h"
#include "database.h"

class BaseWindow : public QWidget {
    Q_OBJECT
public:
    BaseWindow(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~BaseWindow() {}
    virtual void initialize() = 0;
    virtual void showWindow() = 0;
};

class WindowFactory {
public:
    static BaseWindow* createWindow(const QString &role, User &user);
    static bool isSupportedRole(const QString &role);
    static QWidget* createAndShowWindow(const QString &role, User &user, QWidget* parent = nullptr);
};

#endif // WINDOWFACTORY_H
