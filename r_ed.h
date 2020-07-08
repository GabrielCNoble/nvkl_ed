#ifndef R_ED_H
#define R_ED_H

#include "bsh.h"

void r_ed_DrawPolygon(struct bsh_polygon_t *polygon, vec3_t *color, uint32_t wireframe);

void r_ed_DrawBrush(struct bsh_brush_t *brush);

void r_ed_DrawBrushes();


#endif // R_ED_H
