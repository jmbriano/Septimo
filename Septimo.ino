
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi101.h>

#include "config/arduino_secrets.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

// PIN numbers

// Tasks IDs

int NO_TASK = 0; // No task
int TASK_1 = 1;
int TASK_2 = 2;
int TASK_3 = 3;
int TASK_4 = 4;
int TASK_5 = 5;
int TASK_6 = 6;


// LEDs used to indicate the current task
int LED_NO_TASK = 0;
int LED_TASK_1 = 1;
int LED_TASK_2 = 2;
int LED_TASK_3 = 3;
int LED_TASK_4 = 4;
int LED_TASK_5 = 5;
int LED_TASK_6 = 6;

// Sensors to indentify the side facing down
int SENSOR_NO_TASK = A0;
int SENSOR_TASK_1 = A1;
int SENSOR_TASK_2 = A2;
int SENSOR_TASK_3 = A3;
int SENSOR_TASK_4 = A4;
int SENSOR_TASK_5 = A5;
int SENSOR_TASK_6 = A6;


int allProjectIDs[] = {10,20,30,40,50,60 };
String allProjects[] = {"RECAPP", "REX","GTP", "QUOTA", "VSKO", "KANS" };
int allProjectsLenght = 6;

// Each side of Septimo is associated with a project and a task
int projects[7];
String tasks[7];

boolean justBooted;

// The currently active task is kept in this variable. NO_TASK = no timer running
int currentTask = NO_TASK;

// When a new side is facing up, it will wait the following number of second before triggering the task change
int TRANSITION_CONFIRMATION_SECS = 5;

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

// Sensors and Leds Functions
void setSide(int side, int projectId, String task) {
  Serial.println ("Setting side: "+String(side)+" to Project: " + projectId + " and Task: "+ task);
  projects[side] = projectId;
  tasks[side] = task;
}

String getSideTask(int side){
  return tasks[side];
}

int getSideProject(int side){
  return projects[side];
}

void turnOffLedForTask(int taskID){
  digitalWrite(getLedForTaskID(taskID),LOW);
  
}

void turnOnLedForTask(int taskID){
  digitalWrite(getLedForTaskID(taskID),HIGH);
  
}

void turnAllLedsOn(){
  turnOnLedForTask(NO_TASK);
  turnOnLedForTask(TASK_1);
  turnOnLedForTask(TASK_2);
  turnOnLedForTask(TASK_3);
  turnOnLedForTask(TASK_4);
  turnOnLedForTask(TASK_5);
  turnOnLedForTask(TASK_6);
}

void turnAllLedsOff(){
  turnOffLedForTask(NO_TASK);
  turnOffLedForTask(TASK_1);
  turnOffLedForTask(TASK_2);
  turnOffLedForTask(TASK_3);
  turnOffLedForTask(TASK_4);
  turnOffLedForTask(TASK_5);
  turnOffLedForTask(TASK_6);
}

