/*
* Velocity meter for Scara
* Jan Jansen 31-01-2015 
* v01
* v02 01-02-2015 Added stop and clear of the array at the end
 */
const int inputPulsePin = 48; // input pin fixed to internal Timer jj is orgineel 8
const int inputDirectionPin = 2; 
const int ledPin = 13;
const int prescale = 8; // prescale factor (each tick 0.5 us @16MHz)
const byte prescaleBits = B010; // see Table 18-1 or data sheet jja: aanpassen bij andere prescale
// calculate time per counter tick in ns
const long precision = (1000000/(F_CPU/1000)) * prescale ;
const int numberOfEntries = 2000; // the max number of pulses to measure  
const int dir = 2; //See two dimensional results array. used to store direction
const int gateSamplePeriod = 10000; // the sample period in milliseconds
volatile int index = 0; // index to the stored readings
volatile byte gate = 0; // 0 disables capture, 1 enables
volatile int results [numberOfEntries]; // note this is 16 bit value

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 

/* ICR interrupt vector */
ISR(TIMER5_CAPT_vect)
{
  TCNT5 = 0; // reset the counter
  if(gate)
  {
    if( index != 0 || bitRead(TCCR5B ,ICES5) == true) // wait for rising edge
    { // falling edge was detected
      if(index < numberOfEntries)
      {
        if (TIFR5 & 1) {		  // if Timer/Counter n overflow flag =>> forget sample!!!!
          sbi(TIFR5,TOV5);
        }				  // clear Timer/Counter n overflow flag
        else
        { 
          if (digitalRead(inputDirectionPin) == LOW) {
            int absolute = ICR5;
            absolute = -absolute;  // truc 
            results[index] = absolute;
          } // save the input capture value 
          else { 
            results[index] = ICR5;
          } // save the input capture value 
          index++;  
        }
      }
    }
  }
  TCCR5B ^= _BV(ICES5); // toggle bit to trigger on the other edge
}
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(inputPulsePin, INPUT); // ICP pin (Digital input pin for Stepper pulse signal)
  pinMode(inputDirectionPin, INPUT); // Digital input pin for Stepper direction signal
  TCCR5A = 0 ; // Normal counting mode
  TCCR5B = prescaleBits ; // set prescale bits
  TCCR5B |= _BV(ICES5); // enable input capture
  bitSet(TIMSK5,ICIE5); // enable input capture interrupt for timer 5 
  Serial.println("pulses are sampled while LED is lit");
  Serial.print( precision); // report duration of each tick in microseconds
  Serial.println(" microseconds per tick");
  Serial.println("Ready for samples => start de movement"); 
}

void loop()
{
  digitalWrite(ledPin, LOW);
  //delay(gateSamplePeriod);
  
  gate = 1; // enable sampling
  delay(gateSamplePeriod);
  gate = 0; // disable sampling
  digitalWrite(ledPin, HIGH);
  if(index > 0)
  {
    Serial.println("Durations in Microseconds are:") ;
    for( int i=0; i < numberOfEntries; i++)
    {
      long duration;
      duration = (results[i] + results[i+1]) * precision  ; // pulse duration in nanoseconds
      i++;
      delay(20);

      Serial.print(i); Serial.print(",");  //input for spreadsheet
      Serial.println(((duration/2000)) );  // duration in microseconds  jj let op twee maal optellen duraryion
    }
    ;
    Serial.println("samples available in /desktop/logger"); 
    Serial.println("Take new batch of samples  =>> push key + return"); 
    
    delay(1000); 
    while(Serial.available()==0);
    int incomingByte = Serial.read();
    for (int i=0; i < numberOfEntries; i++)
    {results[i]=0;}
    index = 0;
    Serial.println("Ready for samples => start de movement");
    
  }
}




