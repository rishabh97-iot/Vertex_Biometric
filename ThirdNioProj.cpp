#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <BluetoothSerial.h>
#include <WiFi.h>          
#include <HTTPClient.h>    
#include <WiFiClientSecure.h> 
#include <Preferences.h>      // Handles permanent Flash Storage mapping securely

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
HardwareSerial serialPort(2); 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);
BluetoothSerial SerialBT;
Preferences preferences;      // Flash storage namespace wrapper

// Dynamic Configuration Variables
String wifi_ssid = "";
String wifi_password = "";

//const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbxy1k1_DOvYjGEJKbDk3ZtwYShstn--d55tv-3fyoPop0vd6aBev1MOmKsPvqaJwlQFRw/exec";      
//const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbzq85opg00qHhSW_V7Ytsntvz6hDbky0nkzzN98wcdj-PdTVW25Ho2-aa4AlXlQB4aPrA/exec";

const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbxD2KX3L_-IC2GXFLBjF9kaCN7WnpGblKUdgMHP06YPwTaHthhPz4sYAR4naEqcN7VYNA/exec"; // Updated Google Script URL


enum SystemMode { MODE_VERIFICATION, MODE_ENROLLMENT };
SystemMode currentMode = MODE_VERIFICATION;
uint16_t targetEnrollID = 0;
bool nextEnrollStepReady = false;

// Function Prototypes
void handleIncomingBluetooth();
void runVerificationRoutine();
bool runEnrollmentRoutine(uint16_t slotID);
void updateOLEDStatus(String line1, String line2, uint8_t textSize = 1);
void initializeWiFi();                               
void sendCloudAttendanceLog(uint16_t verifiedCallID , String EmpDetails); 
void loadSavedCredentials();  
String getLocalEmployeeName(uint16_t id); // NEW: Local Database Lookup Engine

