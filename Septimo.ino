#include <U8g2lib.h>
#include <WiFi101.h>


#include "config/arduino_secrets.h"
#include "SeptimoPrism.h"
#include "ConfigurationServer.h"
#include "ToggleConnector.h"


// Components
WiFiServer server(80);
WiFiSSLClient client;
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
SeptimoPrism device;
ConfigurationServer configServer;
ToggleConnector connector(client);

// Configuration parameters
int BOOT_INITIAL_BLINK_SECS = 5;
// WiFi Configuration
char ssid[] = SECRET_SSID;  
char pass[] = SECRET_PASS;  
int status = WL_IDLE_STATUS;

// The currently active task is kept in this variable. NO_TASK = no timer running
int currentTask = device.NO_TASK;

// Information of all projects
int all_project_size = 0;
int all_project_ids[50];
String all_project_names [50];

// LCD functions. TODO: extract to a class
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
// END LCD functions. TODO: extract to a class

/* This functions executes thetransition from one side to the other. This include changing the LEDs and pushing to toggle the new ongoing task
*/
void executeTransitionTo(int newTaskID){
  Serial.println("Swritching from side " + String (currentTask) + " to side " + String(newTaskID));
  device.turnOffLedForTask(currentTask);
  device.turnOnLedForTask(newTaskID);
  stopTimmerForTask(currentTask);
  startTimmerForTask(newTaskID);
  currentTask = newTaskID;
}

// Time tracking functions
void startTimmerForTask(int side){
  connector.startActivity(all_project_ids[side],"Heloo World");
}

void stopTimmerForTask(int side){
  connector.stopActivity();
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


  // TODO, remove this block
  device.configureSide(device.NO_TASK,0,"");
  device.configureSide(device.TASK_1,10,"Task 1");
  device.configureSide(device.TASK_2,20,"Task 2");
  device.configureSide(device.TASK_3,30,"Task 3");
  device.configureSide(device.TASK_4,40,"Task 4");
  device.configureSide(device.TASK_5,50,"Task 5");
  device.configureSide(device.TASK_6,60,"Task 6");

  /*for (int i = 1; i <= BOOT_INITIAL_BLINK_SECS * 5; i++){
    device.turnAllLedsOn();
    delay(100);
    device.turnAllLedsOff();
    delay(100);
  }*/

  // Connect to WiFi
  checkOrConnectWifi();

  server.begin(); 

  currentTask = device.getUpFacingTask(); // When turned on, the current position must be consider as no active counting.

  all_project_size = connector.getAvailableProject(all_project_ids,all_project_names);

  configServer.setAvailableProjects(all_project_ids, all_project_names, all_project_size);

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
  // display to show:
  //   - IP
  //   - config URL
  //   - Current project and task (for those in Septimo and thos not)
  //   - Total hours of current day

  
  
}
