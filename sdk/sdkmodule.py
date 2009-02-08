
sdk_commands = {}
from commands import *
import re

def printUsage():
	print ("Titanium SDK (c) 2008-2009 Appcelerator, Inc" +
		"Usage:" +
		"  ti <command> [<arguments>]" +
		"Commands:" +
		"  create:project <projectname>"
	)
	exit()

print str(Titanium)

if len(Titanium.App.arguments) is 0:
	printUsage()

if len(Titanium.App.arguments) is 1 and re.matches(Titanium.App.arguments[0], "-help$"):
	printUsage()
	

Titanium.api.setRunUILoop(False)
sdk_commands[Titanium.Spp.arguments[0]].execute(Titanium.Spp.arguments[1:])