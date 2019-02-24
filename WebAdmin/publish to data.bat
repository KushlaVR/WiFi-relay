del "..\MQTT-Relay\data\html\content\*.html"
xcopy .\data\html\*.svg ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.html ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*pwmedit.html ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.css ..\MQTT-Relay\data\html\ /E /Y
xcopy .\data\html\*.min.js ..\MQTT-Relay\data\html\ /E /Y

ren "..\MQTT-Relay\data\html\content\_onoffedit.min.html" "_onoffedit.html"
ren "..\MQTT-Relay\data\html\content\_newitem.min.html" "_newitem.html"
ren "..\MQTT-Relay\data\html\content\_termoedit.min.html" "_termoedit.html"

copy %APPDATA%\..\Local\Temp\VMBuilds\MQTT-Relay\esp8266_d1_mini\Release\MQTT-Relay.ino.bin ..\MQTT-Relay\data\firmware.bin

readline