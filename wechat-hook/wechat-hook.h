#pragma once

#include "resource.h"

//------------------------------------����������-------------------------------------------------
INT_PTR CALLBACK Dlgproc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void RunSingle();
VOID handleWmCommand(HWND hwndDlg, WPARAM wParam);
//------------------------------------����������-------------------------------------------------