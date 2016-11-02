//

// Enable debug prints
#define MY_DEBUG

//Enable selected radio
#define MY_RADIO_NRF24

#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define PRESSURE_DATA_PIN 3
#define PUMP_RUN_PIN 9
#define MIN_PRESSURE 10
#define LP_TURN_ON 20
#define ONE_WIRE_BUS 3 // Pin where dallas sensor is connected 

uint32_t PRESSURE_TEST_INTERVAL = 43200000;  // 12 hours
uint32_t timeIdle = 0;
int overTempCount = 0;
int maxPressure = 0;
int pressure = 0;
int HPCutOff = 0;
float temperature = 0;
float lastTemperature = 0;
int status = 0;

int GetMaxPressure(int);
int Getpressure(int);
float GetTemperature(int);

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

void setup()  
{ 
  // Startup up the OneWire library  
  sensors.begin();
  Serial.begin(115200);
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
  // set up pins
  pinMode(PRESSURE_DATA_PIN, INPUT);
  pinMode(PUMP_RUN_PIN, OUTPUT);
  timeIdle = PRESSURE_TEST_INTERVAL;  // Trigger max pressure test
}

void loop()     
{
if(status < 3){
    // Set max pressure?
    if(timeIdle >= PRESSURE_TEST_INTERVAL){  // Pump has been idle for a while - get new pressures (max and HP Cutoff)
        maxPressure = GetMaxPressure(PRESSURE_DATA_PIN);
        HPCutOff = static_cast<int>(static_cast<float>(maxPressure) * .9);
        #ifdef MY_DEBUG
        Serial.print("Max Pressure: ");
        Serial.println(maxPressure);
        Serial.print("HP Cutoff: ");
        Serial.println(HPCutOff);
        #endif    
        }
    pressure = GetPressure(PRESSURE_DATA_PIN);
    temperature = GetTemperature(PRESSURE_DATA_PIN);
    if(status == 0){  // Pump is off - increment timeIdle
        timeIdle++;
        #ifdef MY_DEBUG
        Serial.print("Time idle: ");
        Serial.println(timeIdle);
        #endif
        }
    else{
        if(temperature <= lastTemperature){  // Pump is on - track temperature deviation
            overTempCount++;
            }
        else{
            overTempCount--; 
            }
        #ifdef MY_DEBUG
        Serial.print("Overtemp Count: ");
        Serial.println(overTempCount);
        #endif
        }
    // Pressure testing section

   if (pressure > HPCutOff){  // Pump is above high pressure cutoff pressure - turn off pump
        delay(3000);  // Let pressure build a bit
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 0;  // Pump is standby
        }
    else if (pressure < LP_TURN_ON){  // Pump pressure is below turn on pressure - turn on pump
        digitalWrite(PUMP_RUN_PIN, HIGH);
        delay(10000);  // Let pump stabilize        
        status = 1;  // Pump is commanded on
        }
    else if (pressure > LP_TURN_ON && pressure < HPCutOff){  // Pump pressure is normal - let it ride
        status = 2;  // Pump is in operation
        }
    else if (pressure > maxPressure){  // Pump is in overpressure condition - turn pump off and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 3;  // Overpressure error
        }
    else if (pressure < MIN_PRESSURE){  // Pump is in underpressure condition - turn pump off and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 4;  //Underpressure error
        }
    else{  // Unable to determine state of pump operation - turn off pump and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 5;
        }
        
    // Temperature testing section    
    if(overTempCount > 5){  // Temperature is rising - turn off pump and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 6;
       }
    if(overTempCount < 0){  // Temperature is decreasing - limit to maintain rising temp sensitivity
        overTempCount = 0;
        }
        
    // Wait a second   
    delay(1000);  // Minimum time between pressure samples
}

else{  // Something is wrong - turn pump off and send error message
    digitalWrite(PUMP_RUN_PIN, LOW);
    delay(60000);
    //sendErrorMessage();
    }
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
    PSI = 15.367/(inputValue - 102);
    #ifdef MY_DEBUG
    Serial.print("Max Pressure: ");
    Serial.println(static_cast<int>(PSI));
    #endif
    return static_cast<int>(PSI);
  }

int GetPressure(int inputPin)
  {
    int inputValue;
    float PSI;
    inputValue = analogRead(inputPin);
    PSI = 15.367/(inputValue - 102);
    #ifdef MY_DEBUG
    Serial.print("Pressure: ");
    Serial.println(static_cast<int>(PSI));
    #endif     
    return static_cast<int>(PSI);
   }    

float GetTemperature(int inputPin)
  {
    // Fetch temperatures from Dallas sensors  
    sensors.requestTemperatures();
    // query conversion time and delay until conversion completed  
    int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
    delay(conversionTime);
    // Fetch and round temperature to one decimal    
    float temp = static_cast<float>(static_cast<int>(sensors.getTempFByIndex(0)) * 10.) / 10.;
    #ifdef MY_DEBUG
    Serial.print("Temperature: ");
    Serial.println(temp);
    #endif
    return temp;
   }