void executeTransitionTo(int newTaskID){
  turnOffLedForTask(currentTask);
  stopTimmerForTask(currentTask);
  turnOnLedForTask(newTaskID);
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


// This function reads all sensors to find the darkest one (smaller input) and return the task associated to that sensor. Returns values between 1 and 7
int getUpFacingTask(){
  int minSensor = SENSOR_NO_TASK;
  int minTaskID = NO_TASK;
  if (analogRead(SENSOR_TASK_1) < analogRead(minSensor)) { minSensor = SENSOR_TASK_1; minTaskID = TASK_1; }
  if (analogRead(SENSOR_TASK_2) < analogRead(minSensor)) { minSensor = SENSOR_TASK_2; minTaskID = TASK_2; }
  if (analogRead(SENSOR_TASK_3) < analogRead(minSensor)) { minSensor = SENSOR_TASK_3; minTaskID = TASK_3; }
  if (analogRead(SENSOR_TASK_4) < analogRead(minSensor)) { minSensor = SENSOR_TASK_4; minTaskID = TASK_4; }
  if (analogRead(SENSOR_TASK_5) < analogRead(minSensor)) { minSensor = SENSOR_TASK_5; minTaskID = TASK_5; }
  if (analogRead(SENSOR_TASK_6) < analogRead(minSensor)) { minSensor = SENSOR_TASK_6; minTaskID = TASK_6; }

  return minTaskID;
  
}

// Call this function when a new side has become the upfacing one
void processChangeInUpfacingSide(){

  int newUpfacing = getUpFacingTask();
  if (newUpfacing != currentTask) {
    bool executeTransition = false;
    // wait TRANSITION_CONFIRMATION_SECS seconds blinking
    for (int i = 1; i <= TRANSITION_CONFIRMATION_SECS; i++){

      // If we are back to the current side up, then we skip the counting (Except we just booted)
      if (newUpfacing == currentTask && !justBooted) {
        break;
      }
      
      turnOnLedForTask(newUpfacing);
      delay(500);
      
      turnOffLedForTask(newUpfacing);
      delay(500);

      if(getUpFacingTask() != newUpfacing){
        // there was a new change of the upfacing task. Stop the blinking and restart the counting for the new upfacing task
        turnOffLedForTask(newUpfacing); // turn of the previous blinking. It should be off anyway, but just in case.
        newUpfacing = getUpFacingTask();
        i = 1;
      }

      if (i == TRANSITION_CONFIRMATION_SECS){
        executeTransition = true;
      }
    }

    if (executeTransition){
      executeTransitionTo(newUpfacing);
      justBooted = false;
    }  
  }
}

int getLedForTaskID(int task){
  if (TASK_1 == task) return LED_TASK_1;
  if (TASK_2 == task) return LED_TASK_2;
  if (TASK_3 == task) return LED_TASK_3;
  if (TASK_4 == task) return LED_TASK_4;
  if (TASK_5 == task) return LED_TASK_5;
  if (TASK_6 == task) return LED_TASK_6;
  if (NO_TASK == task) return LED_NO_TASK;
}

int getSensorForTaskID(int task){
  if (TASK_1 == task) return SENSOR_TASK_1;
  if (TASK_2 == task) return SENSOR_TASK_2;
  if (TASK_3 == task) return SENSOR_TASK_3;
  if (TASK_4 == task) return SENSOR_TASK_4;
  if (TASK_5 == task) return SENSOR_TASK_5;
  if (TASK_6 == task) return SENSOR_TASK_6;
  if (NO_TASK == task) return SENSOR_NO_TASK;
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

String buildConfigurationFormTask(int side){
  String formGroup = "";
  //formGroup.concat("<div class=\"form-group\">");
  formGroup.concat("  <label>Side "+String(side)+"</label>");
  formGroup.concat("   <div class=\"row\">");
  formGroup.concat("      <div class=\"input-group mb-3 col-sm-4\">");
  formGroup.concat("        <select class=\"form-control\" id=\"exampleFormControlSelect1\" name=\"project"+String(side)+"\">");
  
  for (int i = 0; i< allProjectsLenght; i++) {
    String selected = "";
    if (getSideProject(side) == allProjectIDs[i]) selected=" selected ";
    formGroup.concat("          <option "+selected+"value=\""+String(allProjectIDs[i])+"\">"+String(allProjects[i])+"</option>");  
  }
  
  formGroup.concat("        </select>");
  formGroup.concat("    </div>");
  formGroup.concat("    <div class=\"input-group mb-3 col-sm-4\">");
  formGroup.concat("      <div class=\"input-group-prepend\">");
  formGroup.concat("            <span class=\"input-group-text\" id=\"basic-addon3\">Description</span>");
  formGroup.concat("        </div>");
  formGroup.concat("        <input type=\"text\" class=\"form-control\" id=\"task"+String(side)+"\" name=\"task"+String(side)+"\" aria-describedby=\"basic-addon3\" value=\""+getSideTask(side)+"\" required>");
  formGroup.concat("      </div>");
  formGroup.concat("  </div>");
  //formGroup.concat("</div>");
  return formGroup;
}

void responseWithEmptyOK(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("</html>");
  
}
void responseWithSaveConfirmation(WiFiClient client) {
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("  <head>");
  client.println("    <title>Septimo :: Configuration</title>");
  client.println("    <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css\" integrity=\"sha384-9gVQ4dYFwwWSjIDZnLEWnxCjeSWFphJiwGPXr1jddIhOegiu1FwO5qRGvFXOdJZ4\" crossorigin=\"anonymous\">");
  client.println("  </head>");
  client.println("  <body>");
  client.println("    <div class=\"alert alert-success\" role=\"alert\">Configuration saved</div>");
  client.println("    <a class=\"btn btn-light\" href=\"/config\" role=\"button\">Back</button>");
  client.println("  </body>");
  client.println("</html>");

  
}
void responseWithConfigurationScreen(WiFiClient client) {
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("  <head>");
  client.println("    <title>Septimo :: Configuration</title>");
  client.println("    <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css\" integrity=\"sha384-9gVQ4dYFwwWSjIDZnLEWnxCjeSWFphJiwGPXr1jddIhOegiu1FwO5qRGvFXOdJZ4\" crossorigin=\"anonymous\">");
  client.println("  </head>");
  
  client.println("  <body>");
  client.println("    <div id=\"currentTask\">Current Project: " + String(projects[currentTask]) + " Current Task: "+ String(tasks[currentTask]) + "</div>");
  client.println("      <form name=\"tasksForm\" id=\"tasksForm\" method=\"post\" action=\"/config\">");

  for (int side = TASK_1; side <= TASK_6; side++) {
    client.println(buildConfigurationFormTask(side));
  }
  
  client.println("        <input class=\"btn btn-primary\" type='submit' value='Submit' onClick='funct()'>");
  client.println("      </form>");
  client.println("<SCRIPT TYPE='text/JavaScript'>function funct(){document.getElementById('tasksForm').submit;}</SCRIPT></body>");
  
  client.println("</html>");
}


String extractPOSTfromRequest(String request, WiFiClient client) {
    String post = "";
    char c;
    String temp = request.substring(request.indexOf("Content-Length:") + 15);
    temp.trim();
    int data_length = temp.toInt();
    while(data_length-- > 0)
    {
      c = client.read();
      post += c;
    }
    return post;
}

void setSidesForInput(String input){
    setSide(NO_TASK,0,"NO_TASK"); 
    int taskStart = 0;
    int taskEnd = 0;
    int projectStart = 0;
    int projectEnd = 0;
    for (int side = TASK_1; side <= TASK_6; side++) {
       projectStart = input.indexOf("project"+String(side))+9;
       projectEnd = input.indexOf("&", projectStart);
       taskStart = input.indexOf("task"+String(side))+6;
       taskEnd = input.indexOf("&", taskStart);
       String project = input.substring(projectStart,projectEnd);
       String task = input.substring(taskStart,taskEnd);
       task.replace('+',' ');
       setSide(side,project.toInt(),task);
    }
  
}

void processIncomingServerRequest() {
  WiFiClient client = server.available();
  if (client) {
    int timeoutControl = 5000;
    String request = "";
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected() && timeoutControl > 0) {
      timeoutControl--;
      delay(1);
      if (client.available()) {
        char c = client.read();
        request.concat(c);
        
        if (c == '\n' && currentLineIsBlank && request.startsWith("GET /config")) {
            responseWithConfigurationScreen(client);
            break;
        }

        if (c == '\n' && currentLineIsBlank && request.startsWith("GET")) {
            responseWithEmptyOK(client);
            break;
        }
        
        if (c == '\n' && currentLineIsBlank && request.startsWith("POST")) {

          // This corresponds to the porcessing of the form submission.
          String formValues = extractPOSTfromRequest(request,client);
          setSidesForInput(formValues);
         
          responseWithSaveConfirmation(client);
          break;
        }
        
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      } 
    }
    // give the web browser time to receive the data
    delay(10);

    if(timeoutControl == 0){
      Serial.println (" WARN: timeoutControl reached");
    }
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
  }
  
}



void setup() {

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  u8g2.begin();

  writeBigInDisplay("Septimo");

  justBooted = true;

  // Set PIN modes. 
  pinMode(LED_TASK_1,OUTPUT);
  pinMode(LED_TASK_2,OUTPUT);
  pinMode(LED_TASK_3,OUTPUT);
  pinMode(LED_TASK_4,OUTPUT);
  pinMode(LED_TASK_5,OUTPUT);
  pinMode(LED_TASK_6,OUTPUT);
  pinMode(LED_NO_TASK,OUTPUT);
  pinMode(SENSOR_TASK_1, INPUT);
  pinMode(SENSOR_TASK_2, INPUT);
  pinMode(SENSOR_TASK_3, INPUT);
  pinMode(SENSOR_TASK_4, INPUT);
  pinMode(SENSOR_TASK_5, INPUT);
  pinMode(SENSOR_NO_TASK, INPUT);
  Serial.begin(9600);

  setSide(NO_TASK,0,"");
  setSide(TASK_1,10,"Task 1");
  setSide(TASK_2,20,"Task 2");
  setSide(TASK_3,30,"Task 3");
  setSide(TASK_4,40,"Task 4");
  setSide(TASK_5,50,"Task 5");
  setSide(TASK_6,60,"Task 6");

  for (int i = 1; i <= TRANSITION_CONFIRMATION_SECS * 5; i++){
    turnAllLedsOn();
    delay(100);
    turnAllLedsOff();
    delay(100);
  }

  // Connect to WiFi
  checkOrConnectWifi();

  server.begin(); // TODO

  currentTask = getUpFacingTask(); // When turned on, the current position must be consider as no active counting.
}

void loop() {

  // Connect to WiFi
  checkOrConnectWifi();
  
  // Check if there was any upfacing side change. This function will exit with no action if the same side is facing up
  processChangeInUpfacingSide();

  processIncomingServerRequest();
  
}
