#include "ed.h"
#include "r_ed.h"
#include "bsh.h"
#include "neighbor/in.h"
#include "neighbor/g.h"

void ed_Init()
{
    vec3_t brush_position = {3.0, 2.0, -5.0};
    vec3_t brush_scale;
    mat3_t brush_orientation;
    
    in_RegisterKey(SDL_SCANCODE_ESCAPE);
    bsh_CreateCubeBrush(&brush_position, &brush_orientation, &brush_scale);
    
}

void ed_Shutdown()
{
    
}

void ed_Main()
{
    if(in_GetKeyState(SDL_SCANCODE_ESCAPE) & IN_INPUT_STATE_PRESSED)
    {
        g_Quit();
    }
    
    r_DrawBrushes();
}
