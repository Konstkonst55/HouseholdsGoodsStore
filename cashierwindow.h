#ifndef CASHIERWINDOW_H
#define CASHIERWINDOW_H

#include <QWidget>

namespace Ui {
class CashierWindow;
}

class CashierWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CashierWindow(QWidget *parent = nullptr);
    ~CashierWindow();

private:
    Ui::CashierWindow *ui;
};

#endif // CASHIERWINDOW_H
