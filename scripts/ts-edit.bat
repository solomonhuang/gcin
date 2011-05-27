set gcin_sys=%GCIN_DIR%
set gcin_bin=%gcin_sys%\bin
set gcin_script=%gcin_sys%\script
set gcin_user=%APPDATA_GCIN%

cd /d %gcin_user%
"%gcin_bin%"\tsd2a32 %1 > tmpfile
call "%gcin_script%"\utf8-edit.bat tmpfile
"%gcin_bin%"\tsa2d32 tmpfile %1
cd /d %gcin_script%
