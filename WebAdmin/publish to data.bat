del "..\Firmware-V1\data\html\view\*.html"
xcopy .\data\html\*.svg ..\Firmware\data\html\ /E /Y
xcopy .\data\html\*.min.html ..\Firmware\data\html\ /E /Y
xcopy .\data\html\*.min.css ..\Firmware\data\html\ /E /Y
xcopy .\data\html\*.min.js ..\Firmware\data\html\ /E /Y

copy %APPDATA%\..\Local\Temp\VMBuilds\Firmware-V1\esp8266_d1_mini\Release\Firmware-V1.ino.bin ..\Firmware-V1\data\firmware.bin

readline


