# WiFi-relay

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FQRAJZS2GFXQU&source=url)

Інтерфейс користувача
1. Меню [Wifi реле] Керування виходами (ON/OFF) через web інтерфейс.
* Кожен вихід відображено у вигляді плитки. При натисканні мишеою на плитку - стан виходу змінюється на протилежний.
* Кожен вихід має сторінку налаштувань, де можна додавати чи змнювати тригери
* Кожен вихід має підтримку публікації та підпики на MQTT топіки.
* Прилад може створювати свою власну точку доступу, або підєднатися до існуючої WIFI мережі.
* Прилад підтримую визначення мереж Microsoft і відображається у мережевому оточенні MS Windows.
2. MQTT
* Кожен прилад має своє унікальне імя (hostname), яке формується з слова "relay" + MAC адреса
* Кожен вихід повязаний із трьома топіками MQTT хмари
	* Наприклад вихід №1
		* STATE topik => /switches/< hostname >/out1
		* SET topic => /switches/< hostname >/out1/set	
		* AVAILABLE topic => /switches/< hostname >/out1/available
	* Наприклад вбудований світлодіод
		* STATE topik => /switches/< hostname >/led
		* SET topic => /switches/< hostname >/led/set	
		* AVAILABLE topic => /switches/< hostname >/led/available
3. API для роботи з вебінтерфейсу
	1. GET api/wifi - перелік усіх точок доступу WIFI у радіусі доступності
	2. POST api/wifisave?n=ssid&p=psw - задати точку доступу до якої треба підєднатись після перезапуску
	3. GET api/swithces - перелік усіх доступних функцій приладу
	4. POST api/swithces?index=1&state=on   - задати стан виходу. Можливі стани: on - включити, x - переключити стан на протилежний, інакше - виключити
	5. GET api/template?name=templatename    - return file _templatename.html from html/v/ folder
	6. GET api/menu зчитати перелік пунктів меню
	7. GET api/setup?аргументи
		* delete=idТригера&switch=idВиходу видалити тригер за його id
		* type=switch&index=idВиходу зчитати перелік тригерів
		* type=onoff&index=idВиходу&mane=someName&days=daysMask&time=minutesFromDayStart&action=TrueOrFalse записати тригер для виходу idВиходу
		* type=pwm&index=idВиходу&mane=someName&days=daysMask&onlength=minutesON&offlength=minutesoff записати тригер для виходу idВиходу
		* type=termo&index=idВиходу&mane=someName&days=daysMask&start=minutesFromDayStart&end=minutesFromDayEnd&variable=nameOfVar&man=minVarValue&max=maxVarValue записати тригер для виходу idВиходу



## Тригери		
Кожен тригер може бути налаштований для роботи у певний день тиждня та у певні години доби.
### PWM trigger		
	Триер перемикає стан виходу відповідно до часу включення та виключення

### on/off trigger	
	Тригер включає/виелючає вихід у заданий час

### Включити/Виключити 
	Тригер включає вихід коли змінна манша ніж мінімум та вимикає вихід коли змінна більша за максимум
