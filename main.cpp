
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <vector>
#include <cstdio>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "dwmapi.lib")

// ==========================================
// RANK1 CMD - BYrank1shop
// Pure Win32 API + GDI
// ==========================================

const int WIN_W = 630;
const int WIN_H = 560;

const COLORREF C_BG    = RGB(6, 5, 8);
const COLORREF C_WHITE = RGB(215, 215, 220);
const COLORREF C_GREEN = RGB(40, 200, 75);
const COLORREF C_DIM   = RGB(75, 75, 88);
const COLORREF C_ORAN  = RGB(255, 80, 20);

HFONT g_fLogo = nullptr;
HFONT g_fMenu = nullptr;
HFONT g_fSmall = nullptr;
HFONT g_fTiny = nullptr;

enum ScreenState {
    S_MENU,
    S_RUN,
    S_DONE
};

ScreenState g_state = S_MENU;

HWND g_hwnd = nullptr;
bool g_blink = true;
int g_mode = 0;
int g_titleIndex = 0;
int g_runStep = 0;
int g_spinner = 0;

std::vector<std::wstring> g_logs;
std::vector<COLORREF> g_logColors;

// ==========================================
// TITLE ANIMATION
// ==========================================

const wchar_t* TITLES[] = {
    L"RANK1 | \u30E9\u30F3\u30AF1\u30B7\u30E7\u30C3\u30D7\u3088\u308A\u611B\u3092\u8FBC\u3081\u3066 \u2665",
    L"RANK1 | \u611B\u3057\u3066\u308B\u3001\u30E9\u30F3\u30AF1 \u2665",
    L"RANK1 | rank1shop \u304B\u3089\u3042\u306A\u305F\u3078 \u2665",
    L"RANK1 | \u6C38\u9060\u306B\u3042\u308A\u304C\u3068\u3046 \u2665 rank1shop",
    L"RANK1 | \u6700\u9AD8\u306E\u30C4\u30FC\u30EB\u3001\u6700\u9AD8\u306E\u611B \u2665"
};

// ==========================================
// LOGO
// ==========================================

const wchar_t* LOGO[] = {
    L"\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2584 \u2584\u2588\u2588\u2588\u2588\u2588\u2588\u2584 \u2588\u2588\u2588\u2584 \u2588\u2588\u2588\u2588 \u2588\u2588\u2588\u2588 \u2588\u2588\u2588\u2588 \u2588\u2588\u2588\u2588",
    L"\u2588\u2588\u2588\u2588 \u2580\u2588\u2588\u2588\u258C \u258C\u2593\u2588\u2593\u2588 \u2588\u2588\u2593\u258C \u2588\u2588\u2593\u2588\u2588\u2584\u2588\u2588\u2593\u2588 \u2588\u2588\u2593\u2588 \u2584\u2588\u2588\u2593\u2580 \u2588\u2588\u2593\u2588",
    L"\u2592\u2593\u2588\u2588\u2584\u2593\u2588\u2593\u2580 \u2592\u2593\u2588\u2593\u2593\u2580\u2592\u2593\u2588\u2593 \u2592\u2593\u2588\u2593 \u2592\u2593\u2588\u2593 \u2592\u2593\u2588\u2593\u2580\u2592\u2593\u2588\u2584 \u2592\u2593\u2588\u2593",
    L"\u2591\u2591\u2592\u2593\u2580\u2593\u2592\u2592\u2593\u2584 \u2591\u2591\u2592\u2593 \u2591\u2591\u2592\u2593 \u2591\u2591\u2592\u2593 \u2591\u2591\u2592\u2593 \u2591\u2591\u2592\u2593 \u2591\u2591\u2592\u2593",
    L" \u2591 \u2591\u2591\u2592\u2591\u258C \u2591 \u2591 \u2591 \u2591 \u2591 \u2591",
    L" \u2580 \u2580 \u2580\u2580\u2580\u2580 \u2580\u2580\u2580\u2580 \u2580\u2580\u2580\u2580 \u2580\u2580\u2580\u2580 \u2580\u2580\u2580\u2580 \u2580\u2580\u2580\u2580",
    L" \u2580 "
};

