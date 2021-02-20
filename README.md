# WiFi-relay


# How it works
Device works as solid state relay with 1 AC 220V input and 3 AC 220V outputs
Device can works in 2 mode
- Standalone mode
- Slave mode

Device can be configured from:
- web user interface (through API)
- android app

![What you get in result](/image-originals/20200208_185149.jpg)

# First run
After startup device start SoftAP with special name witch contain important information for Android AP
## AP name Format:
KVR_[AP KEY]
## AP name example:
KVR_AABBCCDDEEFF



# WIFI configuration
Device look for wifi settings in /cfg/wifi.json after startup.
Every time you [setup network connection](/Documentation/Web-API) device store given ssid & key in wifi.json  file. History length of stored wifi ssid & key pair is 3. When device loose connection to current wifi station it start to loock for next available ssid. if one available - device connects using ssid & key.  
## wifi.json file example:
``` js
{
  "mode": "server",//start soft AP (can be disable after first setup) (can be: server or client)
  "ssid": "KVR_",//wifi softAP name prefix
  "key": "",//key for wifi soft AP. If empty - MAC address is used to acess to AP
  "ip": "192.168.4.1",//ip address of soft AP
  "mask": "255.255.255.0",//network mask
  "wifi": [//saved wifi credentionals of known networks
    {
      "ssid": "network1",
      "key": "12345678"
    },
    {
      "ssid": "network2",
      "key": "123456789"
    },
    {
      "ssid": "network3",
      "key": "987654321"
    }
  ]
}
```

# NTP configuration
Device have no RTC battary, so NTP time synchronization is used.
Connection parameters of NTP server and time zone configuration stored in ntp.json file
## ntp.json file example:
``` js
{
  "port": "2390",//NTP port
  "server": "europe.pool.ntp.org",//NTP server name/ip
  "standart-time": "Last, Sun, Oct, 4, +2",//Time offset from greenwich winter
  "summer-time": "Last, Sun, Mar, 3, +3"//Time offset from greenwich summer
}
```

# MQTT configuration
MQTT server connection setting stored in /cfg/mqtt.json
## mqtt.json file example
``` js
{
  "server": "192.168.5.100",//ip address of mqtt broker
  "port": "1883",//mqtt broker port
  "user": "",//mqtt user
  "password": ""//mqtt password
}
```

# MQTT API
# Outputs
The device have 3 high current outputs (out1, out2, out3) and 1 buldin led output.
Buildin led is used only for testing device functionaluty at first setup. It have no effect except blinkin.
Each output linked to 3 mqtt topic. Name of then topiks contains from MAC address of device and output name.
- **state topic** "/switches/AABBCCDDEEFF/name"
Evry time the state of output is changed - device publish new state ("ON"/"OFF") to payload of this topic
- **set topic** "/switches/AABBCCDDEEFF/name/set"
Device subscribe for this topic and set the state of output to on when recive "ON" payload else set state to off
- **available topic** "/switches/AABBCCDDEEFF/name/available"
Available topic is used to signalize to automation system that output gose online. Evry time device is connected to mqtt broker - it publish "online" payload data

**AABBCCDDEEFF** - mac address of device (upper case, no separator character)
**name** - name of output


# WEB API
# GET **/api/auth**
API authorization
## Request
``` js
  /api/auth?user=user&password=password&json=true
```
user- user name

password - password

json - define responce format by setting this parameter. If parameter is present (don't metter what value is) device return code 200 and session id in json format. If json paramater don't present - then device returns 302 responce code is authorisation is success.

Before call any API function you must be authorized on device. If authorization success - device generate unique session ID and write user name and id to Cookie header (Cookie "user=<userName>;session=<session key>" Note - only device should write session and user to Cookies.

## Responce (code 302)
``` js
Success if session and user cookies present in responce header
```


## Responce (code 200)
``` js
{"user":"user", "session":"123456"}
```

After call of any API function - device will check Cookies session ID and user name with stored in EEPROM memory user and session data. If Session and user match with stored session and user then authorization successed and API function return result code OK. else it will return 404 error code.

# GET **/api/wifi**
Return list of awailable wifi ap
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
  "mac":"AA:BB:CC:DD:EE:FF",// wifi adapter mac address
  "localip":"192.168.0.2",//ip address of device
  "getway":"192.168.0.1",//ip address of router
  "dnsip":"192.168.0.1",//ip address of primary DNS server
  "ssid"://list of awailable wifi netwoks
  [
    {"name":"name1", "encryption":"type of ecryption", "rssi":"siglal level"},
    {"name":"name2", "encryption":"type of ecryption", "rssi":"siglal level"},
    ..
    {"name":"nameX", "encryption":"type of ecryption", "rssi":"siglal level"},
  ]
}
```

# GET **/api/wifisave**
Add SSID and key to known networks of device (3 networks allowed by default)
## Request
``` js
  /api/wifisave?n=SSID&p=SSKEY&mode=server
```
SSID - name of wifi network to connect

SSKEY - key for wifi network

mode - optional. Possible value server or client. In server mode - device start soft AP. This allows user to connect to device from android App. In client mode soft AP disabled and device connect to available wifi network.

Use this parameter only if you want to switch device mode. If you dont want to change mode after setting wifi credentionals - dont pass this parameter

## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt"//long
}
```

# GET **/api/mqtt**
Return current MQTT server connection parameters
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
  "broker", "broker IP/url",
  "port", "port number",
  "user", "user name(empty if broker is public)",
  "key", "authorization key"
}
```

# GET **/api/mqttsave**
Save MQTT server connection parameters
## Request
``` js
  /api/mqttsave?broker=broker_name&port=port_name&user=user_name&key=authorization_key
