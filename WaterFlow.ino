const byte sensorInterrupt = 0;  // 0 = digital pin 2 (interrupt)
const byte FLOWMETRE       = 2;  // pin to yellow cable flowmetre
                           // black cable to ground, red to +5V 
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.

const byte RELE = 8; // rele pin 8 (digital)  coneted to S, + to +5V and - to ground
                     // Common to ground, NO normally open, NC normally closed

float calibrationFactor = 4.5;

// A variable should be declared volatile whenever its value can be changed by something 
// beyond the control of the code section in which it appears, such as a concurrently
// executing thread. In the Arduino, the only place that this is likely to occur is 
// in sections of code associated with interrupts, called an interrupt service routine. 

volatile byte pulseCount; // volatile due to modification by any process, ex.interrupt

float flowRate;
unsigned int flowMilliLitres;
unsigned long timelapsed;
unsigned long totalMilliLitres;
unsigned long currentTime;
unsigned long oldTime;
unsigned long TIME_INTERVAL = 3;
bool flag = false;


//Interrupt Service Routine
void pulse_counter()
{
  // Increment the pulse counter
  pulseCount++;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(RELE,OUTPUT);
  digitalWrite(RELE, LOW);

  Serial.begin(9600); // for serial connection reporting values

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  timelapsed        = 0;
   
  /* The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
   Configured to trigger on a FALLING state change (transition from HIGH
   state to LOW state), pulse_counter is the subroutine
  */ 

  attachInterrupt(sensorInterrupt, pulse_counter, FALLING);
  
}

void loop() {
  currentTime = millis();
  // Only processing counters once per second 
  if(unsigned long)(currentTime - oldTime) > 1000){
    
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.

    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    timelapsed += (currentTime - oldTime)/1000;

    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    
    oldTime = currentTime;

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    
    flowMilliLitres = (flowRate / 60) * 1000;


    if (flowMilliLitres !=0) 
    { 
      if (timelapsed > TIME_INTERVAL){
      //digitalWrite(LED_BUILTIN,HIGH);
      flag=true;
      //digitalWrite(RELE, HIGH); // rele high
      //delay(1000);
      }
     }
     else 
     {
      //digitalWrite(LED_BUILTIN,LOW);
      //digitalWrite(RELE, LOW); // rele low
      timelapsed=0;
      }
    
    if (flag==true){
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(RELE, HIGH);
    }
    // Add the millilitres passed in this second to the cumulative total

    totalMilliLitres += flowMilliLitres;
    
    //unsigned int frac;
    //delay(1000);

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.print("L");
    Serial.print("\n");
    Serial.print(timelapsed); 
    Serial.print(" s\n");

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulse_counter, FALLING);
  }

}