COLORREF LOGO_COLORS[] = {
    RGB(255,120,120),
    RGB(255,75,75),
    RGB(240,45,45),
    RGB(215,25,25),
    RGB(185,12,12),
    RGB(145,5,5),
    RGB(100,2,2)
};

// ==========================================
// GDI HELPERS
// ==========================================

void DrawTextCenter(
    HDC dc,
    const wchar_t* text,
    int y,
    HFONT font,
    COLORREF color,
    COLORREF shadow = RGB(0,0,0)
) {
    SetBkMode(dc, TRANSPARENT);

    HFONT oldFont = (HFONT)SelectObject(dc, font);

    SIZE sz{};
    GetTextExtentPoint32W(dc, text, (int)wcslen(text), &sz);

    int x = (WIN_W - sz.cx) / 2;

    SetTextColor(dc, shadow);
    TextOutW(dc, x + 1, y + 1, text, (int)wcslen(text));

    SetTextColor(dc, color);
    TextOutW(dc, x, y, text, (int)wcslen(text));

    SelectObject(dc, oldFont);
}

void DrawLine(HDC dc, int x1, int y1, int x2, int y2,
              COLORREF color, int width = 1) {
    HPEN pen = CreatePen(PS_SOLID, width, color);
    HPEN old = (HPEN)SelectObject(dc, pen);

    MoveToEx(dc, x1, y1, nullptr);
    LineTo(dc, x2, y2);

    SelectObject(dc, old);
    DeleteObject(pen);
}

void DrawRect(HDC dc, int x, int y, int w, int h, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT rc{ x, y, x + w, y + h };

    FillRect(dc, &rc, brush);
    DeleteObject(brush);
}

void AddLog(const std::wstring& text, COLORREF color) {
    g_logs.push_back(text);
    g_logColors.push_back(color);

    while (g_logs.size() > 14) {
        g_logs.erase(g_logs.begin());
        g_logColors.erase(g_logColors.begin());
    }

    InvalidateRect(g_hwnd, nullptr, FALSE);
}

// ==========================================
// FONT CREATION
// ==========================================

void CreateFonts() {
    g_fLogo = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier New"
    );

    g_fMenu = CreateFontW(
        25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier New"
    );

    g_fSmall = CreateFontW(
        19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier New"
    );

    g_fTiny = CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Courier New"
    );
}

void DeleteFonts() {
    if (g_fLogo) DeleteObject(g_fLogo);
    if (g_fMenu) DeleteObject(g_fMenu);
    if (g_fSmall) DeleteObject(g_fSmall);
    if (g_fTiny) DeleteObject(g_fTiny);
}

// ==========================================
// SCREEN DRAWING
// ==========================================

void DrawLogo(HDC dc) {
    SetBkMode(dc, TRANSPARENT);

    HFONT old = (HFONT)SelectObject(dc, g_fLogo);

    for (int i = 0; i < 7; i++) {
        SIZE sz{};
        GetTextExtentPoint32W(
            dc, LOGO[i], (int)wcslen(LOGO[i]), &sz
        );

        int x = (WIN_W - sz.cx) / 2;
        int y = 10 + i * 13;

        SetTextColor(dc, RGB(25, 0, 0));
        TextOutW(dc, x + 1, y + 1, LOGO[i], (int)wcslen(LOGO[i]));

        SetTextColor(dc, LOGO_COLORS[i]);
        TextOutW(dc, x, y, LOGO[i], (int)wcslen(LOGO[i]));
    }

    SelectObject(dc, old);
}

