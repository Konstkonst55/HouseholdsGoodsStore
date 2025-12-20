#include "cartobserver.h"
#include <QMessageBox>

CartSubject::CartSubject(const QString &name, QObject *parent)
    : QObject(parent), subjectName(name) {
    qDebug() << "[CartSubject] Created:" << name;
}

CartSubject::~CartSubject() {
    qDebug() << "[CartSubject] Destroyed:" << subjectName;
    for (IObserver* observer : observers) {
        detach(observer);
    }
    observers.clear();
}

void CartSubject::attach(IObserver *observer) {
    if (observer && !observers.contains(observer)) {
        observers.append(observer);
        qDebug() << "[CartSubject] Observer attached:" << observer->getName();
        emit observerAdded(observer->getName());
    }
}

void CartSubject::detach(IObserver *observer) {
    if (observer && observers.removeOne(observer)) {
        qDebug() << "[CartSubject] Observer detached:" << observer->getName();
        emit observerRemoved(observer->getName());
    }
}

void CartSubject::notify(const QString &message) {
    if (observers.isEmpty()) {
        qDebug() << "[CartSubject] No observers to notify";
        return;
    }

    QString fullMessage = QString("[%1] %2").arg(subjectName).arg(message);
    qDebug() << "[CartSubject] Notifying" << observers.size() << "observers:" << message;

    QList<IObserver*> observersCopy = observers;

    for (IObserver *observer : observersCopy) {
        if (observer) {
            observer->update(fullMessage);
        }
    }

    emit cartChanged(message);
}

int CartSubject::getObserverCount() const {
    return observers.size();
}

LoggerObserver::LoggerObserver(const QString &name, bool enabled)
    : observerName(name), enabled(enabled) {
    qDebug() << "[LoggerObserver] Created:" << name;
}

LoggerObserver::~LoggerObserver() {
    qDebug() << "[LoggerObserver] Destroyed:" << observerName;
}

void LoggerObserver::update(const QString &message) {
    if (enabled) {
        qDebug() << QString("[%1] %2").arg(observerName).arg(message);
    }
}

UINotificationObserver::UINotificationObserver(const QString &name)
    : observerName(name) {
    qDebug() << "[UINotificationObserver] Created:" << name;
}

UINotificationObserver::~UINotificationObserver() {
    qDebug() << "[UINotificationObserver] Destroyed:" << observerName;
}

void UINotificationObserver::update(const QString &message) {
    qDebug() << QString("[%1] UI Notification: %2").arg(observerName).arg(message);
}
