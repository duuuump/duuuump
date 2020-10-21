#include <io.h>
#include <direct.h>
#include "expant.h"
#include "stdafx.h"
#include "dll/UsbDll.h"
#include "util/uuid.hpp"
#include "SQL/sqllite.h"
#include <time.h>
#include <dbt.h>
#include "taskhandler.hpp"
#include <QHttpPart>
#include "util.hpp"
#include "hardware/wmic.h"
#include <document.h>

time_t start_time, end_time;
QString dtmt = "";
int dtmt_index = 0;
bool editFlag = false;
QSystemTrayIcon *trayIcon;

QString comming = "";

WMIC wmic;
QString hardwareEncryption("{\"board\":\"%1\",\"cpu\":\"%2\",\"bios\":\"%3\",\"macs\":[]}");

const char* AUDIO_DIR = "c:/phone_connector/audio";
const char* DIR_PREFIX = "c:/phone_connector/";
int last_phone_msg = 0;
int cmd_busy_flg = 0;
int hook_flg = 0;

void EXPANT::mousePressEvent(QMouseEvent* e)
{
    last = e->globalPos();
}
void EXPANT::mouseMoveEvent(QMouseEvent* e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    last = e->globalPos();
    move(x() + dx, y() + dy);
}
void EXPANT::mouseReleaseEvent(QMouseEvent* e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    move(x() + dx, y() + dy);
}


EXPANT::EXPANT(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->init();
}

void EXPANT::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QColor m_defaultBackgroundColor = qRgb(65, 65, 65);
    QColor m_defaultBorderColor = qRgb(100, 100, 100);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(10, 10, this->width() - 20, this->height() - 20, 4, 4);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
        m_defaultBackgroundColor.green(),
        m_defaultBackgroundColor.blue())));

    QColor color(80, 80, 80, 100);
    for (int i = 0; i < 10; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(10 - i, 10 - i, this->width() - (10 - i) * 2, this->height() - (10 - i) * 2, 4, 4);
        if (200 - 25.5 * (i + 5) > 0)
        {
            color.setAlpha(200 - 25.5 * (i + 5));
        }
        else
        {
            color.setAlpha(0);
        }
        painter.setPen(color);
        painter.drawPath(path);
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setX(10);
    rect.setY(10);
    rect.setWidth(rect.width() - 10);
    rect.setHeight(rect.height() - 10);
    // rect: ��������  15��Բ�ǻ���
    painter.drawRoundedRect(rect, 4, 4);
}

