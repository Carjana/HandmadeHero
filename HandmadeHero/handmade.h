#pragma once
#if !defined(HANDMADE_H)

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

static void UpdateGameAndDraw(game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer, int hz);

#define HANDMADE_H
#endif