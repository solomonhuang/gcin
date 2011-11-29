if exist %1 goto edit_it
set gcin_sys=%GCIN_DIR%
set gcin_script=%gcin_sys%\script
set gcin_table=%gcin_sys%\table
copy "%gcin_table%\gtab_append_example" %1
:edit_it
call "%gcin_script%\utf8-edit.bat" %1
