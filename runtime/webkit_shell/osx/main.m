//
//  main.m
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright Appcelerator, Inc 2008. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int argCount;
char** args;

int main(int argc, char *argv[])
{
	argCount = argc;
	args = argv;
	return NSApplicationMain(argc,  (const char **) argv);
}