void EXPANT::init() {
    if (_access(DIR_PREFIX, 0) == -1)
    {
        _mkdir(DIR_PREFIX);
    }
    if (_access(AUDIO_DIR, 0) == -1)
    {
        _mkdir(AUDIO_DIR);
    }
    this->msg_map[701] = QString::fromLocal8Bit("USB�豸����");
    this->msg_map[702] = QString::fromLocal8Bit("USB����PC���ն˵绰�Ͽ�");
    this->msg_map[703] = QString::fromLocal8Bit("���н���");
    this->msg_map[704] = QString::fromLocal8Bit("���жϿ�");
    this->msg_map[705] = QString::fromLocal8Bit("����");
    this->msg_map[706] = QString::fromLocal8Bit("��������");
    this->msg_map[707] = QString::fromLocal8Bit("���ţ�");
    this->msg_map[708] = QString::fromLocal8Bit("���Թ����У����������绰");
    this->msg_map[709] = QString::fromLocal8Bit("ͨ�������У��յ�����������������");
    ui.label->setHidden(true);
    ui.lineEdit->setHidden(true);
    ui.pushButton->setHidden(true);
    ui.pushButton_2->setHidden(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowIcon(QIcon(":/resources/main.ico"));
    trayIcon = new QSystemTrayIcon(this);
    connect(ui.close_btn, &QPushButton::clicked, this, [=]() 
        {
            this->hide();
            QIcon icon = QIcon(":/resources/main.ico");
            trayIcon->setIcon(icon);
            trayIcon->show();
        });
    connect(trayIcon, &QSystemTrayIcon::activated, [=]() 
        {
            this->show();
        });
    connect(ui.shutdown_btn, &QPushButton::clicked, [=]() 
        {
            QMessageBox message(QMessageBox::Warning, QString::fromLocal8Bit("����"),
                QString::fromLocal8Bit("ȷ���˳�����\t\t"),
                QMessageBox::Yes | QMessageBox::No, NULL);
            message.setWindowIcon(QIcon(":/resources/image/msg.png"));
            message.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("ȷ��"));
            message.setButtonText(QMessageBox::No, QString::fromLocal8Bit("ȡ��"));
            message.setWindowFlags(message.windowFlags() | Qt::WindowStaysOnTopHint);
            if (message.exec() == QMessageBox::Yes)
            {
                close();
            }
        });
    connect(ui.top_btn, &QPushButton::clicked, this, [=]()
        {
            HWND hwnd = (HWND)this->winId();
            DWORD dwstyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
            DWORD err = GetLastError();
            if (dwstyle & WS_EX_TOPMOST)
            {
                dwstyle &= ~WS_EX_TOPMOST;
                ui.top_btn->setIcon(QIcon(":/resources/image/top.png"));
                ::SetWindowLong(hwnd, GWL_EXSTYLE, dwstyle);
                ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
            }
            else
            {
                dwstyle |= WS_EX_TOPMOST;
                ui.top_btn->setIcon(QIcon(":/resources/image/top_s.png"));
                ::SetWindowLong(hwnd, GWL_EXSTYLE, dwstyle);
                ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSIZE | SWP_SHOWWINDOW);
            }
        });
    connect(ui.pushButton, &QPushButton::clicked, this, static_cast<void (EXPANT::*)()>(&EXPANT::call));
    connect(ui.pushButton_2, &QPushButton::clicked, this, &EXPANT::end);
    connect(ui.pushButton_5, &QPushButton::clicked, this, &EXPANT::hook);
    connect(ui.pushButton_3, &QPushButton::clicked, this, &EXPANT::search);
    connect(ui.comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EXPANT::chooseLog);
    connect(ui.pushButton_4, &QPushButton::clicked, this, &EXPANT::editConfig);
    connect(ui.checkBox, &QCheckBox::stateChanged, this, &EXPANT::configChange);
    connect(ui.checkBox_2, &QCheckBox::stateChanged, this, &EXPANT::configChange);
    connect(ui.checkBox_3, &QCheckBox::stateChanged, this, &EXPANT::configChange);
    connect(ui.pushButton_6, &QPushButton::clicked, this, &EXPANT::scanRec);

    ui.groupBox->setEnabled(false);
    ui.groupBox_2->setEnabled(false);
    ui.lineEdit_3->setValidator(new QIntValidator(100, 50000, this));
    ui.lineEdit_2->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    int fontId = QFontDatabase::addApplicationFont(":/resources/fonts/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont iconFont = QFont(fontName);
    iconFont.setPointSize(9);
    this->icon = iconFont;
    ui.statusIcon1->setFont(iconFont);
    ui.statusIcon1->setText(QChar(0xf111));
    ui.label_5->setFont(iconFont);
    ui.label_5->setText(QChar(0xf111));
    ui.progressBar->setHidden(true);
    SQLLite sqllite;
    sqllite.createConnection();
    sqllite.createCallRecordTable();
    sqllite.createLogTable();
    sqllite.initConfig();
    initConfig();
    time_t t = time(0);
    // t -= 60 * 60 * 24;
    char ch[64];
    this->hwnd = (HWND)this->winId();
    InitDll();
    BindWindow(this->hwnd);
    strftime(ch, sizeof(ch), "%Y-%m-%d 00:00:00", localtime(&t));
    ui.dateEdit->setDate(QDate::fromString(ch, Qt::DateFormat::ISODate));
    model = new QSqlTableModel(this, sqllite.qdb);
    model->setTable("qt_call_rcd");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setSort(0, Qt::DescendingOrder);
    model->setFilter(QString("call_time>'%1'").arg(ch));
    model->select();
    //model->setHeaderData(0, Qt::Horizontal, tr("id"));
    model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("����ʱ��"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("·��"));
    model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("����"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("ʱ��"));
    
    ui.tableView->setModel(model);
    ui.tableView->horizontalHeader()->setResizeContentsPrecision(1);
    ui.tableView->horizontalHeader()->setResizeContentsPrecision(2);
    // ui.tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    ui.tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui.tableView->verticalHeader()->hide();
    ui.tableView->horizontalHeader()->hide();
    ui.tableView->setColumnHidden(0, true);
    ui.tableView->setColumnHidden(6, true);
    // ui.tableView->setColumnHidden(3, true);
    ui.tableView->resizeColumnToContents(4);
    ui.tableView->resizeColumnToContents(0);
    ui.tableView->setAlternatingRowColors(true);
    ui.tableView->setAutoScroll(true);
    ui.tableView->setFrameShape(QFrame::NoFrame);
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableView->setFocusPolicy(Qt::NoFocus);
    ui.tableView->setColumnWidth(4, 60);
    ui.tableView->setColumnWidth(1, 130);
    ui.tableView->setColumnWidth(5, 80);

    t -= 60 * 60 * 24;
    strftime(ch, sizeof(ch), "%Y-%m-%d 00:00:00", localtime(&t));
    logModel = new QSqlTableModel(this, sqllite.qdb);
    logModel->setTable("qt_log");
    logModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    logModel->setSort(0, Qt::DescendingOrder);
    logModel->setFilter(QString("log_time>'%1'").arg(ch));
    logModel->select();
    ui.tableView2->setModel(logModel);
    logModel->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("ʱ��"));
    logModel->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("����"));
    logModel->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("��Ϣ"));
    logModel->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("���"));
    ui.tableView2->verticalHeader()->hide();
    ui.tableView2->setColumnHidden(0, true);
    ui.tableView2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableView2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableView2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableView2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui.tableView2->setAutoScroll(true);
    ui.tableView2->setFrameShape(QFrame::NoFrame);
    ui.tableView2->setFocusPolicy(Qt::NoFocus);
    ui.tableView2->setColumnWidth(1, 130);
    sqllite.insertLog(Table::Log("PROGRAM", QString::fromLocal8Bit("��ʼ��"), "0"));
    t -= 60 * 60 * 24 * 2;
    strftime(ch, sizeof(ch), "%Y-%m-%d 00:00:00", localtime(&t));
    QSqlQuery query(QSqlDatabase::database("sqlite1"));
    query.exec(QString("delete from qt_log where log_time < '%1'").arg(ch));

    // ��ʼ��token����ֹ����
    ui.tokenInput->setText("-");
}