void DrawMenu(HDC dc) {
    DrawLogo(dc);

    DrawLine(dc, 30, 112, 600, 112, RGB(120,5,5), 1);
    DrawLine(dc, 30, 114, 600, 114, RGB(200,12,12), 2);
    DrawLine(dc, 30, 118, 600, 118, RGB(80,3,3), 1);

    DrawTextCenter(
        dc, L"\u30E9\u30F3\u30AF1 \u2665 RANK1SHOP \u2665 \u30E9\u30F3\u30AF1",
        128, g_fSmall, RGB(60,18,18)
    );

    DrawTextCenter(
        dc, L"\u2014\u2014 S E L E C T   M O D E \u2014\u2014",
        173, g_fSmall, RGB(140,22,22), RGB(40,3,3)
    );

    DrawLine(dc, 90, 207, 540, 207, RGB(40,8,8));

    DrawTextCenter(
        dc, L">> [ 1 ]  I N S T A L L  <<",
        232, g_fMenu, RGB(255,55,55), RGB(80,5,5)
    );

    DrawTextCenter(
        dc, L">> [ 2 ]  R E S E T  <<",
        287, g_fMenu, RGB(190,20,20), RGB(60,3,3)
    );

    DrawLine(dc, 90, 345, 540, 345, RGB(40,8,8));

    if (g_blink) {
        DrawTextCenter(
            dc, L">> \u2588",
            366, g_fMenu, RGB(220,30,30), RGB(60,3,3)
        );
    }

    DrawLine(dc, 0, 526, WIN_W, 526, RGB(50,8,8));
    DrawLine(dc, 0, 528, WIN_W, 528, RGB(25,3,3));

    DrawTextCenter(
        dc, L"\u2665 rank1shop.com \u2014 RANK1 CMD V.1 \u2665",
        535, g_fTiny, RGB(55,12,12)
    );
}

void DrawRunScreen(HDC dc) {
    const wchar_t* header =
        g_mode == 1
        ? L">> I N S T A L L   M O D E <<"
        : L">> R E S E T   M O D E <<";

    DrawTextCenter(dc, header, 35, g_fMenu,
                   RGB(220,35,35), RGB(60,0,0));

    DrawLine(dc, 35, 75, 595, 75, RGB(120,5,5), 1);
    DrawLine(dc, 35, 78, 595, 78, RGB(45,5,5), 1);

    SetBkMode(dc, TRANSPARENT);
    HFONT old = (HFONT)SelectObject(dc, g_fSmall);

    for (size_t i = 0; i < g_logs.size(); i++) {
        int y = 100 + (int)i * 25;

        SetTextColor(dc, g_logColors[i]);
        TextOutW(
            dc, 35, y,
            g_logs[i].c_str(),
            (int)g_logs[i].length()
        );
    }

    SelectObject(dc, old);

    if (g_state == S_RUN) {
        const wchar_t* spin[] = {
            L"-", L"\\", L"|", L"/"
        };

        wchar_t buffer[100];
        swprintf_s(
            buffer, 100,
            L"%s working...",
            spin[g_spinner % 4]
        );

        DrawTextCenter(
            dc, buffer, 462, g_fSmall,
            RGB(255,70,20), RGB(50,0,0)
        );
    }

    if (g_state == S_DONE) {
        DrawTextCenter(
            dc, L"press [R] to return / [ESC] to exit",
            462, g_fSmall, RGB(75,75,88)
        );
    }

    DrawLine(dc, 0, 526, WIN_W, 526, RGB(50,8,8));
    DrawLine(dc, 0, 528, WIN_W, 528, RGB(25,3,3));

    DrawTextCenter(
        dc, L"\u2665 rank1shop.com \u2014 RANK1 CMD V.1 \u2665",
        535, g_fTiny, RGB(55,12,12)
    );
}

// ==========================================
// DEMO WORKFLOW
// ==========================================

void StartDemo(int mode) {
    g_mode = mode;
    g_state = S_RUN;
    g_runStep = 0;
    g_spinner = 0;

    g_logs.clear();
    g_logColors.clear();

    AddLog(
        L">> opening demo session...",
        C_WHITE
    );

    SetTimer(g_hwnd, 3, 900, nullptr);
    InvalidateRect(g_hwnd, nullptr, FALSE);
}

