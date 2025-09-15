#include <Windows.h>

LRESULT CALLBACK MainWindowCallback(HWND window, UINT message,WPARAM w_param,LPARAM l_param)
{
    LRESULT result = 0;
	switch (message)
	{
		case WM_SIZE:
            OutputDebugStringA("WM_Size \n");
            break;
		case WM_DESTROY:
            OutputDebugStringA("WM_Destroy \n");
            break;
		case WM_CLOSE:
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

                static DWORD operation = WHITENESS;

        		PatBlt(deviceContext, x, y, w, h, operation);

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
    mainWindow.lpfnWndProc = MainWindowCallback;
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
            for (;;)
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