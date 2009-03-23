//
//  main.m
//  progress
//
//  Created by Jeff Haynie on 1/28/09.
//  Copyright Appcelerator 2009. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++){
#ifdef DEBUG
		printf("Arg %d is: %s\n",i,argv[i]);
#endif
	}

    return NSApplicationMain(argc,  (const char **) argv);
}
