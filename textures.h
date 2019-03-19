#ifndef TEXTURE_H
#define TEXTURE_H

#include "main.h"
#include <ctype.h>

#define CBTEXTURENAMEMAX	13
#define CTEXTURESMAX		  512

#define CHAR_TEX_CONCRETE	'C'
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_DIRT		  'D'
#define CHAR_TEX_VENT		  'V'
#define CHAR_TEX_GRATE		'G'
#define CHAR_TEX_TILE		  'T'
#define CHAR_TEX_SLOSH		'S'
#define CHAR_TEX_WOOD		  'W'
#define CHAR_TEX_COMPUTER	'P'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_FLESH		'F'

void InitTextureTypes(struct playermove_s *ppmove);
char GetTextureType(pmtrace_t *tr, float *vecSrc, float *vecEnd);

#endif //TEXTURE_H

