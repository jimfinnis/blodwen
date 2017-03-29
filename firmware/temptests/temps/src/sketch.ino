#include <OneWire.h>


#define MAXSENSORS 9

class Temperature {
private:
    OneWire ds;
    unsigned long lastcheck;
    byte devct;
    byte addrs[MAXSENSORS][8];
    
    
public:
    Temperature() : ds(6) {
        devct=0;
        lastcheck=0;
    }
    
    void praddr(int i){
        byte *p = addrs[i];
        for(int j=0;j<8;j++){
            if(p[j]<16)
                Serial.print('0');
            Serial.print(p[j],HEX);
            if(j<7)
                Serial.print(' ');
        }
    }
    
    void init(){
        Serial.println("Searching...");
        for(int i=0;i<MAXSENSORS;i++){
            if(!ds.search(addrs[devct])){
                break;
            }
            Serial.print("device found: ");
            praddr(devct);
            Serial.println("");
            
            if(OneWire::crc8(addrs[devct],7)!=addrs[devct][7]){
                Serial.println("Bad CRC on this device");
            } else if(addrs[devct][0]!=0x10){
                Serial.println("Device is not a DS1820");
            } else
                devct++;
        }
        
        ds.reset_search();
        lastcheck=millis();
    }
    
    void requestTemp(int i){
        ds.reset();
        ds.select(addrs[i]);
        ds.write(0x44,1); // start convo, with parasite
    }
    
    int getDevCount(){
        return devct;
    }
    
    float getTemp(int i){
        ds.reset();
        ds.select(addrs[i]);
        ds.write(0xbe); // read scratch
        
        byte data[9];
        for(int i=0;i<9;i++)
            data[i] = ds.read();
        
        int lo = data[0];
        int hi = data[1];
        int t = (hi<<8)+lo;
        int sign = t & 0x8000;
        if(sign)
            t = (t ^ 0xffff)+1; // 2's complement
        // now this is in 0.5 degree increments
        float temp = t/2;
        if(lo&1)temp+=0.5f;
        if(sign)temp=-temp;
        return temp;
    }
    
    
};


Temperature t;

void setup()
{
    Serial.begin(9600);
    t.init();
    delay(1000);
}


void loop()
{
    for(int i=0;i<t.getDevCount();i++){
        t.requestTemp(i);
        delay(800);
        Serial.print(i);
        Serial.print("  ");
        t.praddr(i);
        Serial.print("  ");
        Serial.print(t.getTemp(i));
        if(t.getTemp(i)>25)
            Serial.println(" ****");
        else
            Serial.println("");
    }
    Serial.println("");
}
