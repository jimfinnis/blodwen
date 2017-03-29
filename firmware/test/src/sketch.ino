#include "rc.h"


RCReceiver rc;

void setup(){
    Serial.begin(9600);
    rc.init();
    pinMode(7,OUTPUT);
    rc.setCalib(0,1940,1000)
          .setCalib(1,1738,940)
          .setCalib(2,910,1800);
    
          
}

void loop(){
    rc.update();
    for(int i=0;i<3;i++){
        Serial.print(rc.get(i));
        Serial.print("(");
        Serial.print(rc.raw(i));
        Serial.print(") ");
    }
    Serial.println(rc.ready());
    
    digitalWrite(7,rc.ready()?HIGH:LOW);
}
