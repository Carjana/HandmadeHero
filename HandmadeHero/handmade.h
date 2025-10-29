#pragma once

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct game_offscreen_buffer
{
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int Bytes_per_pixel;
};

struct game_sound_output_buffer
{
    int32 SamplesPerSecond;
    int32 SampleCount;
    int16* Samples;
};

struct game_button_state
{
    int32 HalfTransitionCount;
    bool EndedDown;
};

struct game_controller_input
{
	float32 StartX;
	float32 StarY;

	float32 MinX;
	float32 MinY;

	float32 MaxX;
	float32 MaxY;

	float32 EndX;
	float32 EndY;
	union
	{
		
	    game_button_state Buttons[6];
		struct
		{
			game_button_state North;
			game_button_state South;
			game_button_state East;
			game_button_state West;
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
		};
	};

};

struct game_input
{
	game_controller_input Controllers[4];
};

static void UpdateGameAndDraw(game_input *input, game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer);