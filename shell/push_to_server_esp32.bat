SET PROJDIR=C:\Users\mem\Documents\GitHub\esp-solar-contoller\Arduino\esp32_solar_controller
SET PUBUPDIR=C:\Users\mem\Documents\GitHub\esp-solar-contoller\updates\esp32v4
SET latest=C:\Users\mem\Documents\GitHub\esp-solar-contoller\Arduino\esp32_solar_controller\data\esp32_solar_controller.ino.bin

echo copy html files
copy /Y %PROJDIR%\data\*.htm %PUBUPDIR%\data\

echo copy css files
copy /Y %PROJDIR%\data\*.css %PUBUPDIR%\data\

echo copy firmware
copy /Y  %latest% %PUBUPDIR%\firmware.bin

echo update cv.txt

"C:\Program Files (x86)\GnuWin32\bin\grep.exe"  "#define FW_VERSION" %PROJDIR%\esp32_solar_controller.ino | "C:\Program Files (x86)\GnuWin32\bin\awk.exe" "{print $3}" > %PUBUPDIR%/cv.txt

echo null changelog.
echo "" > %PUBUPDIR%\changelog.txt

echo
echo Done.
pause
