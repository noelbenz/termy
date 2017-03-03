
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

//------------------------------------------------------------------------------
// Windows types
//------------------------------------------------------------------------------

typedef BOOL      WinBool;
typedef UINT      WinUInt;
typedef WCHAR     WinWChar;

typedef HRESULT   WinResult;
typedef MSG       WinMsg;
typedef LRESULT   WinLResult;
typedef WPARAM    WinWParam;
typedef LPARAM    WinLParam;

typedef HINSTANCE WinInstance;
typedef HWND      WinWindow;
typedef RECT      WinRect;

inline int winSuccess(WinResult result) {
    return SUCCEEDED(result);
}
inline int winFailed(WinResult result) {
    return FAILED(result);
}

//------------------------------------------------------------------------------
// Direct 2D types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Direct Write types
//------------------------------------------------------------------------------

class Termy {
public:
    WinInstance instance;
    WinWindow window;
    ID2D1Factory* factory;
    ID2D1HwndRenderTarget* renderTarget;
    ID2D1SolidColorBrush* brush;

    IDWriteFactory* writeFactory;
    IDWriteTextFormat* textFormat;

    const WinWChar* text;
    WinUInt textLength;

    void createWindow();
    void createDeviceIndepdendentComponents();
    void createDeviceDependentComponents();
    void messageLoop();

    void init();

public:

    Termy(WinInstance);

    void start();
};

WinLResult WINAPI
windowProc(WinWindow window, WinUInt msg, WinWParam wParam, WinLParam lParam) {

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
        WinUInt width = LOWORD(lParam);
        WinUInt height = HIWORD(lParam);

        if (termy->renderTarget)
            termy->renderTarget->Resize(D2D1::SizeU(width, height));

        return 1;
    }
    case WM_PAINT:
    {
        if (!termy->renderTarget)
            termy->createDeviceDependentComponents();

        WinRect rect;
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

void Termy::init() {
    this->instance = {};
    this->window = {};
    this->factory = NULL;
    this->renderTarget = NULL;
    this->brush = NULL;

    this->writeFactory = NULL;
    this->textFormat = NULL;

    this->text = L"Hello, world!";
    this->textLength = static_cast<WinUInt>(wcslen(this->text));
}

Termy::Termy(WinInstance instance) {
    this->init();

    this->instance = instance;

    cout << "Create device independent resources." << endl;
    this->createDeviceIndepdendentComponents();
    cout << "Create window." << endl;
    this->createWindow();
}


void Termy::createWindow() {
    WNDCLASSEXW wincls = {sizeof(WNDCLASSEXW)};

    wincls.style = CS_HREDRAW | CS_VREDRAW;
    wincls.lpfnWndProc = &windowProc;
    wincls.cbClsExtra = 0;
    wincls.cbWndExtra = sizeof(LONG_PTR);
    wincls.hInstance = this->instance;
    wincls.lpszClassName = L"TermyMain";

    if (!RegisterClassExW(&wincls)) {
        // TODO: Get more error details.
        throw "Failed to register window class.";
    }

    this->window = CreateWindowExW(
        0L,
        L"TermyMain",
        L"Termy",
        WS_OVERLAPPEDWINDOW,
        0, 0,
        500, 300,
        NULL,
        NULL,
        this->instance,
        this);

    // TODO: Get more error details.
    if (!this->window) throw "Unable to create window.";

    ShowWindow(this->window, SW_SHOWDEFAULT);
    UpdateWindow(this->window);
}

void Termy::createDeviceIndepdendentComponents() {
    WinResult result;

    result = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &this->factory);

    if (winFailed(result)) throw "Failed to create Direct 2D factory.";

    result = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&this->writeFactory));

    if (winFailed(result)) throw "Failed to create Direct Write factory.";
}

void Termy::createDeviceDependentComponents() {
    WinRect rect;
    D2D1_SIZE_U size;
    WinResult result;

    GetClientRect(this->window, &rect);

    size = D2D1::SizeU(
        rect.right - rect.left,
        rect.bottom - rect.top);

    result = this->factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(this->window, size),
        &this->renderTarget);

    if (winFailed(result)) throw "Failed to create render target.";

    result = this->renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightSlateGray),
        &this->brush);

    if (winFailed(result)) throw "Failed to create brush.";

    result = this->writeFactory->CreateTextFormat(
        L"Gabriola",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        72.f,
        L"en-us",
        &this->textFormat);

    if (winFailed(result)) throw "Failed to create text format.";

    result = this->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    if (winFailed(result)) throw "Failed to set font alignment.";

    result = this->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    if (winFailed(result)) throw "Failed to set font paragraph alignment.";
}

void Termy::messageLoop() {
    WinMsg msg;
    WinBool status;

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

void Termy::start() {
    cout << "Entering message loop." << endl;
    this->messageLoop();
}

int main() {

    cout << "Hello windows!" << endl;

    try {
        Termy *termy = new Termy(GetModuleHandleW(NULL));
        termy->start();
    } catch (char* msg) {
        cout << msg << endl;
        return -1;
    }

    return 0;
}