void EXPANT::scanRec()
{
    QSqlDatabase db = QSqlDatabase::database("sqlite1"); //�������ݿ�����
    QSqlQuery query(db);
    query.exec(QString::fromLocal8Bit("SELECT count(CASE when status='�ȴ��ϴ�' then 1 else NULL end),"
        "count(CASE when status like 'ʧ��%' then 1 else NULL end),"
        "count(CASE when status like '�쳣%' then 1 else NULL end),"
        "count(CASE when status='�ϴ���ʱ' then 1 else NULL end)"
        " FROM qt_call_rcd where token!='-' and token is not null"));
    QString wait;
    QString fail;
    QString err;
    QString timeout;
    while (query.next())
    {
        wait = query.value(0).toString();
        fail = query.value(1).toString();
        err = query.value(2).toString();
        timeout = query.value(3).toString();
    }
    
    
    if (wait.toInt() + err.toInt() + timeout.toInt() == 0)
    {
        QMessageBox message(QMessageBox::Question, QString::fromLocal8Bit("¼��ɨ����"),
            QString::fromLocal8Bit("����¼���Ѿ��ϴ�������"),
            QMessageBox::Yes, NULL);
        message.setWindowFlags(message.windowFlags() | Qt::WindowStaysOnTopHint);
        message.setWindowIcon(QIcon(":/resources/image/msg.png"));
        message.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("ȷ��"));
        message.exec();
        return;
    }
    QMessageBox message(QMessageBox::Question, QString::fromLocal8Bit("¼��ɨ����"),
        QString::fromLocal8Bit("�ȴ��ϴ���%1��\t\t\t\r\n�ϴ��쳣��%2\r\n�ϴ���ʱ��%3").arg(wait).arg(err).arg(timeout),
        QMessageBox::Yes | QMessageBox::No, NULL);
    message.setWindowFlags(message.windowFlags() | Qt::WindowStaysOnTopHint);
    message.setWindowIcon(QIcon(":/resources/image/msg.png"));
    message.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("�����ϴ�"));
    message.setButtonText(QMessageBox::No, QString::fromLocal8Bit("ȡ��"));
    if (message.exec() == QMessageBox::Yes)
    {
        query.exec(QString::fromLocal8Bit("SELECT id,call_time,phone,path,duration,token "
            "FROM qt_call_rcd where status != '�ϴ��ɹ�' and token!='-' and token is not null"));
        while (query.next())
        {
            Table::CallRecord rec;
            rec.id = query.value(0).toInt();
            shared_ptr<Task> t = make_shared<Task>(rec.id, TaskType::UPLOAD);
            connect(t.get(), &Task::upload, this, &EXPANT::upload, Qt::BlockingQueuedConnection);
            TaskQueue::Instance()->push(t);
        }
    }
}

int EXPANT::getSocketPort()
{
    QString str = ui.lineEdit_3->text();
    if (!str.isEmpty())
    {
        return str.toInt();
    }
    return 4399;
}

void EXPANT::configChange(int state)
{
    QCheckBox *that = qobject_cast<QCheckBox*>(sender());
    that->setText(QString::fromLocal8Bit(state ? "��" : "��"));
}

