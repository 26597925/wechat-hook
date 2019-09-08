#include "stdafx.h"
#include "Inject.h"
#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <AtlConv.h>
#include <direct.h>

#define INJECT_PROCESS_NAME "WeChat.exe"
#define INJECT_DLL_NAME "wechat-inject-helper.dll"

//***********************************************************
// ��������: GetDllPath
// ����˵��: ��ȡע��DLLȫ·��
//***********************************************************
char* GetDllPath(const char* dllName)
{
	char szPath[MAX_PATH] = { 0 };
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	(strrchr(buffer, '\\'))[0] = 0; // ɾ���ļ�����ֻ���·���ִ�
	sprintf_s(szPath, "%s\\%s", buffer, dllName);
	return szPath;

	//��ȡ��ǰ����Ŀ¼�µ�dll
	//char szPath[MAX_PATH] = { 0 };
	//char* buffer = NULL;
	//if ((buffer = _getcwd(NULL, 0)) != NULL)
	//{
	//	sprintf_s(szPath, "%s\\%s", buffer, dllName);
	//}
	//return szPath;
}

//***********************************************************
// ��������: ProcessNameToPID
// ����˵��: ͨ���������Ʋ��ҽ���ID
//***********************************************************
DWORD ProcessNameToPID(const char* ProcessName)
{
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hProcess, &pe32) == TRUE)
	{
		do
		{
			USES_CONVERSION;
			if (strcmp(ProcessName, pe32.szExeFile) == 0)
			{
				return pe32.th32ProcessID;
			}
		} while (Process32Next(hProcess, &pe32));
	}
	return 0;
}

//************************************************************
// ��������: CheckIsInject
// ����˵��: ����Ƿ��Ѿ�ע��dll
//************************************************************
BOOL CheckIsInject(DWORD dwProcessid)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	//��ʼ��ģ����Ϣ�ṹ��
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
	//����ģ����� 1 �������� 2 ����ID
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessid);
	//��������Ч�ͷ���FALSE
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		//MessageBox(NULL, "����ģ�����ʧ��", "����", MB_OK);
		return FALSE;
	}
	//ͨ��ģ����վ����ȡ��һ��ģ�����Ϣ
	if (!Module32First(hModuleSnap, &me32))
	{
		//MessageBox(NULL, "��ȡ��һ��ģ�����Ϣʧ��", "����", MB_OK);
		//��ȡʧ����رվ��
		CloseHandle(hModuleSnap);
		return FALSE;
	}
	do
	{
		if (strcmp(me32.szModule, INJECT_DLL_NAME) == 0)
		{
			return FALSE;
		}

	} while (Module32Next(hModuleSnap, &me32));

	return TRUE;
}

//***********************************************************
// ��������: InjectDll
// ����˵��: ע��dll
//***********************************************************
BOOL InjectDll()
{
	// 1.��ȡ��΢�ž��
	DWORD dwPid = ProcessNameToPID(INJECT_PROCESS_NAME);
	if (dwPid == 0) {
		//MessageBox(NULL, "δ�ҵ�΢�Ž��̻�΢��δ����", "����", MB_OK);
		return FALSE;
	}
	//���dll�Ƿ��Ѿ�ע��
	if (!CheckIsInject(dwPid)) {
		//MessageBox(NULL, "�ظ�ע��", "����", MB_OK);
		return FALSE;
	}
	// 2.���ҵ���PID�򿪻�ȡ���ľ��
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwPid);
	if (NULL == hProcess) {
		//MessageBox(NULL, "���̴�ʧ�ܣ�����Ȩ�޲�����߹ر���Ӧ��", "����", MB_OK);
		return FALSE;
	}
	// 3.�ڽ����������ڴ�
	char* dllPath = GetDllPath(INJECT_DLL_NAME);
	DWORD strSize = strlen(dllPath) * 2;
	// ���̴򿪺����ǰ����ǵ�dll·�����ȥ
	// ��������һƬ�ڴ����ڴ���dll·��
	LPVOID pAddress = VirtualAllocEx(hProcess, NULL, strSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (NULL == pAddress) {
		//MessageBox(NULL, "�ڴ�����ʧ��", "����", MB_OK);
		return FALSE;
	}

	// 4.д��dll·��������
	DWORD dwWrite = 0;
	if (WriteProcessMemory(hProcess, pAddress, dllPath, strSize, &dwWrite) == 0) {
		//MessageBox(NULL, "DLL·��д��ʧ��", "����", MB_OK);
		return FALSE;
	}
	// ��ȡLoadLibraryA������ַ		LoadLibraryA��Kernel32.dll���� ���������Ȼ�ȡ���dll�Ļ�ַ
	LPVOID pLoadLibraryAddress = GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA");
	if (pLoadLibraryAddress == NULL)
	{
		//MessageBox(NULL, "��ȡLoadLibraryA������ַʧ��", "����", 0);
		return FALSE;
	}
	// ͨ��Զ���߳�ִ��������� �������� ����dll�ĵ�ַ
	HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryAddress, pAddress, 0, NULL);
	if (NULL == hRemoteThread) {
		//MessageBox(NULL, "Զ���߳�ע��ʧ��", "����", MB_OK);
		return FALSE;
	}

	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);

	return TRUE;
}