void DemoStep() {
    g_spinner++;

    if (g_mode == 1) {
        switch (g_runStep) {
        case 0:
            AddLog(L"[OK] demo session acquired.", C_GREEN);
            break;

        case 1:
            AddLog(L">> Step 2: Preparing demo...", C_WHITE);
            break;

        case 2:
            AddLog(L"[OK] interface initialized.", C_GREEN);
            break;

        case 3:
            AddLog(L"[OK] configuration checked.", C_GREEN);
            break;

        case 4:
            AddLog(L">> Step 5: Finalizing...", C_WHITE);
            break;

        case 5:
            AddLog(
                L"*** INSTALL DEMO COMPLETE ***",
                C_ORAN
            );
            break;
        }
    }
    else {
        switch (g_runStep) {
        case 0:
            AddLog(L"[OK] demo session acquired.", C_GREEN);
            break;

        case 1:
            AddLog(L">> Step 2: Preparing reset demo...", C_WHITE);
            break;

        case 2:
            AddLog(L"[OK] demo settings checked.", C_GREEN);
            break;

        case 3:
            AddLog(L"[OK] interface restored.", C_GREEN);
            break;

        case 4:
            AddLog(L">> Step 5: Finalizing...", C_WHITE);
            break;

        case 5:
            AddLog(
                L"*** RESET DEMO COMPLETE ***",
                C_ORAN
            );
            break;
        }
    }

    g_runStep++;

    if (g_runStep >= 6) {
        KillTimer(g_hwnd, 3);
        g_state = S_DONE;
    }

    InvalidateRect(g_hwnd, nullptr, FALSE);
}

// ==========================================
// KEYBOARD
// ==========================================

void HandleKey(wchar_t key) {
    if (g_state == S_MENU) {
        if (key == L'1') {
            StartDemo(1);
        }
        else if (key == L'2') {
            StartDemo(2);
        }
        else if (key == 27) {
            PostQuitMessage(0);
        }
    }
    else if (g_state == S_DONE) {
        if (key == L'r' || key == L'R' || key == 13) {
            KillTimer(g_hwnd, 3);
            g_state = S_MENU;
            g_logs.clear();
            g_logColors.clear();
            InvalidateRect(g_hwnd, nullptr, FALSE);
        }
        else if (key == 27) {
            PostQuitMessage(0);
        }
    }
}

// ==========================================
// WINDOW PROCEDURE
// ==========================================

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (msg) {

    case WM_CREATE:
        SetLayeredWindowAttributes(
            hwnd, 0, 230, LWA_ALPHA
        );

        SetTimer(hwnd, 1, 2500, nullptr);
        SetTimer(hwnd, 2, 500, nullptr);

        {
            BOOL dark = TRUE;
            COLORREF black = RGB(0,0,0);

            DwmSetWindowAttribute(
                hwnd, 20, &dark, sizeof(dark)
            );

            DwmSetWindowAttribute(
                hwnd, 19, &dark, sizeof(dark)
            );

            DwmSetWindowAttribute(
                hwnd, 35, &black, sizeof(black)
            );
        }
        return 0;

    case WM_TIMER:
        if (wParam == 1) {
            g_titleIndex = (g_titleIndex + 1) % 5;
            SetWindowTextW(hwnd, TITLES[g_titleIndex]);
        }

        if (wParam == 2) {
            g_blink = !g_blink;
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (wParam == 3 && g_state == S_RUN) {
            DemoStep();
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            HandleKey(27);
        }
        else if (wParam == VK_RETURN) {
            HandleKey(13);
        }
        return 0;

    case WM_CHAR:
        HandleKey((wchar_t)wParam);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(
            hdc,
            rc.right,
            rc.bottom
        );

        HBITMAP oldBitmap =
            (HBITMAP)SelectObject(memDC, bitmap);

        HBRUSH bg = CreateSolidBrush(C_BG);
        FillRect(memDC, &rc, bg);
        DeleteObject(bg);

        if (g_state == S_MENU) {
            DrawMenu(memDC);
        }
        else {
            DrawRunScreen(memDC);
        }

        BitBlt(
            hdc, 0, 0,
            rc.right, rc.bottom,
            memDC, 0, 0, SRCCOPY
        );

        SelectObject(memDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        KillTimer(hwnd, 3);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ==========================================
// ENTRY POINT
// ==========================================

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow
) {
    CreateFonts();

    const wchar_t CLASS_NAME[] = L"RANK1_CMD_WINDOW";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    if (!RegisterClassW(&wc)) {
        DeleteFonts();
        return 1;
    }

    DWORD style =
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX;

    DWORD exStyle =
        WS_EX_APPWINDOW |
        WS_EX_LAYERED;

    RECT rect{ 0, 0, WIN_W, WIN_H };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    g_hwnd = CreateWindowExW(
        exStyle,
        CLASS_NAME,
        L"RANK1 CMD",
        style,
        x, y,
        width, height,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!g_hwnd) {
        DeleteFonts();
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg{};

    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteFonts();
    return (int)msg.wParam;
}
