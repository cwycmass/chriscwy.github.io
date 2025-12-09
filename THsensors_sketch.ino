#include <ArduinoBLE.h>
#include <Arduino_BMI270_BMM150.h> // IMU: Accelerometer, Gyroscope, Magnetometer
#include <Arduino_HS300x.h>       // Environment: Temperature and Humidity

// --- 1. Define UUIDs ---
// Service UUID provided by the user
#define SENSE_SERVICE_UUID     "74e39e63-a1fb-446f-a700-6d9fa8c8c858" 

// Dedicated Characteristic for the combined text message
#define TEXT_CHAR_UUID         "74e39e63-a1fb-446f-a701-6d9fa8c8c858" 

// --- 2. Create BLE Objects ---
BLEService senseService(SENSE_SERVICE_UUID);

// Characteristic for the combined text string. 
BLECharacteristic textMessageCharacteristic(TEXT_CHAR_UUID, BLERead | BLENotify, 100); 

// --- 3. Sensor and Timing Variables ---
float tempC, humidity;
float accX, accY, accZ;
unsigned long lastUpdate = 0;
const long updateInterval = 1000; // Update all sensors every 1 second

// Buffer to hold the combined text message
char sensorMessage[100]; 

void setup() {
  // Serial.begin(9600); // <-- REMOVED
  // while (!Serial);    // <-- REMOVED

  // Initialize Sensors
  if (!IMU.begin()) {
    // In a final battery-powered product, you might handle this failure
    // by blinking an LED or entering a low-power mode.
  }
  if (!HS300x.begin()) {
    // Handle sensor failure here
  }

  // Initialize BLE
  if (!BLE.begin()) {
    // Handle BLE startup failure
    while (1); 
  }

  // Set advertised properties
  BLE.setLocalName("Sense_Text_Log");
  BLE.setAdvertisedService(senseService);

  // Add the text characteristic to the service
  senseService.addCharacteristic(textMessageCharacteristic);

  // Add the service to the peripheral
  BLE.addService(senseService);

  // Start advertising
  BLE.advertise();
}

void loop() {
  // Check for a central device (iPhone) connection
  BLEDevice central = BLE.central();

  if (central) {
    
    // While connected...
    while (central.connected()) {
      unsigned long currentMillis = millis();
      
      if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;

        // --- 1. Read Sensor Data ---
        tempC = HS300x.readTemperature();
        humidity = HS300x.readHumidity();

        if (IMU.accelerationAvailable()) {
          IMU.readAcceleration(accX, accY, accZ);
        }

        // --- 2. Format Sensor Data into a Single Text Sentence ---
        sprintf(sensorMessage, 
                "T:%.1fC | H:%.1f%% | A(X:%.2f, Y:%.2f, Z:%.2f)", 
                tempC, 
                humidity, 
                accX, 
                accY, 
                accZ);

        // --- 3. Send the Text String over BLE ---
        textMessageCharacteristic.writeValue(sensorMessage);
      }
    }
  }
}