//************************************************************
// ��������: UnloadDll
// ����˵��: ж��DLL
//************************************************************
BOOL UnloadDll()
{
	//��ȡ΢��Pid
	DWORD dwPid = ProcessNameToPID(INJECT_PROCESS_NAME);
	if (dwPid == 0)
	{
		//MessageBox(NULL, "û���ҵ�΢�Ž��̻���΢��û������", "����", 0);
		return FALSE;
	}

	//����ģ��
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
	MODULEENTRY32 ME32 = { 0 };
	ME32.dwSize = sizeof(MODULEENTRY32);
	BOOL isNext = Module32First(hSnap, &ME32);
	BOOL flag = FALSE;
	while (isNext)
	{
		USES_CONVERSION;
		if (strcmp(ME32.szModule, INJECT_DLL_NAME) == 0)
		{
			flag = TRUE;
			break;
		}
		isNext = Module32Next(hSnap, &ME32);
	}
	if (flag == FALSE)
	{
		//MessageBox(NULL, "�Ҳ���Ŀ��ģ��", "����", 0);
		return FALSE;
	}

	//��Ŀ�����
	HANDLE hPro = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	//��ȡFreeLibrary������ַ
	FARPROC pFun = GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");

	//����Զ���߳�ִ��FreeLibrary
	HANDLE hThread = CreateRemoteThread(hPro, NULL, 0, (LPTHREAD_START_ROUTINE)pFun, ME32.modBaseAddr, 0, NULL);
	if (!hThread)
	{
		MessageBox(NULL, "����Զ���߳�ʧ��", "����", 0);
		return FALSE;
	}

	CloseHandle(hSnap);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hPro);
	MessageBox(NULL, "dllж�سɹ�", "Tip", 0);
	return TRUE;
}

//***********************************************************
// ��������: ReadMemory
// ����˵��: ��ȡ�ڴ�dll
//***********************************************************
void ReadMemory()
{
	DWORD PID = ProcessNameToPID(INJECT_PROCESS_NAME);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
	LPCVOID phoneAdd = (LPCVOID)0x10611E80;
	DWORD reSize = 0xB;
	char buff[0x100] = { 0 };
	char buffTest[0x100] = { 0 };
	ReadProcessMemory(hProcess, phoneAdd, buff, reSize, NULL);
	sprintf_s(buffTest, "add=%p %s ", buff, buff);
	OutputDebugString(buffTest);
}

//***********************************************************
// ��������: RunWechat
// ����˵��: ����΢��
//			 CreateProcess ����Ŀ����� ����ʱ������ý���.
//			 Ȼ��ע��
//			 Ȼ����ResumeThread ��Ŀ���������
//***********************************************************
void RunWechat(char* wechatPath)
{
	// ΢������Ѿ����� ֱ��ע�� ���� ������ ע��
	if (InjectDll()) {
		return;
	}
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	CreateProcess(NULL, wechatPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	LPVOID Param = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	char add[0x100] = { 0 };

	char* dllPath = GetDllPath(INJECT_DLL_NAME);
	WriteProcessMemory(pi.hProcess, Param, dllPath, strlen(dllPath) * 2 + sizeof(char), NULL);

	char buff[0x100] = { 0 };
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