#!/bin/bash

java -classpath idl2npapi/antlr.jar:idl2npapi/freemarker.jar:idl2npapi/classes org.appcelerator.titanium.idl2npapi.IDLToNPAPI $@