// �ϴ�¼��
void EXPANT::upload()
{

    Task *task = static_cast<Task*>(sender());
    SQLLite sqllite;
    Table::CallRecord r = sqllite.getRecord(task->id);
    if (r.id == -1 || r.token == "-") // û��token��¼���ϴ�
    {
        return;
    }
    QString absolutePath(AUDIO_DIR);
    absolutePath.append("/").append(r.path);
    if (r.path.endsWith(".wav"))
    {
        ui.progressBar->setValue(0);
        ui.progressBar->setHidden(false);
        ui.progressBar->setFormat(QString::fromLocal8Bit("��ʽת��%p%"));
        Table::Log log("PROGRAM", QString::fromLocal8Bit("��Ƶ��ʽת��"), "");

        int con = Util::wav2mp3(absolutePath.toStdString().c_str());
        log.res = std::to_string(con).c_str();
        sqllite.insertLog(log);
        this->logModel->select();
        if (con != 0)
        {
            ui.progressBar->setFormat(QString::fromLocal8Bit("�����ϴ�%p%"));
            ui.progressBar->setHidden(true);
            QMessageBox message(QMessageBox::Question, QString::fromLocal8Bit("��ʽת��"),
                QString::fromLocal8Bit("��Ƶ�ļ�ת���쳣!!!\r\n�������(%1)").arg(std::to_string(con).c_str()),
                QMessageBox::No, NULL);
            message.setWindowFlags(message.windowFlags() | Qt::WindowStaysOnTopHint);
            message.setButtonText(QMessageBox::No, QString::fromLocal8Bit("ȷ��"));
            message.exec();
            return;
        }
        else
        {
            r.path.replace(r.path.lastIndexOf("."), 4, ".mp3");
            absolutePath.replace(absolutePath.lastIndexOf("."), 4, ".mp3");
            sqllite.updateCallRecord(r);
            this->model->select();
        }
    }
    QEventLoop loop2;
    QFile* file;
    QNetworkAccessManager* _uploadManager;
    QNetworkReply* _reply;
    ui.progressBar->setValue(0);
    ui.progressBar->setFormat(QString::fromLocal8Bit("�����ϴ�%p%"));
    ui.progressBar->setHidden(false);
    
    file = new QFile(absolutePath);
    if (!file->open(QIODevice::ReadOnly))
    {
        ui.progressBar->setHidden(true);
        file->deleteLater();
        return;
    }
    _uploadManager = new QNetworkAccessManager(this);
    _uploadManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QHttpPart filePart;
    filePart.setBodyDevice(file);
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/x-wav"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"file\";filename=\"%1\"; ").arg(file->fileName())));
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(filePart);
    multiPart->setParent(_uploadManager);
    std::map<QString, QString> configs = sqllite.listConfig();
    
    QUrl url(configs["UPLOAD_ADDR"]);
    qDebug() << url.toString();
    //QUrl::fromPercentEncoding(configs["UPLOAD_ADDR"].toLatin1());
    QNetworkRequest request(QUrl::fromPercentEncoding(configs["UPLOAD_ADDR"].toLatin1()));
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);
    request.setRawHeader("UPLOAD-TOKEN", r.token.toLatin1());
    QTimer timer;
    timer.setInterval(15000); // ���ó�ʱʱ�� 15 ��
    timer.setSingleShot(true); // ���δ���
    _reply = _uploadManager->post(request, multiPart);
    connect(&timer, &QTimer::timeout, &loop2, &QEventLoop::quit);
    connect(_reply, &QNetworkReply::uploadProgress, this, &EXPANT::OnUploadProgress);
    connect(_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), &loop2, &QEventLoop::quit);
    connect(_uploadManager, &QNetworkAccessManager::finished, &loop2, &QEventLoop::quit);
    timer.start();
    loop2.exec();
    Table::Log log("PROGRAM", QString::fromLocal8Bit("¼���ϴ�"), "");
    if (timer.isActive())
    {
        timer.stop();
        if (_reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error String : " << _reply->errorString();
            QVariant variant = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            r.status = QString::fromLocal8Bit("�쳣��%1��").arg(std::to_string(nStatusCode).c_str());
            log.res = std::to_string(nStatusCode).c_str();
        }
        else 
        {
            QVariant variant = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            log.res = std::to_string(nStatusCode).c_str();
            qDebug() << "Status Code : " << nStatusCode;
            if (nStatusCode == 200) // ��Ӧ200��Ϊ�ɹ�
            {
                r.status = QString::fromLocal8Bit("�ϴ��ɹ�");
            }
            else
            {
                r.status = QString::fromLocal8Bit("ʧ�ܡ�%1��").arg(std::to_string(nStatusCode).c_str());
            }
        }
    }
    else 
    {
        _reply->abort();
        _reply->deleteLater();
        qDebug() << "Timeout";
        r.status = QString::fromLocal8Bit("�ϴ���ʱ");
        log.res = std::to_string(-1).c_str();
    }
    sqllite.insertLog(log);
    sqllite.updateCallRecord(r);
    ui.progressBar->setHidden(true);
    this->model->select();
    Sleep(500);
    disconnect(&timer, &QTimer::timeout, &loop2, &QEventLoop::quit);
    disconnect(_reply, &QNetworkReply::uploadProgress, this, &EXPANT::OnUploadProgress);
    disconnect(_uploadManager, &QNetworkAccessManager::finished, &loop2, &QEventLoop::quit);
    multiPart->deleteLater();
    _uploadManager->deleteLater();
}

