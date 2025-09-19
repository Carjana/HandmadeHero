#include <Windows.h>
#include <cstdint>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

static bool is_application_running;

struct win32_window_dimensions
{
    int width;
    int height;
};

struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

static win32_offscreen_buffer global_back_buffer;

win32_window_dimensions GetWindowDimension(HWND window)
{
	win32_window_dimensions result;
	RECT clientRect;
    GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

    return result;
}

static uint32 MakeRGB(uint8 r, uint8 g, uint8 b)
{
    // NOTE: Little endian
    // XX RR GG BB
	return r << 16 | g << 8 | b;
}

static void Win32RenderWeirdGradient(const win32_offscreen_buffer& buffer, int xOffset, int yOffset)
{
    uint8* row = (uint8*)buffer.memory;
    for (int y = 0; y < buffer.height; y++)
    {
        uint32* pixel = (uint32*)row;
        for (int x = 0; x < buffer.width; x++)
        {
            uint8 red = 0;
            uint8 green = 0;
            uint8 blue = 0;

			const int actualY = y + yOffset;
            const int actualX = x + xOffset;

			red = (255-actualX);
        	green = (actualX);
            blue = (255 - actualX) + (actualX);

            *pixel = MakeRGB(red, green, blue);
			pixel++;
        }

        row += buffer.pitch;
    }
}

static void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

	buffer->width = width;
	buffer->height = height;
    buffer->bytes_per_pixel = 4;
          
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height;
    buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = width * height*buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	buffer->pitch = width * buffer->bytes_per_pixel;
}

static void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight, const win32_offscreen_buffer& buffer)
{
    StretchDIBits(deviceContext,
		// Destination
        0, 0, windowWidth, windowHeight,
        // Source
        0, 0, buffer.width, buffer.height,
        buffer.memory, &buffer.info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message,WPARAM w_param,LPARAM l_param)
{
    LRESULT result = 0;
	switch (message)
	{
		case WM_SIZE:
			{
                // Resizing handled in Main
			}
			break;
		case WM_DESTROY:
            is_application_running = false;
            break;
		case WM_CLOSE:
            is_application_running = false;
            break;
		case WM_ACTIVATEAPP:
            break;
        case WM_PAINT:
	        {
		        PAINTSTRUCT paint;
        		HDC deviceContext = BeginPaint(window, &paint);

		        win32_window_dimensions windowDimensions = GetWindowDimension(window);

                Win32DisplayBufferInWindow(deviceContext, windowDimensions.width, windowDimensions.height, global_back_buffer);

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

    //win32_window_dimensions windowDimensions = GetWindowDimension(window);
    Win32ResizeDIBSection(&global_back_buffer, 1280, 720);

    mainWindow.style = CS_HREDRAW | CS_VREDRAW;
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
			is_application_running = true;
            int xOffset = 0;
            int yOffset = 0;
				
            MSG message;

            while (is_application_running)
            {
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
	                    is_application_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                Win32RenderWeirdGradient(global_back_buffer, xOffset, 0);
                HDC deviceContext = GetDC(WindowHandle);

                win32_window_dimensions windowDimensions = GetWindowDimension(WindowHandle);

                Win32DisplayBufferInWindow(deviceContext, windowDimensions.width, windowDimensions.height, global_back_buffer);
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