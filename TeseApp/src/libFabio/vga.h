
#include "../libFB-0.2.4/FBlib.h"

#define VGA_fill(C) FB_clear_screen(C)
#define VGA_rect(X1,Y1,X2,Y2,C) FB_rect(X1, Y1, X2, Y2, C)
#define VGA_fill_rect(X1,Y1,X2,Y2,C) FB_rectfill(X1, Y1, X2, Y2, C)
#define VGA_line(X1,Y1,X2,Y2,C) FB_line(X1,Y1,X2,Y2,C)
#define VGA_set_pixel(X,Y,C) FB_putpixel(X, Y, C)
#define VGA_drawText(TEXT, X,Y,C) FB_printf(X, Y, C, TEXT)
#define VGA_setFontSize(X)  

#define VGA_loadBitmap(NAME) FB_bitmapLoad(NAME)
#define VGA_drawBitmap_Transparent(BITMAP,X,Y,TRANSP_C) FB_bitmapDraw_T(BITMAP,X,Y, TRANSP_C)
#define VGA_destroyBitmap(BITMAP) FB_bitmapDestroy(BITMAP)

#define COLOR_WHITE 	FB_makecol(255,255,255,0)
#define COLOR_BLACK 	FB_makecol(0,0,0,0)
#define COLOR_RED 	FB_makecol(255,0,0,0)
#define COLOR_BLUE 	FB_makecol(0,0,255,0)
#define COLOR_GREEN 	FB_makecol(0,255,0,0)
#define COLOR_LIGHT_BLUE 	FB_makecol(50,150,255,0)
#define COLOR_DARK_BLUE 	FB_makecol(0,20,100,0)
#define COLOR_LIGHT_GRAY 	FB_makecol(160,160,160,0)
#define COLOR_DARK_GRAY 	FB_makecol(70,70,70,0)
#define COLOR_PINK 	FB_makecol(250,80,240,0)
#define COLOR_ORANGE 	FB_makecol(250,100,0,0)

#define WIN_COLOR 	FB_makecol(170,160,180,0)

	



