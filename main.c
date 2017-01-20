
// C standard
#include <stdio.h>

// Windows
#include <windows.h>
#include <d2d1.h>

// Windows libraries
#pragma comment(lib,"user32.lib")
#pragma comment(lib, "d2d1.lib")

LRESULT WINAPI windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT:
        return TRUE;
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

int main() {

    printf("Hello, windows!\n");

    HINSTANCE hInstance = GetModuleHandleW(NULL);

    WNDCLASSW wincls;
    wincls.style = 0L;
    wincls.lpfnWndProc = &windowProc;
    wincls.cbClsExtra = 0;
    wincls.cbWndExtra = 0;
    wincls.hInstance = hInstance;
    wincls.hIcon = NULL;
    wincls.hCursor = NULL;
    wincls.hbrBackground = GetSysColorBrush(GetSysColor(COLOR_WINDOW));
    wincls.lpszMenuName = NULL;
    wincls.lpszClassName = L"termy_main";

    if (!RegisterClassW(&wincls)) {
        printf("Failed: %s\n", "TODO");
        return 1;
    }

    HWND window = CreateWindowExW(0L,
                                  L"termy_main",
                                  L"Termy",
                                  WS_OVERLAPPEDWINDOW,
                                  0, 0,
                                  500, 300,
                                  NULL,
                                  NULL,
                                  hInstance,
                                  NULL);

    if (!window) {
        printf("Failed %s\n", "TODO");
        return 2;
    }

    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);

    MSG msg;
    BOOL bRet;
    while(1) {
        bRet = GetMessageW(&msg, NULL, 0, 0);
        if (bRet == 0) break;

        if (bRet == -1) {
            printf("Failed %s\n", "TODO");
            return 3;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
