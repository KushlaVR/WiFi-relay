del "..\MQTT-Relay\data\html\view\*.html"
xcopy .\data\html\*.svg ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.html ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.css ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.js ..\MQTT-Relay\data\html\ /E /Y

copy %APPDATA%\..\Local\Temp\VMBuilds\MQTT-Relay\esp8266_d1_mini\Release\MQTT-Relay.ino.bin ..\MQTT-Relay\data\firmware.bin

readline


