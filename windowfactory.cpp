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
        window->setAttribute(Qt::WA_DeleteOnClose);
    }

    void showWindow() override {
        if (window) {
            window->show();
            window->raise();
            window->activateWindow();
        }
    }

    ~AdminWindowProduct() {

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
        window->setAttribute(Qt::WA_DeleteOnClose);
    }

    void showWindow() override {
        if (window) {
            window->show();
            window->raise();
            window->activateWindow();
        }
    }

    ~CashierWindowProduct() {

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
        window->setAttribute(Qt::WA_DeleteOnClose);
    }

    void showWindow() override {
        if (window) {
            window->show();
            window->raise();
            window->activateWindow();
        }
    }

    ~ClientWindowProduct() {

    }
};

BaseWindow* WindowFactory::createWindow(const QString &role, User &user) {
    BaseWindow* windowProduct = nullptr;

    if (role == "Администратор") {
        windowProduct = new AdminWindowProduct(user.id);
    }
    else if (role == "Кассир") {
        windowProduct = new CashierWindowProduct(user);
    }
    else if (role == "Клиент") {
        windowProduct = new ClientWindowProduct(user.id);
    }

    if (windowProduct) {
        windowProduct->initialize();
        windowProduct->deleteLater();
    }
    else {
        qWarning() << "Unsupported role:" << role;
    }

    return windowProduct;
}

QWidget* WindowFactory::createAndShowWindow(const QString &role, User &user, QWidget* parent) {
    QWidget* window = nullptr;

    if (role == "Администратор") {
        window = new AdminWindow(parent, user.id);
    }
    else if (role == "Кассир") {
        CashierWindow* cashierWindow = new CashierWindow(parent);
        cashierWindow->setCashierId(user.id);
        cashierWindow->setCashierName(user.login);
        window = cashierWindow;
    }
    else if (role == "Клиент") {
        ClientWindow* clientWindow = new ClientWindow(parent);
        clientWindow->setUserId(user.id);
        window = clientWindow;
    }

    if (window) {
        window->show();
        window->raise();
        window->activateWindow();
    }
    else {
        qWarning() << "Unsupported role:" << role;
    }

    return window;
}

bool WindowFactory::isSupportedRole(const QString &role) {
    return (role == "Администратор" ||
            role == "Кассир" ||
            role == "Клиент");
}
