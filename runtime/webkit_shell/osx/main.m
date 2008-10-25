//
//  main.m
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright Appcelerator, Inc 2008. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <unistd.h>

int argCount;
char** args;

int main(int argc, char *argv[])
{
	/*umask(022);
	int stderrSave = dup(STDERR_FILENO);
	FILE *newStderr = freopen("/tmp/webkit_shell.log", "a", stderr);*/
	
	argCount = argc;
	args = argv;
	int retval = NSApplicationMain(argc,  (const char **) argv);
	
	/*fflush(stderr);
	dup2(stderrSave, STDERR_FILENO);
	close(stderrSave);*/
	

	return retval;
}
