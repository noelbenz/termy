
// C standard
#include <iostream>

// Windows
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

// Windows libraries
#pragma comment(lib,"user32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

using std::cout;
using std::endl;

struct Termy {
    HINSTANCE instance;
    HWND window;
    ID2D1Factory* factory;
    ID2D1HwndRenderTarget* renderTarget;
    ID2D1SolidColorBrush* brush;

    IDWriteFactory* writeFactory;
    IDWriteTextFormat* textFormat;

    const wchar_t* text;
    UINT32 textLength;
};

void setupD2D1(Termy *termy) {
    RECT rect;
    D2D1_SIZE_U size;
    HRESULT result;

    termy->text = L"Hello, world!";
    termy->textLength = (UINT32) wcslen(termy->text);

    GetClientRect(termy->window, &rect);

    size = D2D1::SizeU(
        rect.right - rect.left,
        rect.bottom - rect.top);

    result = termy->factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(termy->window, size),
        &termy->renderTarget);

    if (result != S_OK) throw "Failed to create render target.";

    result = termy->renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightSlateGray),
        &termy->brush);

    if (result != S_OK) throw "Failed to create brush.";

    result = termy->writeFactory->CreateTextFormat(
        L"Gabriola",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        72.f,
        L"en-us",
        &termy->textFormat);

    if (result != S_OK) throw "Failed to create text format.";

    result = termy->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    if (result != S_OK) throw "Failed to set font alignment.";

    result = termy->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    if (result != S_OK) throw "Failed to set font paragraph alignment.";
}

LRESULT WINAPI windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {

    if (msg == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Termy *termy = (Termy *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            window,
            GWLP_USERDATA,
            PtrToUlong(termy));

        return 1;
    }

    Termy *termy = reinterpret_cast<Termy *>(
                   static_cast<LONG_PTR>(
                   ::GetWindowLongPtrW(
                       window,
                       GWLP_USERDATA)));

    switch (msg) {
    case WM_DISPLAYCHANGE:
    {
        InvalidateRect(window, NULL, FALSE);
        return 1;
    }
    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);

        if (termy->renderTarget)
            termy->renderTarget->Resize(D2D1::SizeU(width, height));

        return 1;
    }
    case WM_PAINT:
    {
        if (!termy->renderTarget)
            setupD2D1(termy);

        RECT rect;
        GetClientRect(window, &rect);

        D2D1_RECT_F layoutRect = D2D1::RectF(
            static_cast<FLOAT>(rect.left),
            static_cast<FLOAT>(rect.top),
            static_cast<FLOAT>(rect.right - rect.left),
            static_cast<FLOAT>(rect.bottom - rect.top));


        PAINTSTRUCT ps;
        BeginPaint(window, &ps);
        termy->renderTarget->BeginDraw();
        termy->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        termy->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
        termy->renderTarget->DrawLine(
            D2D1::Point2F(0.0f, 0.0f),
            D2D1::Point2F(50.0f, 50.0f),
            termy->brush,
            1.0f);
        termy->renderTarget->DrawText(
            termy->text,
            termy->textLength,
            termy->textFormat,
            layoutRect,
            termy->brush);
        termy->renderTarget->EndDraw();
        ValidateRect(window, NULL);
        EndPaint(window, &ps);
        return 1;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 1;
    }
    }

    return DefWindowProcW(window, msg, wParam, lParam);
}

void createD2D1Factory(Termy *termy) {
    HRESULT result;

    result = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &termy->factory);

    if (result != S_OK) throw "Failed to create Direct 2D factory.";

    result = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&termy->writeFactory));

    if (result != S_OK) throw "Failed to create Direct Write factory.";
}

void createWindow(Termy *termy) {
    WNDCLASSEXW wincls = {sizeof(WNDCLASSEXW)};

    wincls.style = CS_HREDRAW | CS_VREDRAW;
    wincls.lpfnWndProc = &windowProc;
    wincls.cbClsExtra = 0;
    wincls.cbWndExtra = sizeof(LONG_PTR);
    wincls.hInstance = termy->instance;
    wincls.lpszClassName = L"TermyMain";

    if (!RegisterClassExW(&wincls)) {
        // TODO: Get more error detals.
        throw "Failed to register window class.";
    }

    termy->window = CreateWindowExW(
        0L,
        L"TermyMain",
        L"Termy",
        WS_OVERLAPPEDWINDOW,
        0, 0,
        500, 300,
        NULL,
        NULL,
        termy->instance,
        termy);

    // TODO: Get more error detals.
    if (!termy->window) throw "Unable to create window.";

    ShowWindow(termy->window, SW_SHOWDEFAULT);
    UpdateWindow(termy->window);
}

void messageLoop(Termy *termy) {
    MSG msg;
    BOOL status;

    while(1) {
        status = GetMessageW(&msg, NULL, 0, 0);
        if (status == 0) break;

        if (status == -1)
            // TODO: Get more error detals.
            throw "Messsage loop failure.";

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

int main() {

    cout << "Hello windows!" << endl;

    Termy termy = {};
    termy.instance = GetModuleHandleW(NULL);

    try {
        cout << "Create D2D1 factory." << endl;
        createD2D1Factory(&termy);
        cout << "Create window." << endl;
        createWindow(&termy);
        cout << "Entering message loop." << endl;
        messageLoop(&termy);
    } catch (char* msg) {
        cout << msg << endl;
        return -1;
    }

    return 0;
}
