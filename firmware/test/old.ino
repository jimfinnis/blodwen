volatile long count1,count2;
volatile long servo1,servo2;

ISR(INT0_vect){
    if(PIND & 4)// rising edge?
        count1=micros();
    else
        servo1=micros()-count1;
}
ISR(INT1_vect){
    if(PIND & 8)
        count2=micros();
    else
        servo2=micros()-count2;
}

void setup()
{
    Serial.begin(9600);
    pinMode(2,INPUT);
    pinMode(3,INPUT);
    pinMode(4,INPUT);
    cli();
    // use normal standard IRQs for pins 2 and 3
    EICRA |= (1<<ISC00)|(1<<ISC10);   //int0/1 on any change
    EIMSK |= (1<<INT0)|(1<<INT1); //int0/1 enabled
    // but on pin 4 etc. we need to use the change bits
    
    sei();
}

void loop()
{
    delay(100);
    Serial.print(servo1);Serial.print(" ");Serial.println(servo2);
}
