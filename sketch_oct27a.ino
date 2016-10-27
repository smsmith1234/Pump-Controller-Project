//
#define PRESSURE_DATA_PIN 3
#define PUMP_RUN_PIN 9
#define PUMP_TEST_DELAY 5000
#define MIN_PRESSURE 10
#define ONE_WIRE_BUS 3 // Pin where dallas sensor is connected 

#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>

uint64_t PRESSURE_TEST_INTERVAL = 60000;
uint64_t timeIdle = 0;
int overTempCount = 0;
float maxPressure = 0;
float pressure = 0;
float HPCutOff = 0;
float temperature = 0;
float lastTemperature = 0;

bool highPressureError = false;
bool lowPressureError = false;
bool overTempError = false;

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.

  float GetMaxPressure(int inputPin)
  {
    int inputValue;
    float PSI;
    digitalWrite(PUMP_RUN_PIN, HIGH);
    delay(5000);
    digitalWrite(PUMP_RUN_PIN, LOW);
    delay(5000);
    inputValue = analogRead(inputPin);
    PSI = 15.367/(inputValue - 102);
    return PSI;
  }

  float GetPressure(int inputPin)
  {
    int inputValue;
    float PSI;
    inputValue = analogRead(inputPin);
    PSI = 15.367/(inputValue - 102);
    return PSI;
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
    return temp;
   }

void setup()  
{ 
  // Startup up the OneWire library  
  sensors.begin();
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
  // set up pins
  pinMode(PRESSURE_DATA_PIN, INPUT);
  pinMode(PUMP_RUN_PIN, OUTPUT);
  maxPressure = GetMaxPressure(PRESSURE_DATA_PIN);
  HPCutOff = maxPressure *.9;  
}

void loop()     
{     
    if (highPressureError == false && lowPressureError == false && overTempError == false){
        pressure = GetPressure(PRESSURE_DATA_PIN);
        temperature = GetTemperature(PRESSURE_DATA_PIN);
        if(digitalRead(PUMP_RUN_PIN == false)){
            timeIdle++;
            }
        else{
            if(temperature <= lastTemperature){
                overTempCount++;
                }
            else{
                overTempCount--; 
                }
        }
        switch (pressure){
            case > maxPressure:
                digitalWrite(PUMP_RUN_PIN, LOW);
                highPressureError = true;
                break;
            case < MIN_PRESSURE:
                digitalWrite(PUMP_RUN_PIN, LOW);
                lowPressureError = true;
                break;
            case > HPCutOff:
                delay(3000);
                digitalWrite(PUMP_RUN_PIN, LOW);
                break;
            case < LPTurnOn:
                digitalWrite(PUMP_RUN_PIN, HIGH);
                delay(3000);
                break;
            default:
                break;
        }
    
        if(overTempCount > 5){
            digitalWrite(PUMP_RUN_PIN, LOW);
            overTempError = true;
            }
        elseif(overTempCount < 0){
            overTempCount = 0;
            }
        delay(1000);
 
        if(timeIdle >= PRESSURE_TEST_INTERVAL){
            maxPressure = getMaxPressure;
            HPCutOff = maxPressure * .9;
            }
  
    else{
    digitalWrite(PUMP_RUN_PIN, LOW)
    sendErrorMessage();
    }
}

/*--------------------------------------*/

