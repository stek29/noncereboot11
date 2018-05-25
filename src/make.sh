#!/bin/sh
xcrun -sdk iphoneos cc -Wall -arch arm64 *.c -o noncereboot11 -framework IOKit -framework CoreFoundation
jtool --sign --inplace --ent ent.plist noncereboot11

