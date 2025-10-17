#include <Windows.h>
#include <cstdint>
#include <GameInput.h>
#include <string>
#include <Dsound.h>
#include <math.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "GameInput.lib")

#define PI32 3.14159265359f

using namespace GameInput::v2;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float_t float32;
typedef double_t float64;

#include "handmade.h"
#include "handmade.cpp"

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

struct win32_sound_output
{
    int samplesPerSecond;
    int hz;
    int16 toneVolume;
    uint32 runningSampleIndex;
    int wavePeriod;
    int bytesPerSample;
    int secondaryBufferSize;
};

static bool is_application_running;
static win32_offscreen_buffer global_back_buffer;
static LPDIRECTSOUNDBUFFER secondaryBuffer;

static void Win32InitDSound(HWND window,int32 samplesPerSecond, int32 bufferSize)
{
    // TODO: DirectSound is depricated, use XAudio2

    LPDIRECTSOUND directSound;
    if (SUCCEEDED(DirectSoundCreate(0, &directSound, 0)))
    {
        WAVEFORMATEX waveFormat = {};
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 2;
        waveFormat.nSamplesPerSec = samplesPerSecond;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        directSound->SetCooperativeLevel(window, DSSCL_PRIORITY);

        LPDIRECTSOUNDBUFFER primaryBuffer;
        DSBUFFERDESC bufferDescription = {};
        bufferDescription.dwSize = sizeof(bufferDescription);
		bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        bufferDescription.dwBufferBytes = 0;

        directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0);

        if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
        {
            OutputDebugStringA("Primary Buffer set");
        }

        DSBUFFERDESC secondaryBufferDescription = {};
		secondaryBufferDescription.dwSize = sizeof(secondaryBufferDescription);
        secondaryBufferDescription.dwFlags = 0;
		secondaryBufferDescription.dwBufferBytes = bufferSize;
        secondaryBufferDescription.lpwfxFormat = &waveFormat;

        if (SUCCEEDED(directSound->CreateSoundBuffer(&secondaryBufferDescription, &secondaryBuffer, 0)))
        {
            OutputDebugStringA("Secondary Buffer Created");
        }
    }
}

static void Win32FillSoundBuffer(win32_sound_output *soundOutput, DWORD bytesToLock, DWORD bytesToWrite)
{
    void* region1;
    DWORD region1Size;
    void* region2;
    DWORD region2Size;


    if (SUCCEEDED(secondaryBuffer->Lock(bytesToLock, bytesToWrite,
        &region1, &region1Size,
        &region2, &region2Size,
        0)))
    {


        DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
        int16* sampleOut = (int16*)region1;

        for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; sampleIndex++)
        {
            float t = 2.0f * PI32 * ((float)soundOutput->runningSampleIndex / (float)soundOutput->wavePeriod);
            float sineValue = sinf(t);
            int16 sampleValue = (int16)(sineValue * soundOutput->toneVolume);

            // left
            *sampleOut++ = sampleValue;
            // right
            *sampleOut++ = sampleValue;
            soundOutput->runningSampleIndex++;
        }

        DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
        sampleOut = (int16*)region2;

        for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++)
        {
            float t = 2.0f * PI32 * ((float)soundOutput->runningSampleIndex / (float)soundOutput->wavePeriod);
            float sineValue = sinf(t);
            int16 sampleValue = (int16)(sineValue * soundOutput->toneVolume);
            // left
            *sampleOut++ = sampleValue;
            //  right
            *sampleOut++ = sampleValue;
            soundOutput->runningSampleIndex++;
        }

        secondaryBuffer->Unlock(region1, region1Size,
            region2, region2Size);
    }
}

