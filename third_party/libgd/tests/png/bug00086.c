/* $Id: bug00086.c,v 1.1.2.1 2007/05/17 14:38:24 pajoye Exp $ */
/* id: gdbad3.c, Xavier Roche, May. 2007 */
/* gcc gdbad3.c -o bad -lgd && ./bad */

#include <stdio.h>
#include <stdlib.h>
#include "gd.h"

static const unsigned char pngdata[93];
int main(void) {
	FILE *fp;
	gdImagePtr im;
	int i;
	size_t out;

	if ( ( im = gdImageCreateFromPngPtr(93, (char*) &pngdata[0]) ) == NULL) {
		fprintf(stderr, "success!\n");
		return 0;
	} else {
		fprintf(stderr, "failed!\n");
		gdImageDestroy(im);
		return 1;
	}
	return 0;
}

/* PNG data */
static const unsigned char pngdata[93] = {137,80,78,71,13,10,26,10,0,0,
0,13,73,72,68,82,0,0,0,120,0,0,0,131,8,6,0,0,0,70,49,223,8,0,0,0,6,98,
75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,92,
70,0,0,92,70,1,20,148,67,65,0,0,0,9,118,112,65,103,0,0,0,120,0,0,0,131,
0,226,13,249,45};

