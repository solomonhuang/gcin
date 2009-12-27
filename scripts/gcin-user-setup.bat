set gcin_sys=%GCIN_DIR%
set gcin_table=%gcin_sys%\table
set gcin_user=%APPDATA%\gcin
set gcin_config=%gcin_user%\config

if not exist "%gcin_user%" md "%gcin_user%"
if not exist "%gcin_config%" md "%gcin_config%"

cd "%gcin_user%"

if not exist pho.tab copy "%gcin_table%\pho.tab"
if not exist pho-huge.tab copy "%gcin_table%\pho-huge.tab"
if not exist tsin32 copy "%gcin_table%\tsin32"
if not exist tsin32.idx copy "%gcin_table%\tsin32.idx"
if not exist symbol-table copy "%gcin_table%\symbol-table"
if not exist phrase.table copy "%gcin_table%\phrase.table"
