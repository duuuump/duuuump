#ifndef DATABASE_H
#define DATABASE_H

#include <QTextCodec>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTime>
#include <QSqlError>
#include <QtDebug>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlTableModel>

namespace Table {
    struct CallRecord
    {
        int id;
        QString call_time;
        QString phone;
        QString path;
        QString duration;
        QString status;
        QString token;
    };
    struct Log {
        int id;
        QString log_time;
        QString type;
        QString info;
        QString res;
        Log(QString type, QString info, QString res) {
            this->type = type;
            this->info = info;
            this->res = res;
        }
    };
}

class SQLLite
{
public:
    QSqlDatabase qdb;
    bool createConnection();  //创建一个连接
    bool createCallRecordTable();       //创建数据库表
    bool createLogTable();       //创建数据库表
    Table::CallRecord getRecord(int id);
    int insertCallRecord(Table::CallRecord r);
    bool updateCallRecord(Table::CallRecord r);
    bool insertLog(Table::Log l);
    bool initConfig();
    bool updateConfig(QString code, QString value);
    std::map<QString, QString> listConfig();
};
#endif // DATABASE_H