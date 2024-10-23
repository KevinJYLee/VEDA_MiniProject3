#include "chatlogger.h"
#include "dbmanager.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>

ChatLogger::ChatLogger(QObject *parent) : QObject(parent)
{
    QString dataPath = DBManager::getDataPath();
    m_chatLogPath = dataPath + "/chat_log.txt";
    m_connectionLogPath = dataPath + "/connection_log.txt";

    // 디렉토리가 없으면 생성
    QDir().mkpath(dataPath);

    m_chatThread = new QThread(this);
    m_connectionThread = new QThread(this);

    connect(m_chatThread, &QThread::started, this, &ChatLogger::processChatQueue);
    connect(m_connectionThread, &QThread::started, this, &ChatLogger::processConnectionQueue);
}

ChatLogger::~ChatLogger()
{
    stopLogging();
    delete m_chatThread;
    delete m_connectionThread;
}

void ChatLogger::startLogging()
{
    m_chatThread->start();
    m_connectionThread->start();
}

void ChatLogger::stopLogging()
{
    m_chatThread->quit();
    m_chatThread->wait();
    m_connectionThread->quit();
    m_connectionThread->wait();
}

void ChatLogger::logChat(const QString &message)
{
    QMutexLocker locker(&m_chatMutex);
    m_chatQueue.enqueue(message);
    QMetaObject::invokeMethod(this, "processChatQueue", Qt::QueuedConnection);
}

void ChatLogger::logConnection(const QString &message)
{
    QMutexLocker locker(&m_connectionMutex);
    m_connectionQueue.enqueue(message);
    QMetaObject::invokeMethod(this, "processConnectionQueue", Qt::QueuedConnection);
}

void ChatLogger::processChatQueue()
{
    QMutexLocker locker(&m_chatMutex);
    QFile file(m_chatLogPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        while (!m_chatQueue.isEmpty()) {
            out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << m_chatQueue.dequeue() << "\n";
        }
        file.close();
    }
}

void ChatLogger::processConnectionQueue()
{
    QMutexLocker locker(&m_connectionMutex);
    QFile file(m_connectionLogPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        while (!m_connectionQueue.isEmpty()) {
            out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << m_connectionQueue.dequeue() << "\n";
        }
        file.close();
    }
}