win32_window_dimensions GetWindowDimension(HWND window)
{
	win32_window_dimensions result;
	RECT clientRect;
    GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;

    return result;
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

static void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight, win32_offscreen_buffer *buffer)
{
    StretchDIBits(deviceContext,
		// Destination
        0, 0, windowWidth, windowHeight,
        // Source
        0, 0, buffer->width, buffer->height,
        buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
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

                Win32DisplayBufferInWindow(deviceContext, windowDimensions.width, windowDimensions.height, &global_back_buffer);

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
            // graphics test
            int xOffset = 0;
            int yOffset = 0;

            // sound test

            win32_sound_output soundOutput;

            soundOutput.samplesPerSecond = 48000;
            soundOutput.hz = 256;
            soundOutput.toneVolume = 3000;
            soundOutput.runningSampleIndex = 0;
            soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.hz;
            soundOutput.bytesPerSample = sizeof(int16) * 2;
            soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * sizeof(int16) * 2;
			
            Win32InitDSound(WindowHandle, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
            Win32FillSoundBuffer(&soundOutput, 0, soundOutput.secondaryBufferSize);
            secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            // input test
            IGameInput* gameInput = nullptr;
            if (FAILED(GameInputCreate(&gameInput)))
            {
                MessageBox(WindowHandle, L"GameInputCreate Failed", L"Error", MB_OK);
                return -1;
            }

            MSG message;

            LARGE_INTEGER performanceFrequency;
            QueryPerformanceFrequency(&performanceFrequency);

        	LARGE_INTEGER lastCounter;
            QueryPerformanceCounter(&lastCounter);

			int64 lastCycleCount = __rdtsc();

            // Loop
            while (is_application_running)
            {
                // Windows Messages
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
	                    is_application_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                // Input
                IGameInputReading* reading;
                if (SUCCEEDED(gameInput->GetCurrentReading(GameInputKindController | GameInputKindGamepad, nullptr, &reading)))
                {
                    // Controller Input

					GameInputGamepadState state;

                    GameInputKind currentKind = reading->GetInputKind();

                    if (currentKind == GameInputKindGamepad)
                    {
                        reading->GetGamepadState(&state);
                    }
                    else if (currentKind = GameInputKindController)
                    {
                        uint32_t axisCount = reading->GetControllerAxisCount();

                        float *axisStateArray = new float[axisCount]{};

                        reading->GetControllerAxisState(axisCount, axisStateArray);

                        if (axisCount < 6)
                        {
	                        // Not allowed!
                        }

                        // PS5 Controller

                        // fill axis state;
                        state.leftThumbstickX = axisStateArray[0];
                        state.leftThumbstickY = axisStateArray[1];
                        state.rightThumbstickX = axisStateArray[2];
                        state.rightThumbstickY = axisStateArray[5];
                        state.leftTrigger = axisStateArray[3];
                        state.rightTrigger = axisStateArray[4];

                    	//fill button state

                        uint32_t buttonCount = reading->GetControllerButtonCount();
                        bool* buttonStateArray = new bool[buttonCount]{};
                        reading->GetControllerButtonState(buttonCount, buttonStateArray);

                        uint32_t switchCount = reading->GetControllerSwitchCount();
                        GameInputSwitchPosition *switchArray = new GameInputSwitchPosition[switchCount];
                        reading->GetControllerSwitchState(switchCount, switchArray);

                        uint32 buttons = GameInputGamepadNone;

						buttons = buttons | buttonStateArray[9] << (uint32)log2(GameInputGamepadMenu);
						buttons = buttons | buttonStateArray[13] << (uint32)log2(GameInputGamepadView);
						buttons = buttons | buttonStateArray[1] << (uint32)log2(GameInputGamepadA);
						buttons = buttons | buttonStateArray[2] << (uint32)log2(GameInputGamepadB);
						buttons = buttons | buttonStateArray[0] << (uint32)log2(GameInputGamepadX);
						buttons = buttons | buttonStateArray[3] << (uint32)log2(GameInputGamepadY);
						buttons = buttons | (switchArray[0] == GameInputSwitchUp || switchArray[0] == GameInputSwitchUpLeft || switchArray[0] == GameInputSwitchUpRight) << (uint32)log2(GameInputGamepadDPadUp);
						buttons = buttons | (switchArray[0] == GameInputSwitchDown || switchArray[0] == GameInputSwitchDownLeft || switchArray[0] == GameInputSwitchDownRight) << (uint32)log2(GameInputGamepadDPadDown);
						buttons = buttons | (switchArray[0] == GameInputSwitchLeft || switchArray[0] == GameInputSwitchUpLeft || switchArray[0] == GameInputSwitchDownLeft) << (uint32)log2(GameInputGamepadDPadLeft);
                        buttons = buttons | (switchArray[0] == GameInputSwitchRight || switchArray[0] == GameInputSwitchUpRight || switchArray[0] == GameInputSwitchDownRight) << (uint32)log2(GameInputGamepadDPadRight);
						buttons = buttons | buttonStateArray[4] << (uint32)log2(GameInputGamepadLeftShoulder);
						buttons = buttons | buttonStateArray[5] << (uint32)log2(GameInputGamepadRightShoulder);
						buttons = buttons | buttonStateArray[10] << (uint32)log2(GameInputGamepadLeftThumbstick);
						buttons = buttons | buttonStateArray[11] << (uint32)log2(GameInputGamepadRightThumbstick);


						GameInputGamepadButtons gamepadButtons = static_cast<GameInputGamepadButtons>(buttons);

                        state.buttons = gamepadButtons;

                        // cleanup
                        delete[] axisStateArray;
                        delete[] buttonStateArray;
                        delete[] switchArray;
                    }
                    reading->Release();
                }

                // Rendering
                game_offscreen_buffer buffer;
				buffer.memory = global_back_buffer.memory;
				buffer.width = global_back_buffer.width;
				buffer.height = global_back_buffer.height;
				buffer.pitch = global_back_buffer.pitch;
				buffer.bytes_per_pixel = global_back_buffer.bytes_per_pixel;

                UpdateGameAndDraw(&buffer, xOffset, yOffset);

                HDC deviceContext = GetDC(WindowHandle);

                win32_window_dimensions windowDimensions = GetWindowDimension(WindowHandle);

                Win32DisplayBufferInWindow(deviceContext, windowDimensions.width, windowDimensions.height, &global_back_buffer);
                ReleaseDC(WindowHandle, deviceContext);
				xOffset++;
				xOffset = xOffset % 255;

                // DirectSound output test
				DWORD playCursor = 0;
                DWORD writeCursor = 0;
                if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
                {
                    DWORD bytesToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
                    DWORD bytesToWrite;

                    if(bytesToLock == playCursor)
                    {
                        bytesToWrite = 0;
                    }
					else if (bytesToLock > playCursor)
                    {
                        bytesToWrite = (soundOutput.secondaryBufferSize - bytesToLock);
                        bytesToWrite += playCursor;
                    }
                    else
                    {
						bytesToWrite = playCursor - bytesToLock;
                    }

					Win32FillSoundBuffer(&soundOutput, bytesToLock, bytesToWrite);
                }

				int64 currentCycleCount = __rdtsc();

                LARGE_INTEGER currentCounter;
				QueryPerformanceCounter(&currentCounter);
				int64 cyclesElapsed = currentCycleCount - lastCycleCount;
				int64 counterElapsed = currentCounter.QuadPart - lastCounter.QuadPart;

                float32 msPerFrame = 1000*counterElapsed / (float32)performanceFrequency.QuadPart;

				int32 fps = performanceFrequency.QuadPart / counterElapsed;
                // mega cycles per frame
				float32 mcpf = (float32)cyclesElapsed / (1000 * 1000);

                char testbuffer[256];
                sprintf_s(testbuffer, "%fms %df/s %fmc/f\n", msPerFrame, fps, mcpf);

                OutputDebugStringA(testbuffer);

				lastCounter = currentCounter;
				lastCycleCount = currentCycleCount;
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