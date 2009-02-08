
sdk_commands = {}
from commands import *
import re

def printUsage():
	print ("Titanium SDK (c) 2008-2009 Appcelerator, Inc\n"+
		"Usage:\n"+
		"  ti <command> [<arguments>]\n"+
		"Commands:\n "+
		"  create:project <projectname>\n"
	)
	exit()

arguments = Titanium.App.arguments
if len(arguments) is 1:
	printUsage()

if len(arguments) is 2 and re.match(arguments[0], "-help$"):
	printUsage()
	

Titanium.api.setRunUILoop(False)

#sdk["create:project"] = CreateProject()

if not arguments[1] in sdk_commands:
	print "Titanium SDK (c) 2008-2009 Appcelerator, Inc\n" + "  Error: command %s not found\n" % arguments[1]
	exit()
	
sdk_commands[arguments[1]].execute(arguments[2:])