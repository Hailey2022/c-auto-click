#include <Windows.h>
#include <stdio.h>

#define ID_BtnSel 3301
#define ID_FuncSel 3302
#define ID_TLSet 3303
#define ID_HKSet 3304
#define ID_UorELock 3305
#define HotKey 1011

static HWND hFont;
static HWND hTip1;
static HWND hRadio_Left;
static HWND hRadio_Right;
static HWND hTip2;
static HWND hCombo_Func;
static HWND hTip3;
static HWND hText_TL;
static HWND hTip4;
static HWND hCombo_HK;
static HWND hBtn_UorE;

INT LeftOrRight = 1;
INT Func = 0;
INT dig_TL = 500;
INT HK_Index = 0;
BOOL CfgLocked = FALSE;
TCHAR str_TL[20] = {0};
BOOL IsRunning = FALSE;
HANDLE hClickThread = NULL;

const TCHAR str_HKList[12][4] = {
        TEXT("F1"), TEXT("F2"), TEXT("F3"), TEXT("F4"),
        TEXT("F5"), TEXT("F6"), TEXT("F7"), TEXT("F8"),
        TEXT("F9"), TEXT("F10"), TEXT("F11"), TEXT("F12")};

INT UnLock_NoGUI(HWND this_hwnd) {
    if (IsRunning)
        TerminateThread(hClickThread, 0);
    UnregisterHotKey(this_hwnd, HotKey);
    CfgLocked = FALSE;
    return 0;
}

INT FlashConfig() {
    HKEY hKey = NULL;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(INT);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\AutoClick"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) ==
        ERROR_SUCCESS) {

        RegQueryValueEx(hKey, TEXT("LeftOrRight"), NULL, &dwType, (LPBYTE) (&LeftOrRight), &dwSize);
        RegQueryValueEx(hKey, TEXT("Func"), NULL, &dwType, (LPBYTE) (&Func), &dwSize);
        RegQueryValueEx(hKey, TEXT("TL"), NULL, &dwType, (LPBYTE) (&dig_TL), &dwSize);
        RegQueryValueEx(hKey, TEXT("HK"), NULL, &dwType, (LPBYTE) (&HK_Index), &dwSize);
    }
    RegCloseKey(hKey);
    if (LeftOrRight) {
        SendMessage(hRadio_Left, BM_SETCHECK, 1, 0);
    } else {
        SendMessage(hRadio_Right, BM_SETCHECK, 1, 0);
    }
    SendMessage(hCombo_Func, CB_SETCURSEL, Func, 0);
    SendMessage(hText_TL, WM_SETTEXT, (WPARAM) NULL, (LPARAM) _itow(dig_TL, (wchar_t *) str_TL, 10));
    SendMessage(hCombo_HK, CB_SETCURSEL, HK_Index, 0);

    return 0;
}

INT SaveConfig() {
    HKEY hKey = NULL;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(INT);
    DWORD dwDisposition = 0;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\AutoClick"), 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, TEXT("LeftOrRight"), 0, dwType, (LPCBYTE) (&LeftOrRight), dwSize);
        RegSetValueEx(hKey, TEXT("Func"), 0, dwType, (LPCBYTE) (&Func), dwSize);
        RegSetValueEx(hKey, TEXT("TL"), 0, dwType, (LPCBYTE) (&dig_TL), dwSize);
        RegSetValueEx(hKey, TEXT("HK"), 0, dwType, (LPCBYTE) (&HK_Index), dwSize);
        RegCloseKey(hKey);
        return 0;
    } else {
        return 1;
    }
}

DWORD WINAPI ClickRunner(LPVOID lpParam) {
    INPUT Down = {0}, Up = {0};
    if (Func == 1) {
        if (LeftOrRight) {
            while (TRUE) {
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                Sleep(dig_TL);
            }
        } else {
            while (TRUE) {
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                Sleep(dig_TL);
            }
        }
    } else {
        Down.type = INPUT_MOUSE;
        Up.type = INPUT_MOUSE;
        if (LeftOrRight) {
            Down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            Up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        } else {
            Down.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            Up.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        }
        while (TRUE) {
            SendInput(TRUE, &Down, sizeof(INPUT));
            SendInput(TRUE, &Up, sizeof(INPUT));
            Sleep(dig_TL);
        }
    }
}

