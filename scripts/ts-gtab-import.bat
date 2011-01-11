set gcin_sys=%GCIN_DIR%
set gcin_bin=%gcin_sys%\bin
set gcin_script=%gcin_sys%\script
set gcin_user=%APPDATA_GCIN%
cd /d %gcin_user%

set _f=%1
set f=%_f%.append.gtab.tsin-db

if exist %f% goto found
set f=%_f%.tsin-db
echo %f%
if exist %f% goto found
echo "No %_f.*.gtab.tsin-db exist"
rem exit 0
pause

:found
copy %2 tmpfile
"%gcin_bin%"\tsd2a32 %f% >> tmpfile
"%gcin_bin%"\tsa2d32 tmpfile %f%
