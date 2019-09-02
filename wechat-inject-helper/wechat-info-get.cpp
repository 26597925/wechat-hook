#include "stdafx.h"
#include "utils.h"
#include "wechat-info-get.h"

/*
WechatWin.dll ��ַ
�ǳ� + 0x126D91C
wxid + 0x126D8A4
΢���˺� + 0x
�ֻ����� + 0x126D950
��¼�豸 + 0x126DD48
ͷ���ַ + 0x126DBFC
*/

wxMyUserInfo getUserInfo()
{
	// �õ�ģ���ַ
	DWORD WechatWin = getWechatWinAdd();
	// ȡ����
	wxMyUserInfo userInfo = { 0 };
	userInfo.wxid = UTF8ToUnicode((const char *)WechatWin + 0x126D8A4);
	if (wcslen(userInfo.wxid) < 0x6) {
		DWORD pWxid = WechatWin + 0x126D8A4;
		pWxid = *((DWORD *)pWxid);
		userInfo.wxid = UTF8ToUnicode((const char *)pWxid);
	}
	userInfo.wxUsername = UTF8ToUnicode((const char *)WechatWin + 0x126D950);
	userInfo.wxNick = UTF8ToUnicode((const char *)WechatWin + 0x126D91C);
	userInfo.wxBindPhone = UTF8ToUnicode((const char *)WechatWin + 0x126D950);
	userInfo.wxDevice = UTF8ToUnicode((const char *)WechatWin + 0x126DD48);
	DWORD pHeadPic = WechatWin + 0x126DBFC;
	pHeadPic = *((DWORD *)pHeadPic);
	userInfo.wxHeadPic = UTF8ToUnicode((const char *)pHeadPic);
	return userInfo;
}