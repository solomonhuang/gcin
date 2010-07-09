echo "enter gcin-user"
set gcin_sys=%GCIN_DIR%
set gcin_table=%gcin_sys%\table
set gcin_user=%APPDATA_GCIN%
set gcin_config=%gcin_user%\config

if not exist "%gcin_user%" md "%gcin_user%"
if not exist "%gcin_config%" md "%gcin_config%"
