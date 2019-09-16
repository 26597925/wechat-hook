#include "stdafx.h"
#include "tools.h"
#include <time.h>

//�����ֽ�char*ת��Ϊ���ֽ�wchar_t*  
wchar_t* AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}

//�����ֽ�wchar_t*ת��Ϊ���ֽ�char*  
inline char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

//stringתUTF8
std::string string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//һ��Ҫ��1����Ȼ�����β�� 
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

//wchar_tתstring
std::string Wchar_tToString(wchar_t *wchar)
{
	std::string szDst;
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte������
	char *psText; // psTextΪchar*����ʱ���飬��Ϊ��ֵ��std::string���м����
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte���ٴ�����
	szDst = psText;// std::string��ֵ
	delete[]psText;// psText�����
	return szDst;
}


//stringתwchar_t
wchar_t * StringToWchar_t(const std::string & str)
{
	wchar_t * m_chatroommmsg = new wchar_t[str.size() * 2];  //
	memset(m_chatroommmsg, 0, str.size() * 2);
	setlocale(LC_ALL, "zh_CN.UTF-8");
	//setlocale(LC_CTYPE, "chs");
	swprintf(m_chatroommmsg, str.size() * 2, L"%S", str.c_str());

	return m_chatroommmsg;
}


void WriteLog(const std::string & level, const std::string & content) {
	setlocale(LC_ALL, "zh_CN.UTF-8");
	time_t t = time(0);
	struct tm nowTime;
	localtime_s(&nowTime, &t);
	char ch[64];
	strftime(ch, sizeof(ch), "%Y-%m-%d %H-%M-%S", &nowTime); //��-��-�� ʱ-��-��
	std::string times = ch;
	std::string log;
	log = string_To_UTF8(
		times + " " + level + " [rmation]\n" +
		content + "\n"
	);
	FILE *fp;
	errno_t err;  //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&fp, "log.txt", "ab+"); //��return 1 , ��ָ������ļ����ļ�����fp
	fwrite(log.c_str(), log.length(), 1, fp);
	fclose(fp);
}