INT IsPosDigitStr(LPTSTR in_str) {
    INT ret = 0;
    INT i;
    INT len = lstrlen(in_str);
    if (!len)
        return 0;
    for (i = 0; i < len; i++) {
        if (!isdigit(in_str[i]))
            return 0;
        else {
            ret *= 10;
            ret += in_str[i] - '0';
        }
    }
    return ret;
}

LRESULT WINAPI CtlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int i;

    switch (message) {
        case WM_CREATE:
            hTip1 = CreateWindow(TEXT("Static"), TEXT("Mouse:"), WS_CHILD | WS_VISIBLE, 15, 10, 300, 100, hWnd, NULL,
                                 (HINSTANCE) hWnd, 0);
            hRadio_Left = CreateWindow(TEXT("Button"), TEXT("left"),
                                       WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 60, 35, 80, 20, hWnd,
                                       (HMENU) ID_BtnSel, (HINSTANCE) hWnd, 0);
            hRadio_Right = CreateWindow(TEXT("Button"), TEXT("right"),
                                        WS_CHILD | WS_VISIBLE | BS_LEFT | BS_AUTORADIOBUTTON, 200, 35, 80, 20, hWnd,
                                        (HMENU) ID_BtnSel, (HINSTANCE) hWnd, 0);
            hTip2 = CreateWindow(TEXT("Static"), TEXT("Method:"), WS_CHILD | WS_VISIBLE, 15, 60, 300, 100, hWnd, NULL,
                                 (HINSTANCE) hWnd, 0);
            hCombo_Func = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 40, 85,
                                       280, 100, hWnd, (HMENU) ID_FuncSel, (HINSTANCE) hWnd, 0);
            hTip3 = CreateWindow(TEXT("Static"), TEXT("Interval (ms):"), WS_CHILD | WS_VISIBLE, 15, 120, 300, 100, hWnd,
                                 NULL, (HINSTANCE) hWnd, 0);
            hText_TL = CreateWindow(TEXT("Edit"), TEXT("500"), ES_CENTER | WS_CHILD | WS_VISIBLE, 30, 145, 80, 25, hWnd,
                                    (HMENU) ID_TLSet, (HINSTANCE) hWnd, 0);
            hTip4 = CreateWindow(TEXT("Static"), TEXT("Hotkey:"), WS_CHILD | WS_VISIBLE, 210, 120, 300, 100, hWnd, NULL,
                                 (HINSTANCE) hWnd, 0);
            hCombo_HK = CreateWindow(TEXT("ComboBox"), TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 225, 145, 80,
                                     1000, hWnd, (HMENU) ID_HKSet, (HINSTANCE) hWnd, 0);
            hBtn_UorE = CreateWindow(TEXT("Button"), TEXT("Lock"), ES_CENTER | WS_CHILD | WS_VISIBLE, 25, 180, 290, 50,
                                     hWnd, (HMENU) ID_UorELock, (HINSTANCE) hWnd, 0);

            SendMessage(hTip1, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hRadio_Left, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hRadio_Right, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hTip2, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hTip3, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hTip4, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hText_TL, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hBtn_UorE, WM_SETFONT, (WPARAM) hFont, (LPARAM) NULL);
            SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT((LPARAM) "mouse_event"));
            SendMessage(hCombo_Func, CB_ADDSTRING, 0, TEXT((LPARAM) "SendInput"));

            for (i = 0; i < 12; i++) {
                SendMessage(hCombo_HK, CB_ADDSTRING, 0, (LPARAM) str_HKList[i]);
            }
            FlashConfig();
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_UorELock:
                    if (CfgLocked) {
                        UnLock_NoGUI(hWnd);
                        EnableWindow(hRadio_Left, TRUE);
                        EnableWindow(hRadio_Right, TRUE);
                        EnableWindow(hCombo_Func, TRUE);
                        EnableWindow(hText_TL, TRUE);
                        EnableWindow(hCombo_HK, TRUE);
                        EnableWindow(hRadio_Left, TRUE);
                        SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT((LPARAM) "Lock"));
                    } else {
                        if (SendMessage(hRadio_Left, BM_GETCHECK, 0, 0) == BST_CHECKED)
                            LeftOrRight = 1;
                        else if (SendMessage(hRadio_Right, BM_GETCHECK, 0, 0) == BST_CHECKED)
                            LeftOrRight = 0;
                        else {
                            MessageBox(hWnd, TEXT("Please choose left or right"), TEXT("Error"), MB_ICONERROR);
                            break;
                        }
                        if ((Func = SendMessage(hCombo_Func, CB_GETCURSEL, 0, 0)) == -1) {
                            MessageBox(hWnd, TEXT("Invalid method"), TEXT("Error"), MB_ICONERROR);
                            break;
                        }
                        GetWindowText(hText_TL, str_TL, 19);
                        if ((dig_TL = IsPosDigitStr(str_TL)) == 0) {
                            MessageBox(hWnd, TEXT("Invalid interval"), TEXT("Error"), MB_ICONERROR);
                            break;
                        }
                        if ((HK_Index = SendMessage(hCombo_HK, CB_GETCURSEL, 0, 0)) == -1) {
                            MessageBox(hWnd, TEXT("Invalid hotkey"), TEXT("Error"), MB_ICONERROR);
                            break;
                        }
                        if (!RegisterHotKey(hWnd, HotKey, 0, VK_F1 + HK_Index)) {
                            MessageBox(hWnd, TEXT("Try another hotkey"), TEXT("Error"), MB_ICONERROR);
                            break;
                        }
                        EnableWindow(hRadio_Left, FALSE);
                        EnableWindow(hRadio_Right, FALSE);
                        EnableWindow(hCombo_Func, FALSE);
                        EnableWindow(hText_TL, FALSE);
                        EnableWindow(hCombo_HK, FALSE);
                        char tips[32] = "";
                        sprintf(tips, "Unlock (Press F%d to run or stop)", HK_Index + 1);
                        SendMessage(hBtn_UorE, WM_SETTEXT, 0, TEXT((LPARAM) tips));
                        CfgLocked = TRUE;
                        SaveConfig();
                    }
                    break;
                default:
                    break;
            }
            break;
        case WM_DESTROY:
            if (CfgLocked)
                UnLock_NoGUI(hWnd);
            DeleteObject(hFont);
            PostQuitMessage(0);
            break;
        case WM_HOTKEY:
            if (IsRunning) {
                TerminateThread(hClickThread, 0);
                IsRunning = FALSE;
            } else {
                hClickThread = CreateThread(NULL, 0, ClickRunner, NULL, 0, NULL);
                IsRunning = TRUE;
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    WNDCLASSEX WC;
    HWND hwnd;
    INT width = GetSystemMetrics(SM_CXSCREEN);
    INT height = GetSystemMetrics(SM_CYSCREEN);

    WC.cbSize = sizeof(WNDCLASSEX);
    WC.style = CS_HREDRAW | CS_VREDRAW;
    WC.lpfnWndProc = CtlProc;
    WC.cbClsExtra = 0;
    WC.cbWndExtra = 0;
    WC.hInstance = hInstance;
    WC.hIcon = 0;
    WC.hCursor = 0;
    WC.hbrBackground = (HBRUSH) GetSysColorBrush(COLOR_BTNFACE);
    WC.lpszMenuName = 0;
    WC.lpszClassName = TEXT("WND");
    WC.hIconSm = 0;

    RegisterClassEx(&WC);
    hwnd = CreateWindow(TEXT("WND"), TEXT("Auto Click"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                        width / 2 - 178, height / 2 - 155, 356, 280, NULL, 0, 0, 0);
    ShowWindow(hwnd, 1);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}