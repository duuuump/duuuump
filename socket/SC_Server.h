#pragma once

#ifndef _WEB_COMMAND_H_
#define _WEB_COMMAND_H_
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "ScoketComm.h"

#include <windows.h>
#include <string.h>
#include <thread>
#include <functional>
#include <iostream>
#include <set>

#ifdef _UNICODE
typedef std::wstring tstring;
#define tcout std::wcout
#define tcin std::wcin
#else
typedef std::string tstring;
#define tcout std::cout
#define tcin std::cin
#endif

using namespace std;

enum ScoketType
{
	Normal,
	Web,
	ActionScript,
};

//typedef void (*CallBackComm)(SOCKET comm);
typedef std::function<void (SOCKET comm)> CallBackComm;

class ServerSC : public ScoketComm
{
public:
	//ʵ�ֵ���ģʽ
	static ServerSC* Instance()
	{
		static ServerSC obj;
		return &obj;
	}

public:
	ServerSC(void) {}
	~ServerSC(void) {}

	/************************************************************************/
	/* ����
	* @return ������ɵ�ServerSC����
	* @param pCallBack �ص�ͨ�ź���
	* @param port �˿ں�
	* @param ip IP��ַ
	* @param resolve_ip ������ַ
	* @param type Scoket����
	* // [6/24/2016 Wings] */
	/************************************************************************/
	bool Create(CallBackComm pCallBack, const int port, const char *ip = NULL, 
		BOOL resolve_ip = FALSE, ScoketType type = Normal);

	/************************************************************************/
	/* ������Ϣ
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Send(const SOCKET &scoket, const char *data, int len);
	/************************************************************************/
	/* ������Ϣ
	* // [7/18/2016 Wings] */
	/************************************************************************/
	virtual BOOL Recv(SOCKET &scoket, char *const data, int len, int &size);

	BOOL Close(SOCKET& scoket);

	static set<SOCKET> clients;

	void Replay(string msg);
protected:
private:
	/************************************************************************/
	/* �ȴ�����ͨ������
	* @param server �������׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Accept(SOCKET server, ScoketType type);

	/************************************************************************/
	/* ����
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool Handshake(SOCKET comm, ScoketType type);

	/************************************************************************/
	/* ����Web
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeWeb(SOCKET comm);

	/************************************************************************/
	/* ����ActionScript
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	bool HandshakeActionScript(SOCKET comm);

	/************************************************************************/
	/* ͨ��
	* @param comm ͨ���׽���
	* // [7/15/2016 Wings] */
	/************************************************************************/
	void Command(SOCKET comm);

	/************************************************************************/
	/* ����
	* @return ���ܽ��
	* // [6/24/2016 Wings] */
	/************************************************************************/
	char* HashString(std::string szMsg);

	SOCKET m_sServer; // �������׽��� [6/29/2016 Wings]

	CallBackComm m_fCallBack; // �ص�ͨ�ź��� [10/13/2016 Wings]
};

#endif