void EXPANT::initConfig()
{
    SQLLite sqllite;
    std::map<QString, QString> configs = sqllite.listConfig();
    ui.checkBox->setChecked("1" == configs["DIALING"]);
    ui.checkBox_2->setChecked("1" == configs["AUTO_ANSWER"]);
    ui.checkBox_3->setChecked("1" == configs["BELL"]);
    ui.lineEdit_2->setText(configs["OUT_CODE"]);
    ui.lineEdit_3->setText(configs["PORT"]);
    ui.lineEdit_4->setText(configs["UPLOAD_ADDR"]);
    SetDialTone(0, ui.checkBox->isChecked() ? 1 : 0);
    SetAutoAnswer(0, ui.checkBox_2->isChecked() ? 1 : 0);
    Bell(0, ui.checkBox_3->isChecked() ? 1 : 0);
    SetOutcode(0, ui.lineEdit_2->text().toStdString().c_str());
}

void EXPANT::editConfig() 
{
    if (!editFlag)
    {
        editFlag = true;
        QIcon iconSave;
        iconSave.addFile(QStringLiteral(":/resources/image/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui.pushButton_4->setIcon(iconSave);
        ui.pushButton_4->setText(QString::fromLocal8Bit("����"));
        ui.groupBox->setEnabled(true);
        ui.groupBox_2->setEnabled(true);
    }
    else
    {
        editFlag = false;
        QIcon iconEdit;
        iconEdit.addFile(QStringLiteral(":/resources/image/edit.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui.pushButton_4->setIcon(iconEdit);
        ui.pushButton_4->setText(QString::fromLocal8Bit("�༭"));
        ui.groupBox->setEnabled(false);
        ui.groupBox_2->setEnabled(false);

        QString dialing = ui.checkBox->isChecked() ? "1" : "0";
        QString autoAnswer = ui.checkBox_2->isChecked() ? "1" : "0";
        QString bell = ui.checkBox_3->isChecked() ? "1" : "0";
        QString outCode = ui.lineEdit_2->text();
        QString port = ui.lineEdit_3->text();
        QString uploadAddr = ui.lineEdit_4->text();
        SQLLite sqllite;
        sqllite.updateConfig("DIALING", dialing);
        sqllite.updateConfig("AUTO_ANSWER", autoAnswer);
        sqllite.updateConfig("BELL", bell);
        sqllite.updateConfig("OUT_CODE", outCode);
        sqllite.updateConfig("PORT", port);
        sqllite.updateConfig("UPLOAD_ADDR", uploadAddr);
        initConfig();
    }
}

// ժ����������
void EXPANT::hook() 
{
    if (cmd_busy_flg)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hook\", \"res\":\"�ظ�ָ��\"}").toStdString());
        return;
    }
    if (hook_flg)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hook\", \"res\":\"�ظ�ժ��\"}").toStdString());
        return;
    }
    hook_flg = 1;
    cmd_busy_flg = 1;
    int status = QueryPhoneStatus(0);
    if (status)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hook\", \"res\":\"������æ\"}").toStdString());
        cmd_busy_flg = 0;
        return;
    }
    ui.pushButton_2->setProperty("can", true);
    ui.lineEdit->setDisabled(true);
    ui.pushButton->setDisabled(true);
    ui.pushButton_2->setDisabled(false);
    
    SQLLite sqllite;
    QString phone(">");
    phone.append(comming);
    std::string uuid = utility::uuid::generate();
    std::string filename = AUDIO_DIR;
    filename.append("/").append(uuid).append(".wav");
    
    int openRec = setLocalRecord(0, TRUE);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("����¼���˿�"), std::to_string(openRec).c_str()));
    int startRec = StartRecordFile(0, filename.c_str(), 1);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("��ʼ¼��"), std::to_string(startRec).c_str()));
    Sleep(200);
    int hook = OffHookCtrl(0);
    if (hook == 0)
    {
        QString path(QString::fromStdString(uuid));
        path.append(".wav");
        QString token = ui.tokenInput->text();
        if (token != "-")
        {
            ui.tokenInput->setText("-");
        }
        Table::CallRecord r;
        r.phone = phone;
        r.path = path;
        r.duration = QString::fromLocal8Bit("ͨ����");
        r.status = "-";
        r.token = token;
        int id = sqllite.insertCallRecord(r);
        this->currentRecordID = id;
        start_time = time(NULL);
        char ch[64];
        strftime(ch, sizeof(ch), "%Y-%m-%d", localtime(&start_time));
        model->setFilter(QString("call_time>'%1'").arg(ch));
        ui.dateEdit->setDate(QDate::fromString(ch, Qt::DateFormat::ISODate));
        model->select();
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hook\", \"res\":\"ժ���ɹ�\", \"cid\":%1}").arg(std::to_string(id).c_str()).toStdString());
    }
    else
    {
        int save = StopRecordFile(0);
        int close = setLocalRecord(0, FALSE);
        if (!save)
        {
            int rm = remove(filename.c_str());
            sqllite.insertLog(Table::Log("PEOGRAM", QString::fromLocal8Bit("ɾ��ʧ��¼���ļ�"), std::to_string(rm).c_str()));
        }
        ui.lineEdit->setDisabled(false);
        ui.pushButton->setDisabled(false);
        ui.pushButton_2->setDisabled(true);
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hook\", \"res\":\"ժ��ʧ��\"}").toStdString());
    }
    sqllite.insertLog(Table::Log("PROGRAM", QString::fromLocal8Bit("����"), std::to_string(hook).c_str()));
    logModel->select();
    cmd_busy_flg = 0;
}

