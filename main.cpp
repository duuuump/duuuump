#include "expant.h"
#include "stdafx.h"
#include <windows.h>
#include <vector>
#include <QtWidgets/QApplication>
#include "socket/SC_Server.h"
#include "socket/websocket_request.h"
#include <stringbuffer.h>
#include <writer.h>
#include <document.h>
#include "SQL/sqllite.h"
#include "taskhandler.hpp"
#include "qsslconfiguration.h"

ServerSC* m_pWebServer;
Websocket_Request* request_;// webSocket返回数据解析
EXPANT *pro;
SQLLite sqllite;

void createWebServer(int port);
void WebServerCommand(SOCKET comm);
void taskExec();

int main(int argc, char *argv[])
{
	auto hMutex = CreateMutex(NULL, FALSE, L"_phone_connector"); //命名Mutex是全局对象    
	if (hMutex == NULL || ERROR_ALREADY_EXISTS == ::GetLastError()) 
	{
		::MessageBox(NULL, L"已经有一个客户端在运行，不要重复运行！\t", L"WARING", MB_OK);
		return 0;
	}
    QApplication a(argc, argv);
	QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
	sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
	QSslConfiguration::setDefaultConfiguration(sslConf);
    EXPANT w;
    w.show();
    a.installNativeEventFilter(&w);
	pro = &w;
	int port = w.getSocketPort();
    createWebServer(port);
	std::thread(taskExec).detach();
    return a.exec();
}

void taskExec() {
	//启动任务队列
	TaskQueue::Instance()->start();
	while (TaskQueue::Instance()->size() > 0)
	{
		std::chrono::milliseconds dura(1);
		std::this_thread::sleep_for(dura);
	}
}

void createWebServer(int port)
{

	request_ = new Websocket_Request();
	m_pWebServer = new ServerSC();

	//非线程阻塞
	auto callback = std::bind(&WebServerCommand, std::placeholders::_1);
	if (!m_pWebServer->Create(callback, port, "127.0.0.1", FALSE, Web))
	{
		pro->socketMsg(QString::fromLocal8Bit("初始化失败!"), false);
	}
	else
	{
		pro->socketMsg(QString::fromLocal8Bit("SOCKET端口【%1】").arg(QString(std::to_string(port).c_str())), true);
	}
}

void WebServerCommand(SOCKET comm)
{
	bool bLoop = true;
	char data[4096];
	int recvSize;
	ServerSC::clients.insert(comm);
	int sum = m_pWebServer->GetCommSum();
	pro->socketMsg(QString::fromLocal8Bit("SOCKET【%1个连接】")
		.arg(std::to_string(ServerSC::clients.size()).c_str()));
	std::vector<char> recvDatas;

	while (bLoop)
	{
		memset(data, 0, 4096 * sizeof(char));
		BOOL recResult = m_pWebServer->Recv(comm, data, sizeof(data), recvSize);
		if (!recResult)
		{
			m_pWebServer->GetScoketError();
			m_pWebServer->Close(comm);
			ServerSC::clients.erase(comm);
			pro->socketMsg(QString::fromLocal8Bit("SOCKET【%1个连接】")
				.arg(std::to_string(ServerSC::clients.size()).c_str()));
			break;
		}

		Websocket_Request request;
		recvDatas.insert(recvDatas.end(), data, data + recvSize);
		request.fetch_websocket_info(recvDatas);
		if (request.getOpenCode() == 8)
		{
			ServerSC::clients.erase(comm);
			m_pWebServer->Close(comm);
			pro->socketMsg(QString::fromLocal8Bit("SOCKET【%1个连接】")
				.arg(std::to_string(ServerSC::clients.size()).c_str()));
			break;
		}
		string resultStr = request.getDataStr();

		if (resultStr.size() > 0)
		{
			rapidjson::Document doc;
			if (!doc.Parse(resultStr.data()).HasParseError()) {
				if (doc.HasMember("cmd") && doc["cmd"].IsString()) //忽略非指令请求
				{
					QString cmd = doc["cmd"].GetString();
					QString str = QString::fromLocal8Bit("指令【%1】");
					if (cmd == "call") // 拨号
					{
						pro->cmdMsg(str.arg(QString::fromLocal8Bit("拨号")));
						if (doc.HasMember("phone") && doc["phone"].IsString() && doc.HasMember("token") && doc["token"].IsString())
						{
							QString phone = doc["phone"].GetString();
							QString token = doc["token"].GetString();
							pro->call(phone, token);
						}
						else
						{
							pro->cmdMsg(QString::fromLocal8Bit("错误指令"));
						}
					}
					else if (cmd == "hang") // 挂机
					{
						pro->cmdMsg(str.arg(QString::fromLocal8Bit("挂机")));
						pro->hang();
					}
					else if (cmd == "hook") // 摘机
					{
						if (doc.HasMember("token") && doc["token"].IsString()) 
						{
							QString token = doc["token"].GetString();
							pro->ui.tokenInput->setText(token);
						}
						else
						{
							pro->ui.tokenInput->setText("-");
						}
						pro->ui.pushButton_5->clicked();
						pro->cmdMsg(str.arg(QString::fromLocal8Bit("摘机")));
					}
					else if (cmd == "set_upload") // 初始化设置录音上传URL
					{
						pro->cmdMsg(str.arg(QString::fromLocal8Bit("配置")));
						if (doc.HasMember("data") && doc["data"].IsString())
						{
							QString addr = doc["data"].GetString();
							sqllite.updateConfig("UPLOAD_ADDR", addr);
							pro->ui.lineEdit_4->setText(addr);
						}
						else
						{
							pro->cmdMsg(QString::fromLocal8Bit("错误指令"));
						}
					}
					else if (cmd == "refresh_token") // 刷新上传token
					{
						if (doc.HasMember("cid") && doc["cid"].IsInt() && doc.HasMember("token") && doc["token"].IsString())
						{
							int id = doc["cid"].GetInt();
							QString token = doc["token"].GetString();
							Table::CallRecord r = sqllite.getRecord(id);
							r.token = token;
							sqllite.updateCallRecord(r);
						}
						else
						{
							pro->cmdMsg(QString::fromLocal8Bit("错误指令"));
						}
					}
					else if (cmd == "auth") // 认证请求
					{
						pro->cmdMsg(str.arg(QString::fromLocal8Bit("认证")));
						if (doc.HasMember("url") && doc["url"].IsString() 
							&& doc.HasMember("username") && doc["username"].IsString() 
							&& doc.HasMember("code") && doc["code"].IsString())
						{
							QString url = doc["url"].GetString();
							QString username = doc["username"].GetString();
							QString code = doc["code"].GetString();
							pro->auth(url, username, code);
						}
						else
						{
							pro->cmdMsg(QString::fromLocal8Bit("错误指令"));
						}
					}
				}
				else
				{
					pro->cmdMsg(QString::fromLocal8Bit("错误指令"));
				}
			}
		}
	}
}
