#include <Windows.h>
#include <cstdint>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

static bool IsApplicationRunning;

static BITMAPINFO bitmapInfo;
static void *bitmapMemory;
static int bitmapWidth;
static int bitmapHeight;

static int bytesPerPixel = 4;

static void Win32RenderBitmap(int xOffset, int yOffset)
{
	int width = bitmapWidth;
	int height = bitmapHeight;

    int pitch = width * bytesPerPixel;
    uint8* row = (uint8*)bitmapMemory;
    for (int y = 0; y < bitmapHeight; y++)
    {
        uint32* pixel = (uint32*)row;
        for (int x = 0; x < bitmapWidth; x++)
        {
            uint8 blue = (x + xOffset);
            uint8 green = (y + yOffset);
            *pixel++ = (uint32)((green << 8) | blue);
        }

        row += pitch;
    }
}

static void Win32ResizeDIBSection(int width, int height)
{
    if (bitmapMemory)
    {
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

	bitmapWidth = width;
	bitmapHeight = height;

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = (width * height)*bytesPerPixel;
    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

static void Win32UpdateWindow(HDC deviceContext, RECT *clientRect, int x, int y, int w, int h)
{
	int windowWidth = clientRect->right - clientRect->left;
	int windowsHeight = clientRect->bottom - clientRect->top;
    StretchDIBits(deviceContext,
		// Destination
        0, 0, windowWidth, windowsHeight,
        // Source
        0, 0, bitmapWidth, bitmapHeight,
        bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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

                RECT client_rect;
                GetClientRect(window, &client_rect);

                Win32UpdateWindow(deviceContext,  &client_rect, x, y, w, h);

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
            int xOffset = 0;
            int yOffset = 0;
            while (IsApplicationRunning)
            {
				MSG message;
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
	                    IsApplicationRunning = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                Win32RenderBitmap(xOffset, 0);
                HDC deviceContext = GetDC(WindowHandle);
                RECT clientRect;
                GetClientRect(WindowHandle, &clientRect);

				int windowWidth = clientRect.right - clientRect.left;
				int windowsHeight = clientRect.bottom - clientRect.top;

                Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowsHeight);
                ReleaseDC(WindowHandle, deviceContext);
				xOffset++;
				xOffset = xOffset % 255;
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