# WiFi-relay
Інтерфейс користувача
1. Керування виходами (ON/OFF) через web інтерфейс
2. MQTT вихід
* Кожен прилад має своє унікальне імя (hostname), яке формується з слова "relay" + AMC адреса
* Кожен прилад має перемикач вбудованого світлодіода та перемикачі виходів із назвами (led, out1, out2, .., outN)
* Кожен вихід повязаний із трьома топіками MQTT хмари 
	* Наприклад вихід №1
		* STATE topik => /switches/< hostname >/out1
		* SET topic => /switches/< hostname >/out1/set	
		* AVAILABLE topic => /switches/< hostname >/out1/available
	* Наприклад вбудований світлодіод
		* STATE topik => /switches/< hostname >/led
		* SET topic => /switches/< hostname >/led/set	
		* AVAILABLE topic => /switches/< hostname >/led/available
3. Налаштування включення/виключення виходів по графіку
4. Налаштування WIFI точки доступу
5. API для роботи з вебінтерфейсу
	1. GET api/wifi - перелік усіх точок доступу WIFI у радіусі доступності
	2. POST api/wifisave?n=ssid&p=psw - задати точку доступу до якої треба підєднатись після перезапуску
	3. GET api/swithces - перелік усіх доступних функцій приладу
	4. POST api/swithces?name=outName&...   - задати значення однієї із функцій приладу