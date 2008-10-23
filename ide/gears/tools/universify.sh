#!/bin/sh

# Creates a universal binaries for the Gecko SDK. Intended to be run from the
# obj directory of a Gecko SDK checkout like so:
#
#   universify.sh i386/path/to/some/file.dylib

ppc="${1//i386/ppc}"
output="${1##*/}"
lipo -create $1 $ppc -output $output
