#ifndef CARTOBSERVER_H
#define CARTOBSERVER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QDebug>

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update(const QString &message) = 0;
    virtual QString getName() const = 0;
};

class ISubject {
public:
    virtual ~ISubject() = default;
    virtual void attach(IObserver *observer) = 0;
    virtual void detach(IObserver *observer) = 0;
    virtual void notify(const QString &message) = 0;
    virtual int getObserverCount() const = 0;
};

class CartSubject : public QObject, public ISubject {
    Q_OBJECT

private:
    QList<IObserver*> observers;
    QString subjectName;

public:
    explicit CartSubject(const QString &name = "CartSubject", QObject *parent = nullptr);
    ~CartSubject();

    void attach(IObserver *observer) override;
    void detach(IObserver *observer) override;
    void notify(const QString &message) override;
    int getObserverCount() const override;

    QString getSubjectName() const { return subjectName; }
    void setSubjectName(const QString &name) { subjectName = name; }

signals:
    void cartChanged(const QString &message);
    void observerAdded(const QString &observerName);
    void observerRemoved(const QString &observerName);
};

class LoggerObserver : public IObserver {
private:
    QString observerName;
    bool enabled;

public:
    explicit LoggerObserver(const QString &name = "Logger", bool enabled = true);
    ~LoggerObserver();

    void update(const QString &message) override;
    QString getName() const override { return observerName; }

    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }
};

class UINotificationObserver : public IObserver {
private:
    QString observerName;

public:
    explicit UINotificationObserver(const QString &name = "UINotifications");
    ~UINotificationObserver();

    void update(const QString &message) override;
    QString getName() const override { return observerName; }
};

#endif // CARTOBSERVER_H
