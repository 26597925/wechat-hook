#pragma once

#include "resource.h"

//------------------------------------����������-------------------------------------------------
INT_PTR CALLBACK Dlgproc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void RunSingle();
void handleWmCommand(HWND hwndDlg, WPARAM wParam);
//------------------------------------����������-------------------------------------------------