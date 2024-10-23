#ifndef CHATLOGGER_H
#define CHATLOGGER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QThread>

class ChatLogger : public QObject
{
    Q_OBJECT
public:
    explicit ChatLogger(QObject *parent = nullptr);
    ~ChatLogger();

    void startLogging();
    void stopLogging();

public slots:
    void logChat(const QString &message);
    void logConnection(const QString &message);

private slots:
    void processChatQueue();
    void processConnectionQueue();

private:
    QString m_chatLogPath;
    QString m_connectionLogPath;
    QQueue<QString> m_chatQueue;
    QQueue<QString> m_connectionQueue;
    QMutex m_chatMutex;
    QMutex m_connectionMutex;
    QThread *m_chatThread;
    QThread *m_connectionThread;
};

#endif // CHATLOGGER_H
