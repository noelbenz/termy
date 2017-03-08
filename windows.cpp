
// C++ standard
#include <iostream>

// Windows libraries
#pragma comment(lib,"user32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// Termy
#include "windows.hpp"

using std::cout;
using std::endl;

//------------------------------------------------------------------------------
// Exception classes
//------------------------------------------------------------------------------

UnhandledMessageError::UnhandledMessageError(Win::UInt msgType)
    : std::runtime_error("unhandled window message")
{
    this->msgType = msgType;
}

WindowError::WindowError(const char *msg)
    : std::runtime_error(msg)
{
    this->window = NULL;
    this->errorCode = Win::lastError();
}

WindowError::WindowError(const char *msg, Win::Window window)
    : std::runtime_error(msg)
{
    this->window = window;
    this->errorCode = Win::lastError();
}

WindowError::WindowError(const char *msg, Win::Window window, Win::DWord errorCode)
    : std::runtime_error(msg)
{
    this->window = window;
    this->errorCode = errorCode;
}

//------------------------------------------------------------------------------
// Window process callback
//------------------------------------------------------------------------------

Win::LResult WINAPI
windowProc(Win::Window window, Win::UInt msgType,
           Win::WParam wParam, Win::LParam lParam) {

    if (msgType == WM_CREATE) {
        // lParam is a pointer to CreateStruct which holds the parameters
        // passed to CreateWindow.
        Win::CreateStruct *pcs = (Win::CreateStruct *)lParam;
        // Here we get the termy instance we passed to CreateWindow.
        Termy *termy = static_cast<Termy *>(pcs->lpCreateParams);
        // And set it as window user data for future access.
        SetWindowLongPtrW(
            window,
            Win::gwlpUserdata,
            PtrToUlong(termy));
    }

    // Retrieve our termy object from the window's user data.
    Termy *termy = reinterpret_cast<Termy *>(
                   static_cast<Win::LPointer>(
                   GetWindowLongPtrW(window, Win::gwlpUserdata)));

    Win::LResult ret;
    try {

        if (!termy)
            throw UnhandledMessageError(msgType);

        ret = termy->handleMessage(msgType, wParam, lParam);

    } catch (const UnhandledMessageError &ex) {
        ret = DefWindowProcW(window, msgType, wParam, lParam);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Main class
//------------------------------------------------------------------------------

void Termy::init() {
    this->instance = {};
    this->window = {};
    this->factory = NULL;
    this->renderTarget = NULL;
    this->brush = NULL;

    this->writeFactory = NULL;
    this->textFormat = NULL;

    this->text = L"Hello, world!";
    this->textLength = static_cast<Win::UInt>(wcslen(this->text));
}

Termy::Termy(Win::Instance instance) {
    this->init();

    this->instance = instance;

    cout << "Create device independent resources." << endl;
    this->createDeviceIndepdendentComponents();
    cout << "Create window." << endl;
    this->createWindow();
}


void Termy::createWindow() {
    Win::WindowClassEx windowClass = {};

    windowClass.cbSize = sizeof(Win::WindowClassEx);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &windowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = sizeof(LONG_PTR);
    windowClass.hInstance = this->instance;
    windowClass.lpszClassName = L"TermyMain";

    if (!RegisterClassExW(&windowClass)) {
        throw WindowError("Failed to register window class.");
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

    if (!this->window)
        throw WindowError("Unable to create window.");

    ShowWindow(this->window, SW_SHOWDEFAULT);
    UpdateWindow(this->window);
}

void Termy::createDeviceIndepdendentComponents() {
    Win::Result result;

    result = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &this->factory);

    if (Win::failed(result)) throw WindowError("Failed to create Direct 2D factory.", this->window);

    result = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&this->writeFactory));

    if (Win::failed(result)) throw WindowError("Failed to create Direct Write factory.", this->window);

    result = this->writeFactory->CreateTextFormat(
        L"Gabriola",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        72.0f,
        L"en-us",
        &this->textFormat);

    if (Win::failed(result)) throw WindowError("Failed to create text format.", this->window);
}

void Termy::createDeviceDependentComponents() {
    Win::Rect rect;
    Win::Result result;
    D2D1_SIZE_U size;

    GetClientRect(this->window, &rect);

    size = D2D1::SizeU(
        rect.right - rect.left,
        rect.bottom - rect.top);

    result = this->factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(this->window, size),
        &this->renderTarget);

    if (Win::failed(result)) throw WindowError("Failed to create render target.", this->window);

    result = this->renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightSlateGray),
        &this->brush);

    if (Win::failed(result)) throw WindowError("Failed to create brush.", this->window);

    result = this->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    if (Win::failed(result)) throw WindowError("Failed to set font alignment.", this->window);

    result = this->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    if (Win::failed(result)) throw WindowError("Failed to set font paragraph alignment.", this->window);
}

void Termy::messageLoop() {
    Win::Msg msg;
    Win::Bool status;

    while(1) {
        status = GetMessageW(&msg, NULL, 0, 0);
        if (status == 0) break;

        if (status == -1)
            throw WindowError("GetMessage failed.", this->window);

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Termy::requestPaint() {
    InvalidateRect(this->window, NULL, FALSE);
}

void Termy::onResize(int width, int height) {
    if (!this->renderTarget) return;

    this->renderTarget->Resize(D2D1::SizeU(width, height));
}

void Termy::paint() {
    Win::Rect rect;
    GetClientRect(this->window, &rect);

    D2D1_RECT_F layoutRect = D2D1::RectF(
        static_cast<Win::Float>(rect.left),
        static_cast<Win::Float>(rect.top),
        static_cast<Win::Float>(rect.right - rect.left),
        static_cast<Win::Float>(rect.bottom - rect.top));


    Win::PaintStruct ps;
    BeginPaint(this->window, &ps);
    this->renderTarget->BeginDraw();
    this->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    this->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    this->renderTarget->DrawLine(
        D2D1::Point2F(0.0f, 0.0f),
        D2D1::Point2F(50.0f, 50.0f),
        this->brush,
        1.0f);
    this->renderTarget->DrawText(
        this->text,
        this->textLength,
        this->textFormat,
        layoutRect,
        this->brush);
    this->renderTarget->EndDraw();
    ValidateRect(this->window, NULL);
    EndPaint(this->window, &ps);
}

Win::LResult Termy::handleMessage(Win::UInt msgType, Win::WParam wParam,
                                  Win::LParam lParam) {

    switch (msgType) {

        case WM_DISPLAYCHANGE:
        {
            this->requestPaint();
            return 1;
        }

        case WM_SIZE:
        {
            unsigned int width;
            unsigned int height;
            width = static_cast<unsigned int>(Win::lowWord(lParam));
            height = static_cast<unsigned int>(Win::highWord(lParam));

            this->onResize(width, height);
            return 1;
        }

        case WM_PAINT:
        {
            if (!this->renderTarget)
                this->createDeviceDependentComponents();

            this->paint();
            return 1;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 1;
        }

        default:
        {
            throw UnhandledMessageError(msgType);
        }

    }
}

void Termy::start() {
    cout << "Entering message loop." << endl;
    this->messageLoop();
}
