#include "windowfactory.h"
#include <QDebug>

class AdminWindowProduct : public BaseWindow {
private:
    AdminWindow *window;
    int userId;

public:
    AdminWindowProduct(int userId) : window(nullptr), userId(userId) {}

    void initialize() override {
        window = new AdminWindow(nullptr, userId);
    }

    void showWindow() override {
        if (window) {
            window->show();
        }
    }

    ~AdminWindowProduct() {
        if (window) {
            window->deleteLater();
        }
    }
};

class CashierWindowProduct : public BaseWindow {
private:
    CashierWindow *window;
    User user;
public:
    CashierWindowProduct(const User &user) : window(nullptr), user(user) {}

    void initialize() override {
        window = new CashierWindow();
        window->setCashierId(user.id);
        window->setCashierName(user.login);
    }

    void showWindow() override {
        if (window) {
            window->show();
        }
    }

    ~CashierWindowProduct() {
        if (window) {
            window->deleteLater();
        }
    }
};

class ClientWindowProduct : public BaseWindow {
private:
    ClientWindow *window;
    int userId;

public:
    ClientWindowProduct(int userId) : window(nullptr), userId(userId) {}

    void initialize() override {
        window = new ClientWindow();
        window->setUserId(userId);
    }

    void showWindow() override {
        if (window) {
            window->show();
        }
    }

    ~ClientWindowProduct() {
        if (window) {
            window->deleteLater();
        }
    }
};

BaseWindow* WindowFactory::createWindow(const QString &role, User &user) {
    if (role == "Администратор") {
        BaseWindow *window = new AdminWindowProduct(user.id);
        window->initialize();
        return window;
    }
    else if (role == "Кассир") {
        BaseWindow *window = new CashierWindowProduct(user);
        window->initialize();
        return window;
    }
    else if (role == "Клиент") {
        BaseWindow *window = new ClientWindowProduct(user.id);
        window->initialize();
        return window;
    }

    qWarning() << "Unsupported role:" << role;
    return nullptr;
}

bool WindowFactory::isSupportedRole(const QString &role) {
    return (role == "Администратор" ||
            role == "Кассир" ||
            role == "Клиент");
}
