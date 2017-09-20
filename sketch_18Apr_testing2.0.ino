/* STATUS: 1 = normal pressure; 2 = low pressure (turn on); 3 = high pressure(turn off); 4 = low pressure alarm; 
						 5 = high pressure alarm  */

// Enable debug prints
#define MY_DEBUG

#define PRESSURE_DATA_PIN 3
#define PUMP_RUN_PIN 9
#define MIN_PRESSURE 10
#define LP_TURN_ON 20

int maxPressure = 0;
int pressure = 0;
int HPCutOff = 0;
int status = 0;

int GetMaxPressure(int);
int GetPressure(int);

void setup()  
{ 
	pinMode(PRESSURE_DATA_PIN, INPUT);
	pinMode(PUMP_RUN_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);

	maxPressure = GetMaxPressure(PRESSURE_DATA_PIN);
	HPCutOff = static_cast<int>(static_cast<float>(maxPressure) * .9);

	#ifdef MY_DEBUG
	Serial.println("Status 0");
	Serial.print("Max pressure: ");
	Serial.println(maxPressure);
	Serial.print("HP cut off pressure: ");
	Serial.println(HPCutOff);
	#endif 
}

void loop()     
{
	pressure = GetPressure(PRESSURE_DATA_PIN);
	
	if (pressure > maxPressure){  
		digitalWrite(PUMP_RUN_PIN, LOW);
		status = 5;  // Overpressure error
		}
	else if (pressure < MIN_PRESSURE){  
		digitalWrite(PUMP_RUN_PIN, LOW);
		status = 4;  // Underpressure error
		}   
	else if (pressure >= HPCutOff && PUMP_RUN_PIN == HIGH){  
		digitalWrite(PUMP_RUN_PIN, LOW);   
		status = 3;  // Pressure above cut off - turn off pump
		}
	else if (pressure <= LP_TURN_ON && PUMP_RUN_PIN == LOW){  // 
		digitalWrite(PUMP_RUN_PIN, HIGH);     
		status = 2;  // Pressure below cut on - turn on pump
		}
	else if (pressure >= LP_TURN_ON && pressure <= HPCutOff){  
		status = 1;  // Pressure good - no change
		}
	else{ 
		digitalWrite(PUMP_RUN_PIN, LOW);
		status = 6;  // Undetermined error
		}
	
	delay(1000);  
	#ifdef MY_DEBUG
	Serial.print("Status: ");
	Serial.println(status);
	Serial.print("Pressure: ");
	Serial.println(pressure);
	#endif
}

/*--------------------------------------*/
int GetMaxPressure(int inputPin)
  {
    int inputValue;
    float PSI;
    digitalWrite(PUMP_RUN_PIN, HIGH);
    delay(5000);
    digitalWrite(PUMP_RUN_PIN, LOW);
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
    #ifdef MY_DEBUG
    Serial.print("Pressure: ");
    Serial.println(static_cast<int>(PSI));
    #endif     
    return static_cast<int>(PSI);
   }    
