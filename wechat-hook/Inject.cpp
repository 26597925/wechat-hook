#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>
#include <wchar.h>
#include <TlHelp32.h>
#include "Inject.h"

// ��ȡע��DLLȫ·��
LPSTR GetDllPath(LPCSTR dllName)
{
	char szPath[0x1000] = { 0 };
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	(strrchr(szPath, '\\'))[0] = 0; // ɾ���ļ�����ֻ���·���ִ�
	TCHAR paths[0x1000] = { 0 };
	sprintf_s(paths, "%s\\%s", szPath, dllName);
	return paths;
}

// ͨ���������Ʋ��ҽ���ID
DWORD ProcessNameToPID(LPCSTR processName)
{
	TCHAR buffText[0x100] = { 0 };
	// ��ȡ���̿���
	// #include <TlHelp32.h>
	HANDLE ProcessAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 processInfo = { 0 };
	processInfo.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (strcmp(processName, processInfo.szExeFile) == 0) {
			sprintf_s(buffText, "��������=%s ����ID=%d \r\n", processInfo.szExeFile, processInfo.th32ProcessID);
			OutputDebugString(buffText);
			return processInfo.th32ProcessID;
		}
	} while (Process32Next(ProcessAll, &processInfo));

	return 0;
}

// ע��dll
VOID injectDll(char * dllPath)
{
	TCHAR buff[0x100] = { 0 };
	// 1.��ȡ��΢�ž��
	DWORD PID = ProcessNameToPID(INJECT_PROCESS_NAME);
	if (PID == 0) {
		MessageBox(NULL, "δ�ҵ�΢�Ž��̻�΢��δ����", "����", MB_OK);
		return;
	}
	// 2.���ҵ���PID�򿪻�ȡ���ľ��
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
	if (NULL == hProcess) {
		MessageBox(NULL, "���̴�ʧ�ܣ�����Ȩ�޲�����߹ر���Ӧ��", "����", MB_OK);
		return;
	}
	// 3.�����ڴ�
	DWORD strSize = strlen(dllPath) * 2;
	//���̴򿪺����ǰ����ǵ�dll·�����ȥ
	//��������һƬ�ڴ����ڴ���dll·��
	LPVOID allocRes = VirtualAllocEx(hProcess, NULL, strSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == allocRes) {
		MessageBox(NULL, "�ڴ�����ʧ��", "����", MB_OK);
		return;
	}

	// 4.dll·��д�뵽������ڴ���
	if (WriteProcessMemory(hProcess, allocRes, dllPath, strSize, NULL) == 0) {
		MessageBox(NULL, "DLL·��д��ʧ��", "����", MB_OK);
		return;
	}
	// ·��д�� �ɹ����������ڻ�ȡLoadLibraryW ��ַ
	// LoadLibraryW ��Kernel32.dll���� ���������Ȼ�ȡ���dll�Ļ�ַ
	HMODULE hModule = GetModuleHandle("Kernel32.dll");
	LPVOID address = GetProcAddress(hModule, "LoadLibraryA");
	sprintf_s(buff, "loadLibrary=%p path=%p", address, allocRes);
	OutputDebugString(buff);
	// ͨ��Զ���߳�ִ��������� �������� ����dll�ĵ�ַ
	// ��ʼע��dll
	HANDLE hRemote = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, allocRes, 0, NULL);
	if (NULL == hRemote) {
		MessageBox(NULL, "Զ��ִ��ʧ��", "����", MB_OK);
		return;
	}
}

// ��ȡ�ڴ�
VOID readMemory()
{
	DWORD PID = ProcessNameToPID(INJECT_PROCESS_NAME);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
	LPCVOID phoneAdd = (LPCVOID)0x10611E80;
	DWORD reSize = 0xB;
	char buff[0x100] = { 0 };
	TCHAR buffTest[0x100] = { 0 };
	ReadProcessMemory(hProcess, phoneAdd, buff, reSize, NULL);
	sprintf_s(buffTest, "add=%p %s ", buff, buff);
	OutputDebugString(buffTest);
}


VOID setWindow(HWND thisWindow)
{
	HWND wechatWindow = FindWindow("WeChatMainWndForPC", NULL);
	//�ϣ�20 �£�620 ��10 �ң�720
	//MoveWindow(wechatWindow, 10, 20, 100, 600, TRUE);

	RECT wechatHandle = { 0 };
	GetWindowRect(wechatWindow, &wechatHandle);
	LONG width = wechatHandle.right - wechatHandle.left;
	LONG height = wechatHandle.bottom - wechatHandle.top;
	MoveWindow(thisWindow, wechatHandle.left - 230, wechatHandle.top, 240, height, TRUE);
	SetWindowPos(thisWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	TCHAR buff[0x100] = {};
	sprintf_s(buff, "�ϣ�%d �£�%d ��%d �ң�%d\r\n", wechatHandle.top, wechatHandle.bottom, wechatHandle.left, wechatHandle.right);

	OutputDebugString(buff);
}

//����΢��
//CreateProcess ����Ŀ����� ����ʱ������ý���.
//Ȼ��ע��
//Ȼ����ResumeThread ��Ŀ���������
VOID runWechat(TCHAR * dllPath, TCHAR * wechatPath)
{
	//injectDll(dllPath);
	//TCHAR szDll[] = dllPath;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;//SW_SHOW
	//TCHAR szCommandLine[MAX_PATH] = TEXT("D:\\Program Files (x86)\\Tencent\\WeChat\\WeChat.exe");

	CreateProcess(NULL, wechatPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	LPVOID Param = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	TCHAR add[0x100] = { 0 };

	WriteProcessMemory(pi.hProcess, Param, dllPath, strlen(dllPath) * 2 + sizeof(char), NULL);

	TCHAR buff[0x100] = { 0 };
	HMODULE hModule = GetModuleHandle("Kernel32.dll");
	LPVOID address = GetProcAddress(hModule, "LoadLibraryA");
	//ͨ��Զ���߳�ִ��������� �������� ����dll�ĵ�ַ
	//��ʼע��dll
	HANDLE hRemote = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, Param, 0, NULL);
	if (NULL == hRemote) {
		MessageBox(NULL, "Զ��ִ��ʧ��", "����", MB_OK);
		return;
	}

	DWORD rThread = ResumeThread(pi.hThread);
	if (-1 == rThread) {
		MessageBox(NULL, "΢�Ž��̻���ʧ��", "����", MB_OK);
		return;
	}
}