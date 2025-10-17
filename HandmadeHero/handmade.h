#pragma once
#if !defined(HANDMADE_H)

struct game_offscreen_buffer
{
    void* memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

void UpdateGameAndDraw(game_offscreen_buffer *buffer);

#define HANDMADE_H
#endif