#include <OneWire.h>
#include <DallasTemperature.h>

// pH Sensor Configuration
const int phSensorPin = 26;   // Analog pin for pH sensor ('To')
float calibrationOffset = 0.0;  // Adjust based on initial dry reading
float scalingFactor = 0.18;     // Adjust based on calibration

// Flow Sensor Configuration
const int flowSensorPin = 15;  
volatile int pulseCount = 0;    

// Variables for flow calculation
unsigned long previousMillis = 0;
const unsigned long interval = 1000;  
float flowRate = 0;

// DS18B20 Configuration
#define DS18B20 4  // Connect to GPIO4 pin
OneWire ourWire(DS18B20);
DallasTemperature sensor(&ourWire);

// Set the calibration offset for DS18B20
const float calibrationOffsetC = 127.0;  // Adjust this value as needed
const float calibrationOffsetF = calibrationOffsetC * 9.0 / 5.0;  // Convert offset for Fahrenheit

// Interrupt function for flow sensor
void IRAM_ATTR countPulses() {
    pulseCount++;
}

void setup() {
    Serial.begin(115200);
    
    // pH Sensor Setup
    delay(1000);  // Allow time for stabilization
    int baselineAnalogValue = analogRead(phSensorPin);
    float baselineVoltage = baselineAnalogValue * (3.3 / 4095);
    calibrationOffset = 7 + ((2.5 - baselineVoltage) / scalingFactor);
    
    // Flow Sensor Setup
    pinMode(flowSensorPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulses, RISING);
    
    // DS18B20 Setup
    sensor.begin();
}

float readPH() {
    int analogValue = analogRead(phSensorPin);           // Read analog value
    float voltage = analogValue * (3.3 / 4095);          // Convert to voltage (0 - 3.3V)
    float phValue = (voltage - 2.5) / scalingFactor + calibrationOffset;  // Calculate pH
    return phValue;
}

void loop() {
    // Read pH value
    float ph = readPH();

    // Flow sensor calculation
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        float calibrationFactor = 4.5;  
        flowRate = (pulseCount / calibrationFactor);
        pulseCount = 0;  // Reset pulse count for the next interval
    }
    
    // Request temperature from DS18B20 sensor
    sensor.requestTemperatures(); 
    
    // Get temperature readings and apply calibration offset
    float tempC = sensor.getTempCByIndex(0) + calibrationOffsetC;
    float tempF = sensor.getTempFByIndex(0) + calibrationOffsetF;
    
    // Print the pH, flow rate, and temperature values to the serial monitor
    Serial.print("pH: ");
    Serial.print(ph);
    Serial.print(" | Flow rate: ");
    Serial.print(flowRate);
    Serial.print(" L/min | Calibrated Celsius temperature: ");
    Serial.print(tempC);
    Serial.print(" | Calibrated Fahrenheit temperature: ");
    Serial.println(tempF);
    
    delay(1000);  // Delay for 1 second before the next loop iteration
}
