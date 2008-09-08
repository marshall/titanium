/* $Id: bug00011.c,v 1.1.2.2 2007/04/10 20:32:37 pajoye Exp $ */
#include "gd.h"
#include <stdio.h>
#include <stdlib.h>
#include "gdtest.h"

int main()
{
 	gdImagePtr im;
	FILE *fp;
	char path[2048];

	sprintf(path, "%s/png/emptyfile", GDTEST_TOP_DIR);
	printf("opening %s\n", path);
	fp = fopen(path, "rb");
	if (!fp) {
		printf("failed, cannot open file\n");
		return 1;
	}
	im = gdImageCreateFromPng(fp);
	fclose(fp);

	if (!im) {
		return 0;
	} else {
		return 1;
	}
}
