
static uint32 MakeRGB(uint8 r, uint8 g, uint8 b)
{
    // NOTE: Little endian
    // XX RR GG BB
    return r << 16 | g << 8 | b;
}

static void RenderWeirdGradient(game_offscreen_buffer* buffer, int xOffset, int yOffset)
{

    uint8* row = (uint8*)buffer->memory;
    for (int y = 0; y < buffer->height; y++)
    {
        uint32* pixel = (uint32*)row;
        for (int x = 0; x < buffer->width; x++)
        {
            uint8 red = 0;
            uint8 green = 0;
            uint8 blue = 0;

            const int actualY = y + yOffset;
            const int actualX = x + xOffset;

            red = (255 - actualX);
            green = (actualX);
            blue = (255 - actualX) + (actualX);

            *pixel = MakeRGB(red, green, blue);
            pixel++;
        }

        row += buffer->pitch;
    }
}

static void UpdateGameAndDraw(game_offscreen_buffer *buffer, uint32 xOffset, uint32 yOffset)
{
    RenderWeirdGradient(buffer, xOffset, yOffset);
}