// ����
void EXPANT::call()
{
    if (cmd_busy_flg)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"call\", \"res\":\"�ظ�ָ��\"}").toStdString());
        return;
    }
    cmd_busy_flg = 1;
    int status = QueryPhoneStatus(0);
    if (status)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"call\", \"res\":\"������æ\"}").toStdString());
        cmd_busy_flg = 0;
        return; 
    }
    ui.pushButton_2->setProperty("can", true);
    QString token = ui.tokenInput->text();
    if (token != "-")
    {
        ui.tokenInput->setText("-");
    }
    ui.lineEdit->setDisabled(true);
    ui.pushButton->setDisabled(true);
    ui.pushButton_2->setDisabled(false);
    SQLLite sqllite;
    QString phone = ui.lineEdit->text();
    bool not_local = phone.startsWith("0");
    
    std::string uuid = utility::uuid::generate();
    QString filename(AUDIO_DIR);
    filename.append("/").append(uuid.c_str()).append(".wav");
    int openRec = setLocalRecord(0, TRUE);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("����¼���˿�"), std::to_string(openRec).c_str()));
    QByteArray qb = filename.toLatin1();
    const char* fm = qb.constData();
    int startRec = StartRecordFile(0, fm, 1);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("��ʼ¼��"), std::to_string(startRec).c_str()));
    Sleep(200);
    QByteArray data = phone.toLatin1();
    const char* p = data.constData();
    int out = -1;
    OffHookCtrl(0);
    Sleep(1000);
    out = SendDTMF(0, p);

    QString path(QString::fromStdString(uuid));
    path.append(".wav");
    cmd_busy_flg = 0;
    if (out == 0)
    {
        Table::CallRecord r;
        r.phone = phone;
        r.path = path;
        r.duration = QString::fromLocal8Bit("ͨ����");
        r.status = "-";
        r.token = token;
        int id = sqllite.insertCallRecord(r);
        this->currentRecordID = id;
        start_time = time(0);
        char ch[64];
        strftime(ch, sizeof(ch), "%Y-%m-%d", localtime(&start_time));
        model->setFilter(QString("call_time>'%1'").arg(ch));
        ui.dateEdit->setDate(QDate::fromString(ch, Qt::DateFormat::ISODate));
        model->select();
        QString msg = QString::fromLocal8Bit("{\"event\":\"call\", \"res\":\"���ųɹ�\", \"cid\": %1}");
        ServerSC::Instance()->Replay(msg.arg(std::to_string(id).c_str()).toStdString());
    }
    else
    {
        int save = StopRecordFile(0);
        int close = setLocalRecord(0, FALSE);
        if (!save)
        {
            int rm =  remove(fm);
            sqllite.insertLog(Table::Log("PEOGRAM", QString::fromLocal8Bit("ɾ��ʧ��¼���ļ�"), std::to_string(rm).c_str()));
        }
        ui.lineEdit->setDisabled(false);
        ui.pushButton->setDisabled(false);
        ui.pushButton_2->setDisabled(true);
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"call\", \"res\":\"����ʧ��\"}").toStdString());
    }
    sqllite.insertLog(Table::Log("PROGRAM", QString::fromLocal8Bit("����"), std::to_string(out).c_str()));
    logModel->select();
}