// Bitmap configuration variables...
const unsigned char epd_bitmap_Vertex_Groups [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xfc, 0x01, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x81, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xf0, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x83, 0xe3, 0x0f, 0x3c, 0x23, 0xe2, 0x11, 0xf3, 0x3f, 0xf8, 0x67, 0xc3, 0xfc, 0x1e, 0x1e, 0x63, 
	0xc7, 0xf7, 0x8f, 0x9c, 0x38, 0xe7, 0x39, 0xc7, 0x9f, 0xfc, 0x71, 0xc3, 0xfe, 0x3f, 0x3c, 0xf3, 
	0xe7, 0xf7, 0x8f, 0xfe, 0x78, 0xef, 0x3d, 0x8f, 0xcf, 0xfc, 0xf1, 0xe7, 0xfe, 0x3f, 0xb8, 0xfb, 
	0xe3, 0xef, 0x8f, 0xfe, 0x78, 0xff, 0x3f, 0x8f, 0xc7, 0xfc, 0x71, 0xe7, 0xfe, 0x3f, 0xf8, 0xff, 
	0xe3, 0xef, 0x8f, 0xfe, 0x78, 0xff, 0x3f, 0x1f, 0xc7, 0xfc, 0xf1, 0xe7, 0xfe, 0x3f, 0xf8, 0x7f, 
	0xf1, 0xff, 0x8e, 0xfe, 0x71, 0xff, 0x3f, 0x1f, 0xc3, 0xfc, 0x71, 0xe7, 0xfe, 0x3f, 0xfc, 0x0f, 
	0xf1, 0xdf, 0x86, 0xfe, 0x03, 0xff, 0x3f, 0x1f, 0xc3, 0xfc, 0x47, 0xe7, 0xfe, 0x3f, 0xfe, 0x03, 
	0xf8, 0xff, 0x8f, 0xfe, 0x63, 0xff, 0x3f, 0x1f, 0xc7, 0xfc, 0x7f, 0xe7, 0xfe, 0x3f, 0xff, 0xc1, 
	0xf8, 0xff, 0x8f, 0xfe, 0x71, 0xff, 0x3f, 0x1f, 0xc7, 0xfc, 0xff, 0xe7, 0xfe, 0x3f, 0xff, 0xf1, 
	0xfc, 0x7f, 0x8f, 0xfe, 0x70, 0xff, 0x3f, 0x8f, 0xc7, 0xfc, 0xff, 0xe7, 0xfe, 0x3f, 0xfb, 0xf1, 
	0xfc, 0x7f, 0x8f, 0xde, 0x78, 0x7f, 0x3f, 0xcf, 0xcf, 0xfc, 0xff, 0xe7, 0xef, 0x3f, 0x7b, 0xf1, 
	0xfc, 0x7f, 0x87, 0x9c, 0x3c, 0x3e, 0x1f, 0xe7, 0x9f, 0xf8, 0x7f, 0xc3, 0xcf, 0x1e, 0xf8, 0xf3, 
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0x0f, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x8f, 0x88, 0x59, 0xcc, 0x59, 0x88, 0x6c, 0x43, 0xb3, 0x66, 0x69, 0xc9, 0x1f, 0xb1, 0xe6, 0x21, 
	0x8d, 0xd8, 0xca, 0x6c, 0xcd, 0xc8, 0x66, 0x43, 0x39, 0x72, 0xcd, 0xc9, 0x96, 0xb3, 0x37, 0x21, 
	0x8c, 0xdc, 0xe3, 0x0d, 0x8d, 0xe8, 0x67, 0x03, 0x38, 0x78, 0xc6, 0xc1, 0x86, 0x37, 0x33, 0x81, 
	0x8f, 0x8c, 0x73, 0xcd, 0x8c, 0x70, 0x63, 0xc3, 0x2e, 0x5c, 0xc6, 0xe1, 0xc6, 0x36, 0x39, 0xe1, 
	0x8e, 0x2e, 0x38, 0xed, 0x8d, 0x38, 0x61, 0xc3, 0x27, 0x4e, 0xc6, 0x73, 0xc6, 0x36, 0x30, 0xe1, 
	0x8c, 0x26, 0x9e, 0x6d, 0x8d, 0x18, 0x64, 0xc3, 0x27, 0x46, 0xce, 0x66, 0xe6, 0x33, 0x30, 0x61, 
	0x8e, 0x66, 0xda, 0x4c, 0xd9, 0x18, 0x6c, 0x43, 0xa3, 0x46, 0x6c, 0x24, 0xe6, 0x33, 0x66, 0x21, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
void drawStandardLogo();
void testfillroundrect(void);

void setup() {
    Serial.begin(115200);
    delay(500);
    display.setRotation(2);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("OLED initialization failed!")); 
        for(;;); 
    }
    display.clearDisplay(); 
    testfillroundrect(); 
    display.drawBitmap(0, 0, epd_bitmap_Vertex_Groups, 128, 64, SSD1306_WHITE); 
    display.display(); 
    drawStandardLogo(); 
    delay(1000); 
    
    updateOLEDStatus("VERTEX BIOMETRIC", "Loading memory..."); 

    // Open NVS and dynamically fetch permanently saved credentials
    loadSavedCredentials();
    initializeWiFi();

    updateOLEDStatus("Saved Wifi SSID", wifi_ssid);
    delay(1000);

    Serial.println(("The Wifi SSID is: " + wifi_ssid));

    finger.begin(57600); 
    if (finger.verifyPassword()) { 
        Serial.println(F("Found R307 fingerprint sensor!")); 
    } else {
        updateOLEDStatus("HARDWARE ERROR", "Sensor not found."); 
        for(;;); 
    }

    SerialBT.begin("Vertex_Punch_Node"); 
    updateOLEDStatus("SYSTEM READY", "Mode: Verification"); 
}

void loop() {
    handleIncomingBluetooth(); 
    
    if (currentMode == MODE_VERIFICATION) 
    { 
        runVerificationRoutine(); 
    }
    else if (currentMode == MODE_ENROLLMENT && nextEnrollStepReady) 
    { 
        if (runEnrollmentRoutine(targetEnrollID)) { 
            currentMode = MODE_VERIFICATION; 
            nextEnrollStepReady = false; 
            updateOLEDStatus("ENROLL SUCCESS", "Mode: Verification"); 
        } else {
            updateOLEDStatus("ENROLL FAILED", "Try processing again"); 
            delay(2000); 
            updateOLEDStatus("MODE: ENROLLMENT", "Awaiting finger placement..."); 
        }
    }
    delay(50); 
}

