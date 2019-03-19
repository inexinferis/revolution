#ifndef NOPENGL_H
#define NOPENGL_H

#include <gl/gl.h>
#include <gl/glu.h>

typedef void (WINAPI *tglEnable)(GLenum);
typedef void (WINAPI *tglDisable)(GLenum);
typedef void (WINAPI *tglColor4f)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (WINAPI *tglGetFloatv)(GLenum,GLfloat *);
typedef void (WINAPI *tglGetIntegerv)(GLenum,GLint *);
typedef void (WINAPI *tglClearColor)(GLclampf,GLclampf,GLclampf,GLclampf);
typedef void (WINAPI *tglTexEnvi)(GLenum,GLenum,GLenum);
typedef void (WINAPI *tglBlendFunc)(GLenum,GLenum);
typedef void (WINAPI *tglDepthRange)(GLclampd,GLclampd);
typedef void (WINAPI *tglDepthFunc)(GLenum);
typedef void (WINAPI *tglClear)(GLbitfield);
//typedef void (WINAPI *tglVertex2f)(GLfloat x,GLfloat y);
typedef void (WINAPI *tglVertex3f)(GLfloat,GLfloat,GLfloat);
typedef void (WINAPI *tglVertex3fv)(const GLfloat *);
typedef void (WINAPI *tglBegin)(GLenum);
typedef void (WINAPI *tglEnd)();
typedef void (WINAPI *tglPopMatrix)();
typedef void (WINAPI *tglFrustum)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void (WINAPI *tglReadBuffer)(GLenum);
typedef void (WINAPI *tglReadPixels)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,GLvoid*);
typedef void (WINAPI *tglPolygonMode)(GLenum face,GLenum mode);
typedef void (WINAPI *tglLineWidth)(GLfloat width);
typedef void (WINAPI *tglColor3f)(GLfloat red,GLfloat green,GLfloat blue);
typedef void (WINAPI *tglViewport)(GLint x,GLint y,GLsizei width,GLsizei height);
typedef void (WINAPI *tglPopMatrix)();
typedef void (WINAPI *twglSwapBuffers)(HDC hdc);
typedef void (WINAPI *tglGenTextures)(GLsizei n,GLuint* textures);
typedef void (WINAPI *tglBindTexture)(GLenum target,GLuint texture);
typedef void (WINAPI *tglTexImage2D)(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const GLvoid * data);
typedef void (WINAPI *tglTexParameteri)(GLenum target,GLenum pname,GLint param);
typedef void (WINAPI *tglTexCoord2f)(GLfloat s,GLfloat t);

//New OpenGL Functions
typedef PROC (WINAPI *twglGetProcAddress)(LPCSTR lpszProc);
typedef GLvoid (WINAPI *tglBindBuffer)(GLenum target, GLuint buffer);
typedef GLvoid* (WINAPI *tglMapBuffer)(GLenum target, GLenum access);
typedef GLboolean (WINAPI *tglUnmapBuffer)(GLenum target);
typedef void (WINAPI *tglGenBuffers)(GLsizei n,GLuint * buffers);
typedef void (WINAPI *tglBufferData)(GLenum target,GLsizei size,const GLvoid * data,GLenum usage);

typedef int (WINAPI *tSetCltProcTable)(PDWORD pDst1,PDWORD pDst2,DWORD iSize);

BOOL WINAPI HookOpenGl();
void WINAPI glCreateTextureBuffer(GLint width,GLint height,GLvoid* data);
void WINAPI glDrawTexture(GLint x,GLint y,GLint width,GLint height);

extern tglDepthRange pglDepthRange;
extern tglDepthFunc pglDepthFunc;
extern tglTexEnvi pglTexEnvi;
extern tglEnable pglEnable;
extern tglDisable pglDisable;
extern tglColor4f pglColor4f;
extern tglGetFloatv pglGetFloatv;
extern tglGetIntegerv pglGetIntegerv;
extern tglClearColor pglClearColor;
extern tglBlendFunc pglBlendFunc;
extern tglClear pglClear;
//extern tglVertex2f pglVertex2f;
extern tglVertex3f pglVertex3f;
extern tglVertex3fv pglVertex3fv;
extern tglBegin pglBegin;
extern tglEnd pglEnd;
extern tglFrustum pglFrustum;
extern tglReadBuffer pglReadBuffer;
extern tglReadPixels pglReadPixels;
extern tglPolygonMode pglPolygonMode;
extern tglLineWidth pglLineWidth;
extern tglColor3f pglColor3f;
extern tglViewport pglViewport;
extern tglGenTextures pglGenTextures;
extern tglBindTexture pglBindTexture;
extern tglTexImage2D pglTexImage2D;
extern tglTexParameteri pglTexParameteri;
extern tglTexCoord2f pglTexCoord2f;

//New OpenGL Functions
extern twglGetProcAddress pwglGetProcAddress;
extern tglBindBuffer pglBindBuffer;
extern tglMapBuffer pglMapBuffer;
extern tglUnmapBuffer pglUnmapBuffer;
extern tglGenBuffers pglGenBuffers;
extern tglBufferData pglBufferData;

extern bool oglSubtractive;
extern bool oglChams;
extern float oglChamsColor[3];

#endif //NOPENGL_H
