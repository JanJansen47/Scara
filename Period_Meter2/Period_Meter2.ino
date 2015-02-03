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
int numberOfSteps = 2000; // the max number of pulses to measure  
volatile int index = numberOfSteps; // index to the stored readings
volatile int results [3000]; // note this is 16 bit value
byte store =0; // store samples on disk
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 

/* ICR interrupt vector */
ISR(TIMER5_CAPT_vect)
{
  TCNT5 = 0; // reset the counter
  if( bitRead(TCCR5B ,ICES5) == true) // wait for rising edge
  { // falling edge was detected
    if(index < numberOfSteps)

    {
      digitalWrite(ledPin, HIGH);
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
        digitalWrite(ledPin, LOW);
      }
    }
  }

  //TCCR5B ^= _BV(ICES5); // toggle bit to trigger on the other edge
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

    Serial.println("====================================================================================");
  Serial.println("Enter the number of steps you want to sample (between 1 and 3000):   "); 
  //Serial.println("====================================================================================");
  while(Serial.available()==0);
  numberOfSteps  = Serial.parseInt();
  if (numberOfSteps > 3000 | numberOfSteps <1) numberOfSteps = 500;
  index = numberOfSteps;
  Serial.println("====================================================================================");
  Serial.println("pulses are sampled while LED is lit");
  Serial.print( precision); // report duration of each tick in microseconds
  Serial.println(" microseconds per tick");
  Serial.print("Number of steps to sample:  "); 
  Serial.println(numberOfSteps);
  Serial.println("====================================================================================");

}

void loop()
{   
  delay(100);
  if(index == numberOfSteps){
    Serial.print(" Store samples on disk (y/n): ");
    while(Serial.available()==0);
    if (Serial.read() =='y' ) { 
      store =1;
    } 
    // print samples based on decision
    if(store ==1)
    {
      Serial.println("Durations in Microseconds are:") ;
      for( int i=0; i < numberOfSteps; i++)
      {
        long duration;
        duration = (results[i] ) * precision  ; // pulse duration in nanoseconds
        // delay(40);  // heb een file probleem??
        Serial.print(i); 
        Serial.print(",");  //input for spreadsheet
        Serial.println(((duration/1000)) );  // duration in microseconds  
      }

      Serial.println("samples available in /desktop/logger"); 
      store =0;   
    }

    // decide for a new batch of samples and clean the old one's 
    Serial.println(""); 
    Serial.println("");
    Serial.println("Take new batch of samples  =>> push key"); 
    while(Serial.available()==0);
    int incomingByte = Serial.read();
    for (int i=0; i < numberOfSteps; i++)  // clean samples
    {
      results[i]=0;
    }
    Serial.println("Ready for samples => start de movement");
    index = 0;
  }  
}