void loadSavedCredentials() {
    preferences.begin("wifi-gate", true); // Read-only mode
    wifi_ssid = preferences.getString("ssid", "NONE");
    wifi_password = preferences.getString("pass", "NONE");
    preferences.end();
}

void initializeWiFi() {
    if (wifi_ssid == "NONE" || wifi_ssid == "") return;
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(500);
        attempts++;
    }
}

// NEW: Local Database Engine to pull Employee Meta String (e.g., "VAS32 | Ashish")
String getLocalEmployeeName(uint16_t id) 
{
    preferences.begin("emp-db", true); // Open Employee table in Read-Only mode
    String key = String(id);
    String record = preferences.getString(key.c_str(), "UNKNOWN ID");
    preferences.end();
    Serial.println("Local Lookup: ID #" + String(id) + " => " + record);
    return record;
}
void handleIncomingBluetooth() {
        if (SerialBT.available()) 
        {
                String rxData = SerialBT.readStringUntil('\n');
                rxData.trim();
                if (rxData.length() == 0) return;
                Serial.print("BT Packet Received: ");
                Serial.println(rxData);
                // ─── OPTION 1: WI-FI PROVISIONING ───
                if (rxData.startsWith("WIFI:")) 
                {
                    String payload = rxData.substring(5);
                    int commaIndex = payload.indexOf(',');
                    if (commaIndex != -1)
                    {
                        String newSSID = payload.substring(0, commaIndex);
                        String newPASS = payload.substring(commaIndex + 1);
                        preferences.begin("wifi-gate", false);
                        preferences.putString("ssid", newSSID);
                        preferences.putString("pass", newPASS);
                        preferences.end();
                        updateOLEDStatus("WIFI SAVED", newSSID);
                        delay(1000);
                        SerialBT.println("WIFI_SAVED:REBOOTING");
                        updateOLEDStatus("SETTINGS SAVED", "Rebooting node...");
                        delay(2000);
                        ESP.restart();
                    }
                }
                // ─── OPTION 2: ENROLL DURATION WITH META DATA EXTEXT EXTRACTION ───
                // Expecting format over BT: "ENROLL:CallID,EmpID,FirstName"
                else if (rxData.startsWith("ENROLL:")) 
                {
                    String payload = rxData.substring(7);
                    int firstComma = payload.indexOf(',');
                    int secondComma = payload.indexOf(',', firstComma + 1);
                    int thirdComma = payload.indexOf(',', secondComma + 1);
                    if (firstComma != -1 && secondComma != -1 && thirdComma != -1) 
                    {
                        String idStr = payload.substring(0, firstComma);
                        String empID = payload.substring(firstComma + 1, secondComma);
                        String name = payload.substring(secondComma + 1, thirdComma);
                        String Area = payload.substring(thirdComma + 1);
                        targetEnrollID = (uint16_t)idStr.toInt();
                        if (targetEnrollID > 0 && targetEnrollID <= 1000) 
                        {
                            // Create formatted metadata block string layout
                            String metaData = empID + " | " + name + " | " + Area;
                            // Store employee meta data inside permanent NVS flash row mapping keys
                            preferences.begin("emp-db", false); // Open vault in Read/Write mode
                            preferences.putString(idStr.c_str(), metaData);
                            preferences.end();
                            currentMode = MODE_ENROLLMENT;
                            nextEnrollStepReady = true;
                            Serial.print("Local mapping successful. Assigned Hardware Slot: #");
                            Serial.println(targetEnrollID);
                        }
                    }
                }
                else if (rxData == "MODE:VERIFY") 
                {
                    currentMode = MODE_VERIFICATION;
                    nextEnrollStepReady = false;
                    updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints...");
                }
                else if (rxData == "RESET:ALL") 
                {
                    updateOLEDStatus("SYSTEM RESET", "Clearing database...");
                    finger.emptyDatabase();
                    preferences.begin("emp-db", false);
                    preferences.clear(); // Wipes the local NVS employee table completely clean
                    preferences.end();
                    delay(2000);
                    updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints...");
                }
                else if (rxData.startsWith("DELETE:")) { //
                    String idSlice = rxData.substring(7); //
                    uint16_t idToDelete = (uint16_t)idSlice.toInt(); //
                    if (idToDelete > 0 && idToDelete <= 1000) { //
                    String processingMsg = "Deleting ID #" + String(idToDelete); //
                    updateOLEDStatus("PROCESSING...", processingMsg); //
                    Serial.print("Attempting to delete template index: #"); //
                    Serial.println(idToDelete); //
                    uint8_t p = finger.deleteModel(idToDelete); //
                    if (p == FINGERPRINT_OK) { //
                    Serial.println("Template deleted successfully!"); //
                    String successMsg = "ID #" + String(idToDelete) + " Removed"; //
                    updateOLEDStatus("DELETE SUCCESS", successMsg); //
                    SerialBT.println("DEL_OK:" + String(idToDelete)); //
                    } else {
                    Serial.print("Deletion failed. Error Code: 0x"); //
                    Serial.println(p, HEX); //
                    updateOLEDStatus("DELETE ERROR", "ID not found/error"); //
                    SerialBT.println("DEL_ERR:" + String(idToDelete)); //
                    }
                    } else {
                    updateOLEDStatus("INVALID ID", "Must be 1 to 1000"); //
                    }
                    delay(2500);  //
                    if (currentMode == MODE_VERIFICATION) { //
                    updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints..."); //
                    } else {
                    updateOLEDStatus("MODE: ENROLLMENT", "Awaiting inputs..."); //
                    }
                }
            }
}

