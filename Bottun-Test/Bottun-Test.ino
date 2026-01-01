
void setup(){
    pinMode( 6, INPUT_PULLUP );
    pinMode( 7, INPUT_PULLUP );
    Serial.begin( 9600 );
}

void loop(){
    int value;
    
    value = digitalRead( 7 );
    Serial.println( value );
    value = digitalRead( 6 );
    Serial.println( value );

    delay( 1000 );
}