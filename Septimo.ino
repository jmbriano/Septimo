#include <U8g2lib.h>
#include <WiFi101.h>
#include "SeptimoPrism.h"
#include "ConfigurationServer.h"


#include "config/arduino_secrets.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
SeptimoPrism device;
ConfigurationServer configServer;

int BOOT_INITIAL_BLINK_SECS = 5;
// PIN numbers

// The currently active task is kept in this variable. NO_TASK = no timer running
int currentTask = device.NO_TASK;

// WiFi Configuration
char ssid[] = SECRET_SSID;  
char pass[] = SECRET_PASS;  
int status = WL_IDLE_STATUS;

WiFiServer server(80);

// FUNCTIONS

// Display Functions
void writeBigInDisplay(const char * text) {
   u8g2.clearBuffer();
   u8g2.setFont(u8g2_font_inb21_mr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(1,30,text);
   u8g2.sendBuffer();
}

void writeLineInDisplay(int line, const char * text) {
   u8g2.clearBuffer();
   u8g2.setFont(u8g2_font_profont10_mr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(5,10*line,text);
   u8g2.sendBuffer();
}

void executeTransitionTo(int newTaskID){
  device.turnOffLedForTask(currentTask);
  stopTimmerForTask(currentTask);
  device.turnOnLedForTask(newTaskID);
  startTimmerForTask(newTaskID);
  currentTask = newTaskID;
}

// Time tracking functions
void startTimmerForTask(int taskID){
  Serial.println("Working on "+ String(taskID));
}

void stopTimmerForTask(int taskID){
  Serial.println("Stopped working on "+ String(taskID));
}

// Wifi Functions
void checkOrConnectWifi() {
  
  
  while ( status != WL_CONNECTED) {
    // attempt to connect to WiFi network:
    writeLineInDisplay(1,"IP: Connecting");
    
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println (ssid);
    
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    
    // wait 10 seconds for connection:
    for (int i = 0; i<=3; i++) {
        if ( status == WL_CONNECTED) break;
        writeLineInDisplay(1,"IP: Connecting.");
        delay(1000);
        writeLineInDisplay(1,"IP: Connecting..");
        delay(1000);
        writeLineInDisplay(1,"IP: Connecting...");
        delay(1000);
    }

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    writeLineInDisplay(1,ip2CharArray(ip)); 
  }
}

char* ip2CharArray(IPAddress ip) {
  static char a[20];
  sprintf(a, "IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return a;
}

void setup() {

  u8g2.begin();

  writeBigInDisplay("Septimo");

  device.begin();
  
  Serial.begin(9600);

  device.configureSide(device.NO_TASK,0,"");
  device.configureSide(device.TASK_1,10,"Task 1");
  device.configureSide(device.TASK_2,20,"Task 2");
  device.configureSide(device.TASK_3,30,"Task 3");
  device.configureSide(device.TASK_4,40,"Task 4");
  device.configureSide(device.TASK_5,50,"Task 5");
  device.configureSide(device.TASK_6,60,"Task 6");

  for (int i = 1; i <= BOOT_INITIAL_BLINK_SECS * 5; i++){
    device.turnAllLedsOn();
    delay(100);
    device.turnAllLedsOff();
    delay(100);
  }

  // Connect to WiFi
  checkOrConnectWifi();

  server.begin(); // TODO

  currentTask = device.getUpFacingTask(); // When turned on, the current position must be consider as no active counting.

  int ids[] = {10,20,30,40,50,60};
  String names [] = {"RECAPP", "REX","GTP", "QUOTA", "VSKO", "KANS" };
  configServer.setAvailableProjects(ids, names, 6);
}


void loop() {

  // Connect to WiFi
  checkOrConnectWifi();
  
  // Check if there was any upfacing side change. This function will exit with no action if the same side is facing up
  int newTaskUp = device.getUpfacingTaskAfterTransition(currentTask);

  if (newTaskUp != currentTask) {
    executeTransitionTo(newTaskUp);
  }
  
  configServer.processIncomingServerRequest(server, device, currentTask);

  delay(500);

  //TODO: add button to switch Display to IP.
  
}
