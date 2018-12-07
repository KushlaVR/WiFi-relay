/*
   -- 3-way WIFI reley --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 2.3.3 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.1.1 or later version;
     - for iOS 1.2.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// определение режима соединения и подключение библиотеки RemoteXY 
#define REMOTEXY_MODE__ESP8266WIFI_LIB
#include <ESP8266WiFi.h>

#include <RemoteXY.h>

// настройки соединения 
#define REMOTEXY_WIFI_SSID "Prosvity19_98"
#define REMOTEXY_WIFI_PASSWORD "29111929"
#define REMOTEXY_SERVER_PORT 6377
// конфигурация интерфейса   
#pragma pack(push, 1) 
uint8_t RemoteXY_CONF[] = 
  { 255,3,0,0,0,80,0,8,31,0,
  129,0,4,4,18,6,15,51,32,45,
  32,119,97,121,32,87,73,70,73,32,
  114,101,108,101,121,0,2,0,5,25,
  27,14,2,26,31,31,79,78,0,79,
  70,70,0,2,0,37,25,27,14,2,
  26,31,31,79,78,0,79,70,70,0,
  2,0,69,25,27,14,2,26,31,31,
  79,78,0,79,70,70,0 }; 
   
// структура определяет все переменные вашего интерфейса управления  
struct { 

    // input variable
  uint8_t switch_1; // =1 если переключатель включен и =0 если отключен 
  uint8_t switch_2; // =1 если переключатель включен и =0 если отключен 
  uint8_t switch_3; // =1 если переключатель включен и =0 если отключен 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY; 
#pragma pack(pop) 

///////////////////////////////////////////// 
//           END RemoteXY include          // 
///////////////////////////////////////////// 

#define PIN_SWITCH_1 D5
#define PIN_SWITCH_2 D6
#define PIN_SWITCH_3 D7


void setup()  
{ 
  RemoteXY_Init ();  
   
  pinMode (PIN_SWITCH_1, OUTPUT);
  pinMode (PIN_SWITCH_2, OUTPUT);
  pinMode (PIN_SWITCH_3, OUTPUT);
   
  // TODO you setup code 
   
} 

void loop()  
{  
  RemoteXY_Handler (); 
   
  digitalWrite(PIN_SWITCH_1, (RemoteXY.switch_1==0)?LOW:HIGH);
  digitalWrite(PIN_SWITCH_2, (RemoteXY.switch_2==0)?LOW:HIGH);
  digitalWrite(PIN_SWITCH_3, (RemoteXY.switch_3==0)?LOW:HIGH);
   
  // TODO you loop code 
  // используйте структуру RemoteXY для передачи данных 


}
