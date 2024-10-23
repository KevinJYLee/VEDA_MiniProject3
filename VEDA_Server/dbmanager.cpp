#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

QString DBManager::s_dataPath;

QString DBManager::getDataPath()
{
    if (s_dataPath.isEmpty()) {
        s_dataPath = QCoreApplication::applicationDirPath() + "/data";
        QDir dir(s_dataPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }
    return s_dataPath;
}

DBManager::DBManager(QObject *parent) : QObject(parent)
{
    m_dbPath = getDataPath() + "/users.db";

    QDir().mkpath(getDataPath());  // 디렉토리가 없으면 생성

    m_userDb = QSqlDatabase::addDatabase("QSQLITE", "UserConnection");
    m_userDb.setDatabaseName(m_dbPath);

    if (!m_userDb.open()) {
        qDebug() << "Error: Failed to connect to database";
        qDebug() << "Database error:" << m_userDb.lastError().text();
    } else {
        qDebug() << "Database: Connection successful";
        qDebug() << "Database path:" << m_dbPath;
    }

    if (!createUserTable()) {
        qDebug() << "Failed to create user table";
    }

    m_userModel = new QSqlTableModel(this, m_userDb);
    m_userModel->setTable("users");
    m_userModel->select();
}

DBManager::~DBManager()
{
    if (m_userDb.isOpen())
        m_userDb.close();
}

bool DBManager::createUserTable()
{
    if (!m_userDb.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }

    QSqlQuery query(m_userDb);
    query.prepare("CREATE TABLE IF NOT EXISTS users "
                  "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "name TEXT, "
                  "team TEXT, "
                  "position TEXT)");

    if (!query.exec()) {
        qDebug() << "Error creating user table:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::addUser(const QString &name, const QString &team, const QString &position)
{
    QSqlQuery query(m_userDb);
    query.prepare("INSERT INTO users (name, team, position) VALUES (:name, :team, :position)");
    query.bindValue(":name", name);
    query.bindValue(":team", team);
    query.bindValue(":position", position);

    if (query.exec()) {
        m_userModel->select();
        return true;
    } else {
        qDebug() << "Error adding user:" << query.lastError().text();
        return false;
    }
}

bool DBManager::updateUser(int id, const QString &name, const QString &team, const QString &position)
{
    QSqlQuery query(m_userDb);
    query.prepare("UPDATE users SET name = :name, team = :team, position = :position WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":team", team);
    query.bindValue(":position", position);

    if (query.exec()) {
        m_userModel->select();
        return true;
    } else {
        qDebug() << "Error updating user:" << query.lastError().text();
        return false;
    }
}

bool DBManager::deleteUser(int id)
{
    QSqlQuery query(m_userDb);
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        m_userModel->select();
        return true;
    } else {
        qDebug() << "Error deleting user:" << query.lastError().text();
        return false;
    }
}

QSqlTableModel* DBManager::getUserModel()
{
    return m_userModel;
}
