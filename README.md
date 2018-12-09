# WiFi-relay
Інтерфейс користувача
1. Керування виходами (ON/OFF)
2. Налаштування включення/виключення виходів по графіку
+3. Налаштування WIFI точки доступу
+4. MQTT вихід
* Кожен прилвд має своє унікальне імя (hostname), яке формується з слова "relay" + AMC адреса
* Кожен прилад має перемикач вбудованого світлодіода та перемикачі виходів із назвами (led, out1, out2, .., outN)
* Кожен вихід повязаний із трьома топіками MQTT хмари 
	Наприклад вихід №1
	* STATE topik => /switches/< hostname >/out1
	* SET topic => /switches/< hostname >/out1/set	
	* AVAILABLE topic => /switches/< hostname >/out1/available
	Наприклад вбудований світлодіод
	* STATE topik => /switches/< hostname >/led
	* SET topic => /switches/< hostname >/led/set	
	* AVAILABLE topic => /switches/< hostname >/led/available
