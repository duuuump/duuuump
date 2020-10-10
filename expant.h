#pragma once

#include <QtWidgets/QWidget>
#include "ui_expant.h"
#include <QSqlTableModel>
#include <QAbstractNativeEventFilter>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>

typedef std::map<int, QString> PHONE_MESSAGE_MAP;

class EXPANT : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    EXPANT(QWidget *parent = Q_NULLPTR);
    void paintEvent(QPaintEvent* event);
    QFont icon;
    void end();
    void call(QString phone, QString token);
    void hang();
    void socketMsg(QString msg, bool init);
    void socketMsg(QString msg);
    void cmdMsg(QString msg);
    int getSocketPort();
    void auth(QString url, QString username, QString code);
    Ui::EXPANTClass ui;
protected:
    // 重写鼠标事件
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    void init();
    void scanRec();
    void configChange(int state);
    void initConfig();
    void editConfig();
    void hook();
    void call();
    void search();
    void chooseLog(int index);

    QSqlTableModel* model;
    QSqlTableModel* logModel;
    PHONE_MESSAGE_MAP msg_map;
    HWND hwnd;
    int currentRecordID;
    virtual bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

    QPoint last;
private slots:
    void OnUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void upload();
};