void runVerificationRoutine() {
        uint8_t imageResult = finger.getImage();
        if (imageResult != FINGERPRINT_OK) return;
        imageResult = finger.image2Tz();
        if (imageResult != FINGERPRINT_OK) 
        {
            updateOLEDStatus("SCAN MATCH ERROR", "Image was too messy");
            delay(1500);
            updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints...");
            return;
        }
        imageResult = finger.fingerFastSearch();
        if (imageResult == FINGERPRINT_OK) 
        {
            Serial.print("Match Registered! Index: #");
            Serial.println(finger.fingerID);
            // DYNAMIC LOCAL LOOKUP: Read real worker name string directly from internal storage vault
            String employeeDetails = getLocalEmployeeName(finger.fingerID);
            // Output real text details straight onto the OLED panel!
            updateOLEDStatus("ACCESS GRANTED", employeeDetails, 1);
            SerialBT.println("PUNCH_OK:" + String(finger.fingerID));
            delay(500); // Flash pacing delay protection buffer
            sendCloudAttendanceLog(finger.fingerID , employeeDetails); // Send verified punch to Google Cloud
            delay(2500);
            updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints...");
        } 
        else 
        {
        updateOLEDStatus("UNKNOWN EMPLOYEE", "Access denied.");
        SerialBT.println("PUNCH_UNKNOWN");
        delay(2000);
        updateOLEDStatus("MODE: VERIFICATION", "Scanning for prints...");
        }
}

