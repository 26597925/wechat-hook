#include "stdafx.h"
#include <string>
#include <strstream>
#include <iostream>
#pragma comment(lib,"Version.lib")
#include "message.h"
using namespace std;

string GetWxVersion() {
	string asVer = "";

	DWORD wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	WCHAR VersionFilePath[MAX_PATH];
	if (GetModuleFileName((HMODULE)wxBaseAddress, VersionFilePath, MAX_PATH) == 0)
	{
		return asVer;
	}
	VS_FIXEDFILEINFO* pVsInfo;
	unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
	int iVerInfoSize = GetFileVersionInfoSize(VersionFilePath, NULL);
	if (iVerInfoSize != 0) {
		char* pBuf = new char[iVerInfoSize];
		if (GetFileVersionInfo(VersionFilePath, 0, iVerInfoSize, pBuf)) {
			if (VerQueryValue(pBuf, TEXT("\\"), (void**)& pVsInfo, &iFileInfoSize)) {
				//���汾2.6.8.52
				//2
				int s_major_ver = (pVsInfo->dwFileVersionMS >> 16) & 0x0000FFFF;
				//6
				int s_minor_ver = pVsInfo->dwFileVersionMS & 0x0000FFFF;
				//8
				int s_build_num = (pVsInfo->dwFileVersionLS >> 16) & 0x0000FFFF;
				//52
				int s_revision_num = pVsInfo->dwFileVersionLS & 0x0000FFFF;

				//�Ѱ汾����ַ���
				strstream wxVer;
				wxVer << s_major_ver << "." << s_minor_ver << "." << s_build_num << "." << s_revision_num;
				wxVer >> asVer;
			}
		}
		delete[] pBuf;
	}
	return asVer;
}


//************************************************************
// ��������: IsWxVersionValid
// ����˵��: ���΢�Ű汾�Ƿ�ƥ��
//************************************************************
BOOL IsWxVersionValid(string wxVersoin)
{
	string asVer = GetWxVersion();
	//MessageBoxA(NULL, asVer.data(), "Tip", MB_OK);
	//�汾ƥ��
	if (asVer == wxVersoin)
	{
		return TRUE;
	}
	//�汾��ƥ��
	return FALSE;
}


//************************************************************
// ��������: CheckIsLogin
// ����˵��: ���΢���Ƿ��½
//************************************************************
void CheckIsLogin()
{
	//��ȡWeChatWin�Ļ�ַ
	DWORD  dwWeChatWinAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll");

	while (true)
	{
		Sleep(100);
		DWORD dwIsLogin = dwWeChatWinAddr + LoginSign_Offset + 0x194;
		if (*(DWORD*)dwIsLogin != 0)
		{
			//���͵����ƶ�
			COPYDATASTRUCT login_msg;
			login_msg.dwData = WM_Login;
			login_msg.lpData = NULL;
			login_msg.cbData = 0;
			SendMessageByThread(&login_msg);
			break;
		}
	}
}