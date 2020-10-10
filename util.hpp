#pragma once
#include <Windows.h>
#include <qstring.h>
#include "stdafx.h"
#include <sys/stat.h>
#include <tchar.h>
#include "crypto/aes.h"
#include "socket/base64.h"

namespace Util 
{
	// 0:正常 -1：找不到lame.exe -2:创建任务失败 -3:wav文件不存在
	static int wav2mp3(const char* input)
	{
		struct stat buffer;
		if (stat("lame.exe", &buffer) != 0)
		{
			return -1;
		}
        if (stat(input, &buffer) != 0)
        {
            return -3;
        }
        DWORD    dwExitCode = -1;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        QString nname(input);
        nname.replace(nname.lastIndexOf("."), 4, ".mp3");
        QString p("lame.exe ");
        p.append(input).append(" ").append(nname);
        LPWSTR cmdLine = (LPWSTR)p.unicode();
        if (!CreateProcess(NULL, // an exe file.   
            cmdLine,        // parameter for your exe file.   
            NULL,             // Process handle not inheritable.   
            NULL,             // Thread handle not inheritable.   
            FALSE,            // Set handle inheritance to FALSE.   
            CREATE_NO_WINDOW,                // No creation flags.   
            NULL,             // Use parent's environment block.   
            NULL,             // Use parent's starting directory.   
            &si,              // Pointer to STARTUPINFO structure.  
            &pi)             // Pointer to PROCESS_INFORMATION structure.  
            )
        {
            return -2;
        }
        // Wait until child process exits.  
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &dwExitCode);
        // Close process and thread handles.   
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (remove(input) != 0)
        {
            qDebug() << "删除WAV失败";
        }
        return (int)dwExitCode;
	}

    static string EncryptionAES(const string& strSrc, const char* pKey, const char* pIV)
    {
        size_t length = strSrc.length();
        int block_num = length / BLOCK_SIZE + 1;
        //明文
        char* szDataIn = new char[block_num * BLOCK_SIZE + 1];
        memset(szDataIn, 0x00, block_num * BLOCK_SIZE + 1);
        strcpy(szDataIn, strSrc.c_str());

        //进行PKCS7Padding填充。
        int k = length % BLOCK_SIZE;
        int j = length / BLOCK_SIZE;
        int padding = BLOCK_SIZE - k;
        for (int i = 0; i < padding; i++)
        {
            szDataIn[j * BLOCK_SIZE + k + i] = padding;
        }
        szDataIn[block_num * BLOCK_SIZE] = '\0';
        char* szDataOut = new char[block_num * BLOCK_SIZE + 1];
        memset(szDataOut, 0, block_num * BLOCK_SIZE + 1);

        //进行进行AES的CBC模式加密
        AES aes;
        aes.MakeKey(pKey, pIV, 16, 16);
        aes.Encrypt(szDataIn, szDataOut, block_num * BLOCK_SIZE, AES::CBC);
        //aes.Encrypt(szDataIn, szDataOut, block_num * BLOCK_SIZE, AES::ECB);
        string str = base64_encode((unsigned char*)szDataOut, block_num * BLOCK_SIZE);
        delete[] szDataIn;
        delete[] szDataOut;
        return str;
    }

    static string DecryptionAES(const string& strSrc, const char* pKey, const char* pIV)
    {
        string strData = base64_decode(strSrc);
        size_t length = strData.length();
        //密文
        char* szDataIn = new char[length + 1];
        memcpy(szDataIn, strData.c_str(), length + 1);
        //明文
        char* szDataOut = new char[length + 1];
        memcpy(szDataOut, strData.c_str(), length + 1);

        //进行AES的CBC模式解密
        AES aes;
        aes.MakeKey(pKey, pIV, 16, 16);//当keylength为32时，为256位，16为128位
        //aes.Decrypt(szDataIn, szDataOut, length, AES::CBC);
        aes.Decrypt(szDataIn, szDataOut, length, AES::ECB);
        //去PKCS7Padding填充
        if (0x00 < szDataOut[length - 1] && szDataOut[length - 1] <= 0x16)
        {
            int tmp = szDataOut[length - 1];
            for (int i = length - 1; i >= length - tmp; i--)
            {
                if (szDataOut[i] != tmp)
                {
                    memset(szDataOut, 0, length);
                    //cout << "去填充失败!解密出错!!" << endl;
                    break;
                }
                else
                    szDataOut[i] = 0;
            }
        }
        string strDest(szDataOut);
        delete[] szDataIn;
        delete[] szDataOut;
        return strDest;
    }

    static QString getSysHome()
    {
        LPCWSTR homeProfile = L"USERPROFILE";
        wchar_t p[1024] = {0};
        unsigned int pathSize = GetEnvironmentVariable(homeProfile, p, 1024);

        if (pathSize == 0 || pathSize > 1024)
        {
            // 获取失败 或者 路径太长 
            int ret = GetLastError();
            return "";
        }
        else
        {
            return QString::fromStdWString(p);
        }
    }
}