void sendCloudAttendanceLog(uint16_t verifiedCallID, String EmpDetails) {
    // 1. Temporarily release Bluetooth memory space so the secure connection doesn't crash
    Serial.println("Releasing Bluetooth memory stack allocations...");
    SerialBT.end(); 
    delay(500); 

    // 2. Safe validation check: Auto-reconnect if Wi-Fi dropped offline
    if (WiFi.status() != WL_CONNECTED && wifi_ssid != "NONE") 
    {
        Serial.println("Wi-Fi dropped offline! Attempting dynamic reconnection...");
        WiFi.disconnect();
        WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
        int checkCounter = 0;
        while (WiFi.status() != WL_CONNECTED && checkCounter < 6) {
            delay(500);
            checkCounter++;
        }
    }

    delay(500); // Short buffer delay to ensure Wi-Fi is stable before sending

    // 3. Execute transmission payload directly to your secure Google Sheet
    if (WiFi.status() == WL_CONNECTED) 
    {
        WiFiClientSecure secureClient;
        secureClient.setInsecure(); // Wide open and perfectly safe because Bluetooth is off!
        secureClient.setTimeout(10000); // 10-second timeout for secure connection
      
        HTTPClient http;
        Serial.println("Publishing punch data directly to secure Google Apps Script Endpoint...");
      
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        http.begin(secureClient, GOOGLE_SCRIPT_URL); 
        http.addHeader("Content-Type", "application/json");
        String PunchStatus = ""; // Default status for this example

        //String jsonPayload = "{\"callId\":" + String(verifiedCallID) + ",\"status\":\"" + PunchStatus + "\"}";

               // UPDATED PAYLOAD: Included empDetails string key
        String jsonPayload = "{\"callId\":" + String(verifiedCallID) + ",\"empDetails\":\"" + EmpDetails + "\"}";



        
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode > 0) {
            String responseBody = http.getString();
            Serial.print("Google Cloud Execution Success Code: ");
            Serial.println(httpResponseCode); 
        } else {
            Serial.print("Secure Network Protocol Error Code: ");
            Serial.println(httpResponseCode);
        }
        http.end(); 
    } 
    else 
    {
        Serial.println("Direct transmission failed: Wi-Fi unavailable.");
    }

    delay(500);
    // 4. Turn Classic Bluetooth right back on seamlessly for your smartphone app
    Serial.println("Restoring Classic Bluetooth broadcast stack...");
    SerialBT.begin("Vertex_Punch_Node");
    Serial.println("System fully responsive. Bluetooth listeners active.");
}

bool runEnrollmentRoutine(uint16_t slotID) {
int p = -1;
String promptMsg = "Place finger for #" + String(slotID);
updateOLEDStatus("ENROLLMENT PASS 1", promptMsg);
while (p != FINGERPRINT_OK) {
p = finger.getImage();
if (currentMode != MODE_ENROLLMENT) return false;
}
p = finger.image2Tz(1);
if (p != FINGERPRINT_OK) return false;
p = finger.fingerFastSearch();
if (p == FINGERPRINT_OK) {
String duplicateWarning = "Already saved as ID#" + String(finger.fingerID);
updateOLEDStatus("DUPLICATE FOUND", duplicateWarning);
SerialBT.println("ERR:DUPLICATE:" + String(finger.fingerID));
delay(4000);
return false;
}
updateOLEDStatus("PASS 1 SUCCESS", "Lift finger off lens");
delay(2000);
p = 0;
while (p != FINGERPRINT_NOFINGER) { p = finger.getImage(); }
p = -1;
updateOLEDStatus("ENROLLMENT PASS 2", "Place same finger again");
while (p != FINGERPRINT_OK) {
p = finger.getImage();
if (currentMode != MODE_ENROLLMENT) return false;
}
p = finger.image2Tz(2);
if (p != FINGERPRINT_OK) return false;
p = finger.createModel();
if (p != FINGERPRINT_OK) return false;
p = finger.storeModel(slotID);
if (p == FINGERPRINT_OK) {
SerialBT.println("ENROLL_OK:" + String(slotID));
return true;
}
return false;
}

void updateOLEDStatus(String line1, String line2, uint8_t textSize) {
display.clearDisplay();
display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
display.setTextColor(SSD1306_BLACK);
display.setTextSize(1);
display.setCursor(4, 3);
display.print("VERTEX ATTENDANCE");
display.setTextColor(SSD1306_WHITE);
display.setTextSize(textSize);
display.setCursor(0, 24);
display.print(line1);
display.setCursor(0, 46);
display.print(line2);
display.display();
}

void testfillroundrect(void) {
display.clearDisplay();
for(int16_t i=0; i<display.height()/2-2; i+=2) {
display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, INVERSE);
display.display();
delay(1);
}
delay(2000);
}

void drawStandardLogo() {
display.clearDisplay();
display.drawBitmap(0, 0, epd_bitmap_Vertex_Groups, 128, 64, SSD1306_WHITE);
display.display();
}