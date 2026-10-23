// #include "BluetoothSerial.h"
// #include <Preferences.h>

// // Check if Bluetooth is properly configured in the IDE
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
// #endif

// BluetoothSerial SerialBT;
// Preferences preferences;

// void setup() {
//   Serial.begin(115200);
  
//   // Initialize Bluetooth Classic with your Office Machine's Broadcast Name
//   SerialBT.begin("Office_Biometric_Fix"); 
//   Serial.println("Bluetooth Device Ready to Pair!");

//   // Initialize ESP32 Flash Database Namespace
//   preferences.begin("emp_database", false);
// }

// void loop() {
//   // Check if the mobile app sent data over Bluetooth
//   if (SerialBT.available()) {
//     String incomingData = SerialBT.readStringUntil('\n');
//     incomingData.trim(); // Clean up whitespace/carriage returns

//     // Expected incoming format protocol: "ID,NAME,EMP_CODE"
//     // Example from App: "5,John Doe,EMP402"
    
//     int firstComma = incomingData.indexOf(',');
//     int secondComma = incomingData.indexOf(',', firstComma + 1);

//     if (firstComma != -1 && secondComma != -1) {
//       String idStr = incomingData.substring(0, firstComma);
//       String nameStr = incomingData.substring(firstComma + 1, secondComma);
//       String codeStr = incomingData.substring(secondComma + 1);

//       // Create unique keys for flash storage based on the Fingerprint ID Slot
//       String nameKey = "n" + idStr;  // e.g., "n5"
//       String codeKey = "c" + idStr;  // e.g., "c5"

//       // Store permanently on ESP32 Flash Memory
//       preferences.putString(nameKey.c_str(), nameStr);
//       preferences.putString(codeKey.c_str(), codeStr);

//       // Send Confirmation back to Mobile App Screen
//       SerialBT.println("SUCCESS: Registered " + nameStr + " to ID Slot " + idStr);
//       Serial.println("Stored: ID " + idStr + " -> " + nameStr + " (" + codeStr + ")");
//     } else {
//       SerialBT.println("ERROR: Use format 'ID,Name,EmployeeCode'");
//     }
//   }
//   delay(20);
// }
