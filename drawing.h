#ifndef DRAWING_H
#define DRAWING_H

#include "main.h"

void InitVisuals(void);

#define VectorEmpty(a) ((a)[0]==0.0&&(a)[1]==0.0&&(a)[2]==0.0)

#define M_D2R   00.01745329251994329577 //M_PI/180
#define M_R2D   57.29577951308232087679 //180/M_PI

#define _DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])

float VectorNormalize(float * v);
float VectorLength(const float *v);
float VectorDistance(const float* orig,const float* dest);
float VectorAngle(const float *a, const float *b);
void VectorAngles(const float *forward, float *angles);
void AnglesVector(const float *angle, float *vector);
inline float AngleNormalizePitch(float angle);
inline float AngleNormalizeYaw(float angle);
inline float AngleNormalize(float angle);
void VectorTransform(float *in1, float in2[3][4], float *out);
void VectorRotateX(const float *in, float angle, float *out);
void VectorRotateY(const float *in, float angle, float *out);
void VectorRotateZ(const float *in, float angle, float *out);

bool CalcScreen(const float *origin, float *vecScreen);
void TraceGround(float * vInput,float * vOutput);
void pfnTintRGBA(int x,int y,int width,int height,int r,int g,int b,int a);
void DrawLine(float *from, float *to,int r,int g,int b,float d);
void DrawTrace(int ent,float life,float width,int r,int g,int b);
void DrawString(bool bCenter, int x, int y, int r, int g, int b, const char *pInput, ...);
void DrawSpecialString(bool bCenter, int x, int y, int r, int g, int b, const char *fmt, ...);
void DrawHudString(bool bCenter,int x, int y, int r, int g, int b, const char *fmt, ... );
void DrawBox(int x,int y,int radius,int width,int r,int g,int b,int a);
void DrawFilledBox(int x,int y,int r,int w,int br,int bg,int bb,int ba,int cr,int cg,int cb,int ca);
void DrawCross(int x,int y,int w,int h,int r, int g, int b, int a);
void DrawCrosshair(int x=0,int y=0,int sWidth=0,int sHeight=0);
void DrawRectangle(int x,int y,int w, int h,int r,int g,int b,int a);
void Draw3DEdge(int x,int y,int w,int h,int br, int bg, int bb, int ba,int dr, int dg, int db, int da);
void DrawCircle(int x,int y,int s,int r,int g,int b,int a);
void DrawSemiCircle(int x,int y,int s,int ag,int am,int r,int g,int b,int a);
void DrawVectorDot(float *origin,int r,int g,int b,int a);
void DrawDot(int x,int y,int r,int g,int b,int a);
void EchoCmdBlocked(char *cmd);

extern int displayCenterX;
extern int displayCenterY;
extern int fontWidth;
extern int fontHeight;
extern SCREENINFO screeninfo;
extern struct cvar_s *chase_active,*r_drawviewmodel,*cl_showfps,*hud_draw,*hud_saytext_time,*cl_timeout;

#endif //DRAWING_H
