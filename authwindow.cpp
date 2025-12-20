#include "authwindow.h"
#include "ui_authwindow.h"
#include "windowfactory.h"
#include "database.h"
#include <QMessageBox>

AuthWindow::AuthWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AuthWindow)
{
    ui->setupUi(this);

    connect(ui->leLogin, &QLineEdit::returnPressed, this, &AuthWindow::on_pbLogin_clicked);
    connect(ui->lePassword, &QLineEdit::returnPressed, this, &AuthWindow::on_pbLogin_clicked);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_pbLogin_clicked()
{
    QString login = ui->leLogin->text();
    QString password = ui->lePassword->text();

    ui->lIncorrectLogin->setVisible(false);

    if (login.isEmpty() || password.isEmpty())
    {
        ui->lIncorrectLogin->setText("Заполните все поля");
        ui->lIncorrectLogin->setVisible(true);
        return;
    }

    Database *db = new Database(this);

    if (!db->initializeDatabase())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
        exit(1);
    }

    if (!db->connectToDatabase())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных");
        exit(1);
    }

    User user = db->authenticateUser(login, password);

    if (user.id == -1)
    {
        ui->lIncorrectLogin->setText("Неверное имя пользователя или пароль");
        ui->lIncorrectLogin->setVisible(true);
        ui->lePassword->clear();
        ui->lePassword->setFocus();
    }
    else
    {
        ui->lIncorrectLogin->setVisible(false);

        BaseWindow* newWindow = WindowFactory::createWindow(user.role, user);

        if (newWindow)
        {
            newWindow->showWindow();
            this->close();
        }
        else
        {
            ui->lIncorrectLogin->setText("Недостаточно прав или неизвестная роль");
            ui->lIncorrectLogin->setVisible(true);
        }
    }
}