```

## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
}
```

# GET **/api/startup**
SET startup states of outputs by name. 
## Request
``` js
  /api/startup?out1=on&out2=on&out3=on&led=off
```
Where **out1**, **out2**, **out3**, **led** - outputs name of device
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
}
```

# GET **/api/template**
Return HTML code fragment of webUI
## Request
``` js
  /api/template?name=html_template_name
```
Data for template stored in minimized html file **/html/v/\_html\__template_name_\.min.html**
## Responce (code 200)
template file exists
``` js
  html content
```
## Responce (code 404)
template file not exists
``` js

```

# GET **/api/setup**
Allow to setup automation of device outputs. Each output can be configured to run triggers task
* on/off trigger
  _set output to ON/OFF state in some time of day_
* PWM trigger
_can set output state to ON for some perion and after set output state to OFF for some period_ 
* Termostat trigger
_set output state to ON when some sensor value is smaller then min, and set output state to off when sensor value is larger then max_
* Venting trigger
_set output state to ON when some sensor value is bigger then max, and set output state to off when sensor value is less then min_
* Timeout trigger
_After output state becomes ON - trigger start timeout timer. After timer ellapsed - trigger set output to OFF_

## Get triggers list assigned to output

### Request 
``` js
/api/setup?index=output_index
```
return list of all automation assigned for output

output_index - index ot uouput
### Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
  "items":
  [
    {
      "name":"trigger name",//title of items
      "uid":"number",//trigger uid
      "days":"number", //days of wee when this trigger is active
      "type":"trigger type", // type name of then trigger
      "template":"template_name" //HTML template for display in web UI
      "editingtemplate":"editingtemplate_name" //HTML template to display trigger in edit mode
      ..//different trigger have some other aditional proprties
    },
    {
      "name":"trigger name",//title of items
      "uid":"number",//trigger uid
      "days":"number", //days of wee when this trigger is active
      "type":"trigger type", // type name of then trigger
      "template":"template_name" //HTML template for display in web UI
      "editingtemplate":"editingtemplate_name" //HTML template to display trigger in edit mode
      ..//different trigger have some other aditional proprties
    }
    ..
    {
      "name":"trigger name",//title of items
      "uid":"number",//trigger uid
      "days":"number", //days of wee when this trigger is active
      "type":"trigger type", // type name of then trigger
      "template":"template_name" //HTML template for display in web UI
      "editingtemplate":"editingtemplate_name" //HTML template to display trigger in edit mode
      ..//different trigger have some other aditional proprties
    }
  ]
}
```
### Responce (code 404)
index of output out of bounds
``` js
```
### Request 
``` js
/api/setup?delete=trigger_uid&switch=output_index
```
delete froigger from device
### Responce (code 200)
trigger deleted from flash
``` js
```

### Responce (code 404)
trigger not found or output with index not exists
``` js
```


# GET **/api/switches**
Return list of all outputs and their states
## Request
``` js
  /api/switches
```
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
  "items":
  [
    {
      "name":"out1",//title of items
      "type":"switch",//type of item
      "state":"ON", //current state "ON" or "OFF"
      "index":"1", // index of item in array
      "visual":"switch" //HTML template for display in web UI
    },
    {
      "name":"out2",//title of items
      "type":"switch",//type of item
      "state":"ON", //current state "ON" or "OFF"
      "index":"1", // index of item in array
      "visual":"switch" //HTML template for display in web UI
    },
    ..
    {
      "name":"outX",//title of items
      "type":"switch",//type of item
      "state":"ON", //current state "ON" or "OFF"
      "index":"1", // index of item in array
      "visual":"switch" //HTML template for display in web UI
    },
  ]
}
```

# POST **/api/switches**
Set state of output to ON/OFF
## Request
``` js
/api/switches?index=item_index&state=items_state
```
item_index - index of output in outputs list

items_state - new state of output. When On - output turns on else output turns off

## Responce (code 200)
``` js
items_state
```
## Responce (code 404)
``` js
item not fpund
```

# GET **/api/menu**
return list of webUI menu items in json format
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt",//long
  "items":
  [
    {"name":"menu item 1 title", "href":"menu item 1 url", "target":"menu item 1 tager"},
    {"name":"menu item 2 title", "href":"menu item 2 url", "target":"menu item 2 tager"},
    ..
    {"name":"menu item X title", "href":"menu item X url", "target":"menu item X tager"}
  ]
}
```
# GET **/api/restart**
reboot the device
## Responce (code 200)
``` js
{
  "systime":"HH:mm:ss",//string
  "uptime":"millis from device strt"//long
}
```
