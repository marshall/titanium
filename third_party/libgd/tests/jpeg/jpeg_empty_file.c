/* $Id: jpeg_empty_file.c,v 1.1.2.3 2007/04/10 20:35:21 pajoye Exp $ */
#include "gd.h"
#include <stdio.h>
#include <stdlib.h>
#include "gdtest.h"

int main()
{
 	gdImagePtr im;
	FILE *fp;
	char path[1024];

	sprintf(path, "%s/jpeg/empty.jpeg", GDTEST_TOP_DIR);
	fp = fopen(path, "rb");
	if (!fp) {
		printf("failed, cannot open file\n");
		return 1;
	}

	im = gdImageCreateFromJpeg(fp);
	fclose(fp);

	if (!im) {
		return 0;
	} else {
		gdImageDestroy(im);
		return 1;
	}
}
