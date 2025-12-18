#include "cashierwindow.h"
#include "ui_cashierwindow.h"

CashierWindow::CashierWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CashierWindow)
{
    ui->setupUi(this);
}

CashierWindow::~CashierWindow()
{
    delete ui;
}
