rem A cleaning batch file.
rem Cleans up most stuff.
rm -rf temp
del *.ncb /S
attrib -r -h -s *.suo /S
del *.suo /S
del *.WW /S
del /y lib\win32
del *.bak /S
del *.ilk /S
del *.idb /S
del *.pdb /S
del *.exp /S
del *.lib /S
del *.exe /S
del *.map /S

