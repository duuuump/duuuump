#pragma once

#include "sqllite.h"
#include <assert.h>
#include <time.h>
#include "../util/uuid.hpp"

std::map<QString, QString> configs;

//����һ�����ݿ�����
bool SQLLite::createConnection()
{
    //�Ժ�Ϳ�����"sqlite1"�����ݿ����������
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "sqlite1");
    //char* sysHome = Util::getSysHome();
    QString path("c:/phone_connector/");
    path.append("db");
    db.setDatabaseName(path);
    if (!db.open())
    {
        qDebug() << QString::fromLocal8Bit("�޷��������ݿ�����");
        return false;
    }
    qDebug() << QString::fromLocal8Bit("�������ݿ�����");
    qdb = db;
    return true;
}

//�������ݿ��
bool SQLLite::createCallRecordTable()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    bool success = query.exec("CREATE TABLE IF NOT EXISTS qt_call_rcd ("
        "id integer,"
        "call_time text,"
        "phone text,"
        "path text,"
        "duration text,"
        "status text,"
        "token text," // �����ϴ�ʱ��֤
        "PRIMARY KEY(id)"
        ")");
    if (success)
    {
        qDebug() << QString::fromLocal8Bit("���ݿ�����ɹ�");
        return true;
    }
    else
    {
        qDebug() << QString::fromLocal8Bit("���ݿ����ʧ��");
        return false;
    }
}

bool SQLLite::createLogTable()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    bool success = query.exec("CREATE TABLE IF NOT EXISTS qt_log ("
        "id integer,"
        "log_time text,"
        "type text,"
        "info text,"
        "res text,"
        "PRIMARY KEY(id)"
        ")");
    if (success)
    {
        qDebug() << QString::fromLocal8Bit("���ݿ�����ɹ�");
        return true;
    }
    else
    {
        qDebug() << QString::fromLocal8Bit("���ݿ����ʧ��");
        return false;
    }
}

Table::CallRecord SQLLite::getRecord(int id) 
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.prepare("select id,call_time,phone,path,duration,status,token from qt_call_rcd where id=?");
    query.bindValue(0, id);
    bool success = query.exec();
    Table::CallRecord r;
    r.id = -1;
    if (!success)
    {
        QSqlError lastError = query.lastError();
        qDebug() << lastError.driverText() << "\n" << QString::fromLocal8Bit("����ʧ��");
    }
    if (query.next())
    {
        r.id = query.value(0).toInt();
        r.call_time = query.value(1).toString();
        r.phone = query.value(2).toString();
        r.path = query.value(3).toString();
        r.duration = query.value(4).toString();
        r.status = query.value(5).toString();
        r.token = query.value(6).toString();
    }
    return r;
}

//�����ݿ��в����¼
int SQLLite::insertCallRecord(Table::CallRecord r)
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.exec("select max(id) from qt_call_rcd;");
    int i = 0;
    if (query.next())
    {
        QVariant id = query.value(0);
        i = id.toInt();
    }
    i++;
    query.prepare("insert into qt_call_rcd(id,phone,call_time,path,duration,status,token) values(?, ?, ?, ?, ?, ?,?)");
    time_t t = time(0);
    char ch[64];
    strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //��-��-�� ʱ-��-��
    query.bindValue(0, i);
    query.bindValue(1, r.phone);
    query.bindValue(2, QString::fromLocal8Bit(ch));
    query.bindValue(3, r.path);
    query.bindValue(4, r.duration);
    query.bindValue(5, r.status);
    query.bindValue(6, r.token);
    bool success = query.exec();

    if (!success)
    {
        QSqlError lastError = query.lastError();
        qDebug() << lastError.driverText() << "\n" << QString::fromLocal8Bit("����ʧ��");
        return -1;
    }
    return i;
}
//����ID���¼�¼
bool SQLLite::updateCallRecord(Table::CallRecord r)
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.prepare(QString("update qt_call_rcd set duration=?,status=?,path=?,token=? "
        "where id=%1").arg(r.id));

    query.bindValue(0, r.duration);
    query.bindValue(1, r.status);
    query.bindValue(2, r.path);
    query.bindValue(3, r.token);

    bool success = query.exec();
    if (!success)
    {
        QSqlError lastError = query.lastError();
        qDebug() << lastError.driverText() << QString(QObject::tr("����ʧ��"));
    }
    return true;
}


//�����ݿ��в����¼
bool SQLLite::insertLog(Table::Log l)
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.prepare("insert into qt_log(log_time,type,info,res) values(?, ?, ?, ?)");
    time_t t = time(0);
    char ch[64];
    strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //��-��-�� ʱ-��-��
    query.bindValue(0, QString::fromLocal8Bit(ch));
    query.bindValue(1, l.type);
    query.bindValue(2, l.info);
    query.bindValue(3, l.res);
    bool success = query.exec();

    if (!success)
    {
        QSqlError lastError = query.lastError();
        qDebug() << lastError.driverText() << "\n" << QString::fromLocal8Bit("����ʧ��");
        return false;
    }
    return true;
}

bool SQLLite::initConfig()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    bool success = query.exec("CREATE TABLE IF NOT EXISTS qt_config ("
        "code text,"
        "value text,"
        "PRIMARY KEY(code)"
        ")");
    if (success)
    {
        qDebug() << QString::fromLocal8Bit("���ݿ�����ɹ�");
        query.exec("insert into qt_config select 'DIALING','1' where not exists (select 1 from qt_config where code='DIALING')");
        query.exec("insert into qt_config select 'AUTO_ANSWER','0' where not exists (select 1 from qt_config where code='AUTO_ANSWER')");
        query.exec("insert into qt_config select 'BELL','1' where not exists (select 1 from qt_config where code='BELL')");
        query.exec("insert into qt_config select 'OUT_CODE','0' where not exists (select 1 from qt_config where code='OUT_CODE')");
        query.exec("insert into qt_config select 'PORT','4399' where not exists (select 1 from qt_config where code='PORT')");
        query.exec("insert into qt_config select 'UPLOAD_ADDR','' where not exists (select 1 from qt_config where code='UPLOAD_ADDR')");
        qDebug() << QString::fromLocal8Bit("ϵͳ�������");
        return true;
    }
    else
    {
        qDebug() << QString::fromLocal8Bit("���ݿ����ʧ��");
        return false;
    }
}

bool SQLLite::updateConfig(QString code, QString value)
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.prepare(QString("update qt_config set value='%1' "
        "where code='%2'").arg(value).arg(code));
    bool success = query.exec();
    if (!success)
    {
        QSqlError lastError = query.lastError();
        qDebug() << lastError.driverText() << QString(QObject::tr("����ʧ��"));
    }
    return true;
}

std::map<QString, QString> SQLLite::listConfig()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.prepare("select * from qt_config");
    bool success = query.exec();
    if (success)
    {
        QString code;
        QString value;
        while (query.next())
        {
            code = query.value(0).toString();
            value = query.value(1).toString();
            configs[code] = value;
        }
    }
    return configs;
}
