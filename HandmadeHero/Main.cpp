#include <Windows.h>

static bool IsApplicationRunning;

static BITMAPINFO bitmapInfo;
static void *bitmapMemory;
static HBITMAP bitmapHandle;
static HDC bitmapDeviceContext;

static void Win32ResizeDIBSection(int width, int height)
{
    if (bitmapHandle)
    {
        DeleteObject(bitmapHandle);
    }
    if (!bitmapDeviceContext)
    {
		bitmapDeviceContext = CreateCompatibleDC(0);
    }

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;


    bitmapHandle = CreateDIBSection(bitmapDeviceContext, &bitmapInfo, DIB_RGB_COLORS, &bitmapMemory, 0, 0);
}

static void Win32UpdateWindow(HDC deviceContext, int x, int y, int w, int h)
{
    StretchDIBits(deviceContext, x, y, w, h, x, y, w, h, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message,WPARAM w_param,LPARAM l_param)
{
    LRESULT result = 0;
	switch (message)
	{
		case WM_SIZE:
			{
				RECT client_rect;
				GetClientRect(window, &client_rect);
                const int width = client_rect.right - client_rect.left;
				const int height = client_rect.bottom - client_rect.top;
				Win32ResizeDIBSection(width, height);
				OutputDebugStringA("WM_Size \n");
			}
			break;
		case WM_DESTROY:
            IsApplicationRunning = false;
            OutputDebugStringA("WM_Destroy \n");
            break;
		case WM_CLOSE:
            IsApplicationRunning = false;
            OutputDebugStringA("WM_Close \n");
            break;
		case WM_ACTIVATEAPP:
            OutputDebugStringA("WM_ActivateApp \n");
            break;
        case WM_PAINT:
	        {
		        PAINTSTRUCT paint;
        		HDC deviceContext = BeginPaint(window, &paint);

        		const int x = paint.rcPaint.left;
        		const int y = paint.rcPaint.top;
        		const int w = paint.rcPaint.right - paint.rcPaint.left;
        		const int h = paint.rcPaint.bottom - paint.rcPaint.top;

                Win32UpdateWindow(deviceContext, x, y, w, h);

        		EndPaint(window, &paint);

	        }
        	break;
		default:
            result = DefWindowProc(window, message, w_param, l_param);
            break;
	}
    return result;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance,PWSTR commandLine, int showCmd)
{
    WNDCLASS mainWindow = {};
    mainWindow.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    mainWindow.lpfnWndProc = Win32MainWindowCallback;
    mainWindow.hInstance = instance;
    // MainWindow.hIcon = ;
    mainWindow.lpszClassName = L"HandmadeHeroWindowClass";

    if (RegisterClass(&mainWindow) != NULL)
    {
        HWND WindowHandle = CreateWindowEx(
			0,
            mainWindow.lpszClassName,
			L"HandmadeHero",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instance,
            0
        );
        if (WindowHandle != NULL)
        {
			IsApplicationRunning = true;
            while (IsApplicationRunning)
            {
				MSG message;
                BOOL messageResult = GetMessage(&message, 0, 0, 0);
                if (messageResult > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                else
                {
                    OutputDebugString(L"Quit! \n");
                    break;
                }
            }
        }
        else
        {
            // TODO: Logging
			// GetLastError();
        }
    }
    else
    {
	    // TODO: Logging
        // GetLastError();
    }

    return 0;
}