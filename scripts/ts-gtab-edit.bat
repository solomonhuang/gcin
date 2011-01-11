set gcin_sys=%GCIN_DIR%
set gcin_bin=%gcin_sys%\bin
set gcin_script=%gcin_sys%\script
set gcin_user=%APPDATA_GCIN%
cd /d %gcin_user%


set f=%1.append.gtab.tsin-db
if exist %f% goto found
set f=%1.tsin-db
echo %f%
if exist %f% goto found
echo "No %1.*.gtab.tsin-db exist"
rem exit 0

:found
set fsrc=tmpfile

"%gcin_bin%"\tsd2a32 %f% > %fsrc% 
call "%gcin_script%"\utf8-edit.bat %fsrc%
"%gcin_bin%"\tsa2d32 %fsrc% %f%
cd /d %gcin_script%
