#include <U8g2lib.h>
#include <WiFi101.h>
#include "SeptimoPrism.h"

#include "config/arduino_secrets.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
SeptimoPrism device;

int BOOT_INITIAL_BLINK_SECS = 5;
// PIN numbers

int allProjectIDs[] = {10,20,30,40,50,60 };
String allProjects[] = {"RECAPP", "REX","GTP", "QUOTA", "VSKO", "KANS" };
int allProjectsLenght = 6;

// Each side of Septimo is associated with a project and a task
//int projects[7]; //TODO: check why if removed it hangs
//String tasks[7]; //TODO: check why if removed it hangs

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

String buildConfigurationFormTask(int side){
  Serial.println("buildConfigurationFormTask");
  Serial.println("device.getSideTask(side):" + device.getSideTask(side));
  Serial.println("device.getSideProject(side):" + String(device.getSideProject(side)));
  
  String formGroup = "";
  //formGroup.concat("<div class=\"form-group\">");
  formGroup.concat("  <label>Side "+String(side)+"</label>");
  formGroup.concat("   <div class=\"row\">");
  formGroup.concat("      <div class=\"input-group mb-3 col-sm-4\">");
  formGroup.concat("        <select class=\"form-control\" id=\"exampleFormControlSelect1\" name=\"project"+String(side)+"\">");
  
  for (int i = 0; i< allProjectsLenght; i++) {
    String selected = "";
    if (device.getSideProject(side) == allProjectIDs[i]) selected=" selected ";
    formGroup.concat("          <option "+selected+"value=\""+String(allProjectIDs[i])+"\">"+String(allProjects[i])+"</option>");  
  }
  
  formGroup.concat("        </select>");
  formGroup.concat("    </div>");
  formGroup.concat("    <div class=\"input-group mb-3 col-sm-4\">");
  formGroup.concat("      <div class=\"input-group-prepend\">");
  formGroup.concat("            <span class=\"input-group-text\" id=\"basic-addon3\">Description</span>");
  formGroup.concat("        </div>");
  formGroup.concat("        <input type=\"text\" class=\"form-control\" id=\"task"+String(side)+"\" name=\"task"+String(side)+"\" aria-describedby=\"basic-addon3\" value=\""+device.getSideTask(side)+"\" required>");
  formGroup.concat("      </div>");
  formGroup.concat("  </div>");
  //formGroup.concat("</div>");
  return formGroup;
}

void responseWithEmptyOK(WiFiClient client) {
  Serial.println("responseWithEmptyOK");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("</html>");
  
}
void responseWithSaveConfirmation(WiFiClient client) {
  Serial.println("responseWithSaveConfirmation");
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
  Serial.println("responseWithConfigurationScreen");
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
  client.println("    <div id=\"currentTask\">Current Project: " + String(device.getSideProject(currentTask)) + " Current Task: "+ device.getSideTask(currentTask) + "</div>");
  client.println("      <form name=\"tasksForm\" id=\"tasksForm\" method=\"post\" action=\"/config\">");

  for (int side = device.TASK_1; side <= device.TASK_6; side++) {
    client.println(buildConfigurationFormTask(side));
  }
  
  client.println("        <input class=\"btn btn-primary\" type='submit' value='Submit' onClick='funct()'>");
  client.println("      </form>");
  client.println("<SCRIPT TYPE='text/JavaScript'>function funct(){document.getElementById('tasksForm').submit;}</SCRIPT></body>");
  
  client.println("</html>");
}


String extractPOSTfromRequest(String request, WiFiClient client) {
    Serial.println("extractPOSTfromRequest");
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
    Serial.println("setSidesForInput");
    device.configureSide(device.NO_TASK,0,"NO_TASK"); 
    int taskStart = 0;
    int taskEnd = 0;
    int projectStart = 0;
    int projectEnd = 0;
    for (int side = device.TASK_1; side <= device.TASK_6; side++) {
       projectStart = input.indexOf("project"+String(side))+9;
       projectEnd = input.indexOf("&", projectStart);
       taskStart = input.indexOf("task"+String(side))+6;
       taskEnd = input.indexOf("&", taskStart);
       String project = input.substring(projectStart,projectEnd);
       String task = input.substring(taskStart,taskEnd);
       task.replace('+',' ');
       device.configureSide(side,project.toInt(),task);
    }
  
}

void processIncomingServerRequest() {
  
  WiFiClient client = server.available();
  if (client) {
    //Serial.println("client");
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
}

void loop() {

  // Connect to WiFi
  checkOrConnectWifi();
  
  // Check if there was any upfacing side change. This function will exit with no action if the same side is facing up
  int newTaskUp = device.getUpfacingTaskAfterTransition(currentTask);

  if (newTaskUp != currentTask) {
    executeTransitionTo(newTaskUp);
  }

  processIncomingServerRequest();
  
}