// ����ͨ��
void EXPANT::end()
{
    if (cmd_busy_flg)
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hang\", \"res\":\"�ظ�ָ��\"}").toStdString());
        return;
    }
    cmd_busy_flg = 1;
    if (!ui.pushButton_2->property("can").toBool())
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hang\", \"res\":\"�ѹҶ�\"}").toStdString());
        cmd_busy_flg = 0;
        return;
    }
    ui.lineEdit->setDisabled(false);
    ui.pushButton->setDisabled(false);
    ui.pushButton_2->setDisabled(true);
    ui.pushButton_2->setProperty("can", false);
    SQLLite sqllite;
    int hang = HangUpCtrl(0);
    int save = StopRecordFile(0);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("ֹͣ¼��"), std::to_string(save).c_str()));
    int close = setLocalRecord(0, FALSE);
    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("�ر�¼���˿�"), std::to_string(close).c_str()));
    if (hang == 0 && this->currentRecordID != -1)
    {
        int status = QueryPhoneStatus(0);
        if (!status)
        {
            QMessageBox message(QMessageBox::Information, QString::fromLocal8Bit("��Ϣ"),
                QString::fromLocal8Bit("��⵽����USB�ӿڿ������Ӳ����������²��뻰��USB��ͷ����\t"),
                QMessageBox::Yes, NULL);
            message.setWindowIcon(QIcon(":/resources/image/msg.png"));
            message.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("ȷ��"));
            message.setWindowFlags(message.windowFlags() | Qt::WindowStaysOnTopHint);
        }
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hang\", \"res\":\"�һ��ɹ�\"}").toStdString());
        end_time = time(NULL);
        double t = difftime(end_time, start_time);
        QString duration = QString("%1s").arg(t);
        Table::CallRecord r;
        r = sqllite.getRecord(this->currentRecordID);
        r.duration = duration;
        bool needUpload = false;
        if (save == 0)
        {
            ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"saveRec\", \"res\":\"����¼���ɹ�\"}").toStdString());
            if (r.token == "-")
            {
                r.status = QString::fromLocal8Bit("�ѱ���");
            }
            else
            {
                r.status = QString::fromLocal8Bit("�ȴ��ϴ�");
                needUpload = true;
            }   
        }
        else
        {
            ui.lineEdit->setDisabled(true);
            ui.pushButton->setDisabled(true);
            ui.pushButton_2->setDisabled(false);
            r.duration = QString::fromLocal8Bit("�쳣");
            ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"saveRec\", \"res\":\"����¼��ʧ��\"}").toStdString());
            r.status = "-";
            r.path = "-";
        }
        sqllite.updateCallRecord(r);
        if (needUpload)
        {
            // ��ʼ�첽�ϴ�¼��
            shared_ptr<Task> t = make_shared<Task>(this->currentRecordID, TaskType::UPLOAD);
            connect(t.get(), &Task::upload, this, &EXPANT::upload, Qt::BlockingQueuedConnection);
            TaskQueue::Instance()->push(t);
        }
        model->select();
    }
    else
    {
        ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"hang\", \"res\":\"�һ�ʧ��\"}").toStdString());
    }
    sqllite.insertLog(Table::Log("PROGRAM", QString::fromLocal8Bit("�һ�"), std::to_string(hang).c_str()));
    logModel->select();
    cmd_busy_flg = 0;
    hook_flg = 0;
}

void EXPANT::call(QString phone, QString token)
{
    ui.lineEdit->setText(phone);
    ui.tokenInput->setText(token);
    ui.pushButton->clicked();
}

void EXPANT::hang()
{
    ui.pushButton_2->clicked();
}

void EXPANT::socketMsg(QString msg, bool init)
{
    if (init)
    {
        ui.label_5->setStyleSheet("color:green");
    }
    else
    {
        ui.label_5->setStyleSheet("color:red");
    }
    socketMsg(msg);
}

void EXPANT::socketMsg(QString msg)
{
    ui.label_4->setText(msg);
}

void EXPANT::cmdMsg(QString msg)
{
    ui.label_11->setText(msg);
}

void EXPANT::search()
{
    QString dateStr;
    QDate date = ui.dateEdit->date();
    dateStr = date.toString(Qt::DateFormat::ISODate);
    QDate dateEnd(date.year(), date.month(), date.day() + 1);
    model->setFilter(QObject::tr("call_time > '%1 00:00:00' and call_time < '%2 00:00:00'")
        .arg(dateStr).arg(dateEnd.toString(Qt::DateFormat::ISODate)));
    model->select();
}

void EXPANT::chooseLog(int index) 
{
    QString type[4] = {"", "PROGRAM", "SOCKET", "PHONE"};
    time_t t = time(0);
    t -= 60 * 60 * 24 * 3;
    char ch[64];
    strftime(ch, sizeof(ch), "%Y-%m-%d 00:00:00", localtime(&t));
    if (index != 0)
    {
        logModel->setFilter(QString("log_time>'%1' and type='%2'").arg(ch).arg(type[index]));
    }
    else
    {
        logModel->setFilter(QString("log_time>'%1'").arg(ch));
    }
    logModel->select();
}

