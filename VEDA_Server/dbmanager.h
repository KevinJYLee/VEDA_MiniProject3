#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    bool createUserTable();
    bool addUser(const QString &name, const QString &team, const QString &position);
    bool updateUser(int id, const QString &name, const QString &team, const QString &position);
    bool deleteUser(int id);
    QSqlTableModel* getUserModel();

    static QString getDataPath();

private:
    QSqlDatabase m_userDb;
    QSqlTableModel *m_userModel;
    QString m_dbPath;

    static QString s_dataPath;
};

#endif // DBMANAGER_H
