#include "handmade.h"

static uint32 MakeRGB(uint8 r, uint8 g, uint8 b)
{
    // NOTE: Little endian
    // XX RR GG BB
    return r << 16 | g << 8 | b;
}

static void GameOutputSound(game_sound_output_buffer *soundBuffer, int32 toneHz)
{
    static float32 tSine;
    int16 toneVolume = 3000;

    int32 wavePeriod = soundBuffer->SamplesPerSecond / toneHz;

    int16 *sampleOut = soundBuffer->Samples;
    for (uint32 sampleIndex = 0; sampleIndex < soundBuffer->SampleCount; ++sampleIndex)
    {
        float sineValue = sinf(tSine);
        int16 sampleValue = (int16)(sineValue * toneVolume);

        // left
        *sampleOut++ = sampleValue;
        // right
        *sampleOut++ = sampleValue;

        tSine += 2.0f * PI32 * 1.0f / (float32)wavePeriod;
    }
}

static void RenderWeirdGradient(game_offscreen_buffer* buffer, int xOffset, int yOffset)
{

    uint8* row = (uint8*)buffer->Memory;
    for (int y = 0; y < buffer->Height; y++)
    {
        uint32* pixel = (uint32*)row;
        for (int x = 0; x < buffer->Width; x++)
        {
            uint8 red = 0;
            uint8 green = 0;
            uint8 blue = 0;

            const int actualY = y + yOffset;
            const int actualX = x + xOffset;

            red = (255 - actualX);
            green = (actualY);
            blue = (255 - actualX) + (actualX);

            *pixel = MakeRGB(red, green, blue);
            pixel++;
        }

        row += buffer->Pitch;
    }
}

static void UpdateGameAndDraw(game_memory *memory, game_input *input, game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer)
{

    Assert(sizeof(game_state) <= memory->permanentStorageSize)

	game_state* gameState = gameState = (game_state*)memory->permanentStorage;
    if (!memory->isInitialized)
    {
        gameState->xOffset = 0;
        gameState->yOffset = 0;
        gameState->toneHz = 256;
		memory->isInitialized = true;
    }

    if (input->Controllers[0].EndX > 0.4f)
    {
        gameState->xOffset++;
    }
    else if (input->Controllers[0].EndX < -0.4f)
    {
        if (gameState->xOffset == 0)
			gameState->xOffset = 255;
        gameState->xOffset--;
    }

    if (input->Controllers[0].EndY > 0.4f)
    {
        gameState->yOffset++;
    }
    else if (input->Controllers[0].EndY < -0.4f)
    {
        if (gameState->yOffset == 0)
            gameState->yOffset = 255;
        gameState->yOffset--;
    }

    gameState->xOffset = gameState->xOffset % 255;
    // Emagine / Emmagine : Engine name
    
    GameOutputSound(soundBuffer, gameState->toneHz);
    RenderWeirdGradient(buffer, gameState->xOffset, gameState->yOffset);
}