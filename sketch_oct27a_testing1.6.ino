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
#define LED_PIN 5
#define MIN_PRESSURE 10
#define LP_TURN_ON 20
#define ONE_WIRE_BUS 4 // Pin where dallas sensor is connected 

unsigned long PRESSURE_TEST_INTERVAL = 43200000;  // 12 hours
unsigned long timeIdle = PRESSURE_TEST_INTERVAL;
int overTempCount = 0;
int maxPressure = 0;
int pressure = 0;
int HPCutOff = 0;
float temperature = 0;
float lastTemperature = 0;
int status = 0;

int GetMaxPressure(int);
int GetPressure(int);
float GetTemperature(int);
void DisplayStatus(int);

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
  //pinMode(PRESSURE_DATA_PIN, INPUT);
  pinMode(PUMP_RUN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop()     
{
if(status < 3){  // Pump in normal operations
    // Time to check max pressure?
    if(timeIdle >= PRESSURE_TEST_INTERVAL){  // Pump has been idle for a while - get new pressures (max and HP Cutoff)
        maxPressure = GetMaxPressure(PRESSURE_DATA_PIN);
        HPCutOff = static_cast<int>(static_cast<float>(maxPressure) * .9);
        timeIdle=0;
        #ifdef MY_DEBUG
        Serial.print("HP Cutoff: ");
        Serial.println(HPCutOff);
        #endif    
        }
    pressure = GetPressure(PRESSURE_DATA_PIN);
    temperature = 55; //GetTemperature(ONE_WIRE_BUS);
    if(status == 0){  // Pump is off - increment timeIdle
        timeIdle++;
        #ifdef MY_DEBUG
        Serial.print("Time idle: ");
        Serial.println(timeIdle);
        #endif
        }
    else{
        if(temperature < lastTemperature){  // Pump is on - track temperature deviation
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
    
    // Pressure condition testing section
      if (pressure > maxPressure){  // Pump is in overpressure condition - turn pump off and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 3;  // Overpressure error
        }
    else if (pressure < MIN_PRESSURE){  // Pump is in underpressure condition - turn pump off and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 4;  // Underpressure error
        }
   
   else if (pressure >= HPCutOff && pressure < maxPressure){  // Pump is above high pressure cutoff pressure - turn off pump
        delay(3000);  // Let pressure build a bit
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 0;  // Pump is in standby
        }
    else if (pressure <= LP_TURN_ON && pressure > MIN_PRESSURE){  // Pump pressure is below turn on pressure - turn on pump
        digitalWrite(PUMP_RUN_PIN, HIGH);
        delay(10000);  // Let pump stabilize        
        status = 1;  // Pump is commanded on
        }
    else if (pressure >= LP_TURN_ON && pressure < HPCutOff){  // Pump pressure is normal - let it ride
        status = 2;  // Pump is in operation
        }

    else{  // Unable to determine state of pump operation - turn off pump and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 5;  // Undetermined error
        }
        
    // Temperature testing section    
    if(overTempCount > 5 || temperature >= 100){  // Temperature is rising - turn off pump and generate error
        digitalWrite(PUMP_RUN_PIN, LOW);
        status = 6;  // Over temp error
       }
    if(overTempCount < 0){  // Temperature is decreasing - limit to maintain rising temp sensitivity
        overTempCount = 0;
        }
        
    // Wait a second   
    delay(1000);  // Minimum time between pressure samples
    }

else{  // Something is wrong - turn pump off and flash error code
    digitalWrite(PUMP_RUN_PIN, LOW);

    }
  Serial.print("Status: ");
  Serial.println(status);
DisplayStatus(status);
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
    inputValue = analogRead(3);
    PSI = (inputValue - 102) / 15.367;
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

void DisplayStatus(int status)
    {
    switch(status){
        case 0:{
            digitalWrite(LED_PIN, HIGH);
            break;
            }
        case 1:{
            }
        case 2:{  
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            break;
            }
        default:{
            for(int i; i <= 60; i++){
                for(int j = 0; j <= status; j++){
                    digitalWrite(LED_PIN, HIGH);
                    delay(100);
                    digitalWrite(LED_PIN, LOW);
                    delay(300);
                    }
                delay(1000);
                }
            break;
            }
        #ifdef MY_DEBUG
        Serial.print("Status: ");
        Serial.println(status);
        #endif
        }
    } 

