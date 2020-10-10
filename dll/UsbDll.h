#ifndef _USBDLL_H_
#define _USBDLL_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the USBDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// USBDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef USBDLL_EXPORTS
#define USBDLL_API __declspec(dllexport)
#else
#define USBDLL_API __declspec(dllimport)
#endif


#include <wtypes.h>
#include <initguid.h>

#define		RECORD_LOCAL 0
#define		RECORD_TALK  1
#define		RECORD_LY    2
// the following functions' definition and macros' definition are the APIs for USB protocol

//----from Phone to PC--------------------------

/*
//设备连接时发此消息（指USB线与PC和终端电话都连接好）.
//参数wParam：(BYTE)序号;  lParam: 0L.
*/
#define WM_DEVICECONNECT	 WM_USER+701

/*
//设备断开时发此消息（指USB线与PC或终端电话的任一方断开时发此消息）.
//参数wParam：(BYTE)0L--全部断开;  lParam: 0L.
//                  1L--断开一台话机;  lParam: (BYTE)序号
*/	
#define WM_DEVICEDISCONNECT		WM_USER+702	

/*
//呼叫建立,即摘机时发送此消息. 
//参数wParam：(BYTE)序号;  lParam: 0L.
*/
#define WM_OFFHOOK		WM_USER+703		

/*
//呼叫断开，即挂机时发送此消息. 
//参数wParam：(BYTE)序号;  lParam: 0L--挂机，1L--终端在保留的状态下按免提键退出了保留.
*/
#define WM_ONHOOK		WM_USER+704	
	
/*
//来电时发送来电号码.
//参数wParam：(BYTE)序号;   lParam:char*.
//普通的号码长度>1；如果长度是1，号码是2，那代表“出局”，号码是1，那代表“保密”。
*/
#define WM_CALLERID		WM_USER+705		

/*
//来电响铃发此消息.
//参数wParam：(BYTE)序号;    lParam: (BYTE)0x01—表示一次铃声响；0x00—表示此次铃声结束
*/
#define WM_RING		WM_USER+706		

/*
//通话过程中，收到本端DTMF键值时，发此消息.
//参数wParam: (BYTE)序号;	lParam: char.
*/
#define WM_KEY	WM_USER+707

/*
//留言过程中，话机接听电话，停止留言，发此消息.
//参数wParam: (BYTE)序号;	lParam: 0L.
*/
#define WM_STOPLY	WM_USER+708

/*
//通话过程中，收到话机按“静音”键
//参数wParam: (BYTE)序号;	lParam: (BYTE)0x01--静音开启；0x00--静音关闭.
*/
#define WM_MUTE		WM_USER+709



extern "C" {
//---------from PC to Phone-------------------------
// 若无说明，则返回 0 代表成功
USBDLL_API int _stdcall InitDll();					//初始化，仅调用一次

USBDLL_API int _stdcall OffHookCtrl(int iDevIdx);					//发送摘机命令

USBDLL_API int _stdcall HangUpCtrl(int iDevIdx);					//发送挂机命令

USBDLL_API int _stdcall StartDial(int iDevIdx,const char* szDest);	//发送拨号命令

USBDLL_API int _stdcall Bell(int iDevIdx,BYTE mode);					//设置振铃开或关,mode: 0--关闭; 1--打开

USBDLL_API int _stdcall SendDTMF(int iDevIdx,const char* szDTMF);	//与StartDial功能相似,供二次拨号用

USBDLL_API int _stdcall BindWindow(HWND hwnd);			//绑定窗口，所有事件发送到此窗口，初始化时调用；系统接收消息 WM_DEVICECHANGE 后调用此函数

USBDLL_API int _stdcall UnBindWindow();					//解除窗口绑定, 则所有事件将不会发送到此窗口，窗口销毁时调用此函数

USBDLL_API int _stdcall QueryPhoneStatus(int iDevIdx);			//查询话机摘挂机状态 0->挂机,1->摘机

USBDLL_API int _stdcall setLocalRecord(int iDevIdx,BOOL rec);	//本地录放音，开 pc 时， rec: false--关闭;true--打开 

USBDLL_API int _stdcall setTalkRecord(int iDevIdx,BOOL rec);		//通话录音，rec: false--关闭;true--打开 

USBDLL_API int _stdcall setLeaveRecord(int iDevIdx,BOOL rec);		//留言录音，rec: false--关闭;true--打开 

USBDLL_API int _stdcall GetSerialNo(int iDevIdx, char * number);	//获取序列号

USBDLL_API int _stdcall SetSerialNo(int iDevIdx, const char * number);	//设置序列号

USBDLL_API int _stdcall Flash(int iDevIdx,UINT ivalue);	//闪断一下，ivalue--Flash操作的时间长度,取值为0--100ms,1--180ms,2--300ms,3--600ms,4--1000ms之间。

USBDLL_API int _stdcall SetDialTone(int iDevIdx,BYTE mode);				//设置拨号音开或关,mode: 0--关闭; 1--打开

USBDLL_API int _stdcall SetAutoAnswer(int iDevIdx,BYTE mode);			//设置自动接听开或关,mode: 0--关闭; 1--打开

USBDLL_API int _stdcall SetFlashTime(int iDevIdx,UINT ivalue);			//设置Flash值, ivalue 取值为0--100ms,1--180ms,2--300ms,3--600ms,4--1000ms之间

USBDLL_API int _stdcall SetOutcode(int iDevIdx, const char * code);		//设置出局码，最多3位

USBDLL_API int _stdcall StartRecordFile(int iDevIdx, const char* strFileName, int iType);	//开始录音操作, strFileName: 录音文件名，完整的路径
																		//如:"C:\\record\\sound.wav"。iType: 录音类型：0:本地录音；1:通话录音；2:留言录音

USBDLL_API int _stdcall StopRecordFile(int iDevIdx);	//停止录音

USBDLL_API int _stdcall ZhuanBo(int iDevIdx,UINT ivalue);	//转拨闪断一下，ivalue--转拨操作的时间长度,取值为0--100ms,1--180ms,2--300ms,3--600ms,4--1000ms之间。

USBDLL_API int _stdcall Hold(int iDevIdx, BOOL bOn);	//bOn--1:开启保留；0:关闭保留

USBDLL_API int _stdcall Mute(int iDevIdx, BOOL bOn);	// bOn->1:开启闭音；0:关闭闭音；
}
#endif