// ������Ϣ����
bool EXPANT::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG* pMsg = reinterpret_cast<MSG*>(message);
        if (pMsg->message == WM_DEVICECHANGE)
        {
            SQLLite sqllite;
            WPARAM w = pMsg->wParam;
            LPARAM l = pMsg->lParam;
            UnBindWindow();
            InitDll();
            BindWindow(this->hwnd);
            if (DBT_DEVICEARRIVAL == w)
            {
                sqllite.insertLog(Table::Log("PHONE", "WM_DEVICECHANGE", "DBT_DEVICEARRIVAL"));
            }
            else if (DBT_DEVICEREMOVEPENDING == w || DBT_DEVICEREMOVECOMPLETE == w)
            {
                sqllite.insertLog(Table::Log("PHONE", "WM_DEVICECHANGE", "DBT_DEVICEREMOVEPENDING"));
            }
            logModel->select();
        }
        if (pMsg->message >= WM_USER + 701 && pMsg->message <= WM_USER + 710)
        {
            SQLLite sqllite;
            UINT u = pMsg->message;
            WPARAM w = pMsg->wParam;
            LPARAM l = pMsg->lParam;
            int val = (int)u - WM_USER;
            QString q;
            q = this->msg_map.find(val)->second;
            char* phone;
            switch (val)
            {
            case 701:
                ui.statusIcon1->setStyleSheet("color:green");
                break;
            case 702:
                ui.statusIcon1->setStyleSheet("color:red");
                break;
            case 703:
                ui.lineEdit->setDisabled(true);
                ui.pushButton->setDisabled(true);
                ui.pushButton_2->setDisabled(false);
                break;
            case 704:
                if (ui.pushButton_2->property("can").toBool())
                {
                    ui.pushButton_2->clicked();
                }
                else
                {
                    ui.lineEdit->setDisabled(false);
                    ui.pushButton->setDisabled(false);
                    ui.pushButton_2->setDisabled(true);
                }
                hook_flg = 0;
                break;
            case 705:
                phone = (char*)l;
                q.append(phone);
                comming = phone;
                ServerSC::Instance()->Replay(QString::fromLocal8Bit("{\"event\":\"comming\", \"res\":\"%1\"}").arg(phone).toStdString());
                break;
            case 707:
                qDebug() << (char)l;
                dtmt.append((char)l);
                q += dtmt;
                dtmt_index++;
                break;
            default:
                break;
            }
            if (val != 707)
            {
                if (dtmt_index)
                {
                    sqllite.insertLog(Table::Log("PHONE", QString::fromLocal8Bit("DTMT����:%1").arg(dtmt), std::to_string(707).c_str()));
                    dtmt = "";
                    dtmt_index = 0;
                }
                if (val == 701 || val == 702)
                {
                    if (last_phone_msg != val)
                    {
                        sqllite.insertLog(Table::Log("PHONE", q, std::to_string(val).c_str()));
                    }
                }
                else
                {
                    sqllite.insertLog(Table::Log("PHONE", q, std::to_string(val).c_str()));
                }
                logModel->select();
            }
            ui.status1->setText(q);
            last_phone_msg = val;
            return true;
        }
    }
    return false;
}

void EXPANT::OnUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    ui.progressBar->setMaximum(bytesTotal); //���ֵ
    ui.progressBar->setValue(bytesSent);  //��ǰֵ
}

void EXPANT::auth(QString url, QString username, QString code)
{
    QString key(username);
    while (key.length() < 16)
    {
        key.append(" ");
    }
    SQLLite sqllite;
    QString h = hardwareEncryption.arg(QString::fromStdWString(wmic.BaseBoard().serialNumber + wmic.BaseBoard().product))
        .arg(QString::fromStdWString(wmic.Processor()[0].processID))
        .arg(QString::fromStdWString(wmic.BIOS().serialNumber));
    QEventLoop loop2;
    QNetworkAccessManager* manager;
    QNetworkReply* _reply;
    manager = new QNetworkAccessManager(this);
    manager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QNetworkRequest request(url);
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("H-AES", Util::EncryptionAES(h.toStdString(), key.toStdString().c_str(), code.toStdString().c_str()).data());
    QTimer timer;
    timer.setInterval(15000); // ���ó�ʱʱ�� 15 ��
    timer.setSingleShot(true); // ���δ���
    _reply = manager->post(request, QByteArray("username=").append(username));
    connect(&timer, &QTimer::timeout, &loop2, &QEventLoop::quit);
    connect(_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), &loop2, &QEventLoop::quit);
    connect(manager, &QNetworkAccessManager::finished, &loop2, &QEventLoop::quit);
    timer.start();
    loop2.exec();
    Table::Log log("PROGRAM", QString::fromLocal8Bit("�����֤"), "");
    if (timer.isActive())
    {
        timer.stop();
        if (_reply->error() != QNetworkReply::NoError)
        {
            QVariant variant = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            log.res = std::to_string(nStatusCode).c_str();
            ServerSC::Instance()->Replay("{\"cmd\":\"auth\",\"data\": {\"success\": false}}");
        }
        else
        {
            QVariant variant = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            log.res = std::to_string(nStatusCode).c_str();
            if (nStatusCode == 200) // ��Ӧ200��Ϊ�ɹ�
            {
                QByteArray json = _reply->readAll();
                rapidjson::Document doc;
                qDebug() << json.data();
                ServerSC::Instance()->Replay(QString("{\"cmd\":\"auth\",\"data\": %1}").arg(json.data()).toStdString());
            }
            else
            {
                ServerSC::Instance()->Replay("{\"cmd\":\"auth\",\"data\": {\"success\": false}}");
            }
        }
    }
    else
    {
        _reply->abort();
        _reply->deleteLater();
        log.res = std::to_string(-1).c_str();
        ServerSC::Instance()->Replay("{\"cmd\":\"auth\",\"data\": {\"success\": false}}");
    }
    sqllite.insertLog(log);
    ui.progressBar->setHidden(true);
    this->model->select();
    Sleep(500);
    disconnect(&timer, &QTimer::timeout, &loop2, &QEventLoop::quit);
    disconnect(manager, &QNetworkAccessManager::finished, &loop2, &QEventLoop::quit);
    manager->deleteLater();
}
