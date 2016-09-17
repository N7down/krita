install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib /Users/n7down/Source/kde/dev/i/bin/krita.app/Contents/MacOS/krita


FILES="$(find /Users/n7down/Source/kde/dev/i/lib/ -name '*so' -o -name '*dylib')"
for FILE in $FILES ; do
    echo $FILE
    install_name_tool -change libboost_system.dylib @rpath/libboost_system.dylib $FILE
done
