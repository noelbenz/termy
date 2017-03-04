
// C++ Standard
#include <exception>

// Windows
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

//------------------------------------------------------------------------------
// Windows types
//------------------------------------------------------------------------------

namespace Win {

    typedef BOOL         Bool;
    typedef UINT         UInt;
    typedef FLOAT        Float;
    typedef WCHAR        WChar;
    typedef LONG_PTR     LPointer;
    typedef WORD         Word;
    typedef DWORD        DWord;

    typedef HRESULT      Result;
    typedef LRESULT      LResult;
    typedef WPARAM       WParam;
    typedef LPARAM       LParam;

    typedef HINSTANCE    Instance;
    typedef HWND         Window;
    typedef CREATESTRUCT CreateStruct;
    typedef MSG          Msg;
    typedef RECT         Rect;

    inline Win::Bool success(Win::Result result) {
        return SUCCEEDED(result);
    }

    inline Win::Bool failed(Win::Result result) {
        return FAILED(result);
    }

    inline Win::Word lowWord(Win::DWord dword) {
        return LOWORD(dword);
    }

    inline Win::Word highWord(Win::DWord dword) {
        return HIWORD(dword);
    }

    const int gwlpUserdata = GWLP_USERDATA;
}

//------------------------------------------------------------------------------
// Direct 2D types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Direct Write types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Exception classes
//------------------------------------------------------------------------------

class UnhandledMessageError: public std::runtime_error {
private:

    Win::UInt msgType;

public:

    UnhandledMessageError(Win::UInt msgType)
        : std::runtime_error("unhandled window message")
        {
            this->msgType = msgType;
        }
};

//------------------------------------------------------------------------------
// Main class
//------------------------------------------------------------------------------

class Termy {
private:
    Win::Instance instance;
    Win::Window window;
    ID2D1Factory* factory;
    ID2D1HwndRenderTarget* renderTarget;
    ID2D1SolidColorBrush* brush;

    IDWriteFactory* writeFactory;
    IDWriteTextFormat* textFormat;

    const Win::WChar* text;
    Win::UInt textLength;

    void init();

    void createWindow();
    void createDeviceIndepdendentComponents();
    void createDeviceDependentComponents();
    void messageLoop();

    void onResize(int width, int height);
    void paint();

public:

    Termy(Win::Instance instance);

    void start();

    Win::LResult handleMessage(Win::UInt msgType, Win::WParam wParam,
                               Win::LParam lParam);

    void requestPaint();
};
