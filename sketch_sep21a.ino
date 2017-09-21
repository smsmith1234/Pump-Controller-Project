// Super Simple
// STATUS: 1 = normal pressure; 2 = low pressure (turn on); 3 = high pressure (turn off); 4 = low pressure alarm; 5 = high pressure alarm  */

#define MY_DEBUG

#define PRESSURE_DATA_PIN 3
#define PUMP_RUN_PIN 9
#define MIN_PRESSURE 10
#define LP_TURN_ON 20

int maxPressure = 0;
int pressure = 0;
int HPCutOff = 0;
int status = 0;
int led = 13;  
int GetMaxPressure(int);
int GetPressure(int);

void setup()  
{ 
  pinMode(PRESSURE_DATA_PIN, INPUT);
  pinMode(PUMP_RUN_PIN, OUTPUT);
pinMode(led, OUTPUT); 
  maxPressure = GetMaxPressure(PRESSURE_DATA_PIN);
  HPCutOff = static_cast<int>(static_cast<float>(maxPressure) * .9);
  Serial.begin(9600);

  Serial.print("Max pressure: ");
  Serial.print(maxPressure);
  Serial.print("  HP cut off pressure: ");
  Serial.println(HPCutOff);

}

void loop()     
{
  pressure = GetPressure(PRESSURE_DATA_PIN);

  if (pressure > maxPressure){  
    digitalWrite(PUMP_RUN_PIN, HIGH);
    status = 5;  // Overpressure error
  }
  else if (pressure <= MIN_PRESSURE){  
    digitalWrite(PUMP_RUN_PIN, HIGH);
    status = 4;  // Underpressure error
  }   
  else if (pressure >= HPCutOff){  
    digitalWrite(PUMP_RUN_PIN, HIGH);   
    status = 3;  // Pressure above cut off - turn off pump
  }
  else if (pressure <= LP_TURN_ON){  // 
    digitalWrite(PUMP_RUN_PIN, LOW);     
    status = 2;  // Pressure below cut on - turn on pump
  }
  else if (pressure > LP_TURN_ON && pressure < HPCutOff){  
    status = 1;  // Pressure good - no change
  }
  else { 
    digitalWrite(PUMP_RUN_PIN, HIGH);
    status = 6;  // Undetermined error
  }

  //delay(1000);  

  Serial.print("Status: ");
  Serial.print(status);
  Serial.print("  Pressure: ");
  Serial.print(pressure);
    Serial.print("  Pump run pin: ");
  Serial.println(digitalRead(PUMP_RUN_PIN));
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}

/*--------------------------------------*/
int GetMaxPressure(int inputPin)
{
  int inputValue;
  float PSI;
  digitalWrite(PUMP_RUN_PIN, LOW);
  delay(5000);
  digitalWrite(PUMP_RUN_PIN, HIGH);
  delay(5000);
  inputValue = analogRead(inputPin);
  PSI = (inputValue - 102) / 15.367;
  return static_cast<int>(PSI);
}

int GetPressure(int inputPin)
{
  int inputValue;
  float PSI;
  inputValue = analogRead(inputPin);
  PSI = (inputValue - 102) / 15.367;
  return static_cast<int>(PSI);
  //return inputValue;
}    


