if exist %1 goto edit_it
echo aaaa phrases > %1
echo bbbb characters >> %1
:edit_it
set gcin_sys=%GCIN_DIR%
set gcin_script=%gcin_sys%\script
call "%gcin_script%\utf8-edit.bat" %1
