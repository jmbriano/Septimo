#include "ConfigurationServer.h"


using namespace std;

ConfigurationServer::ConfigurationServer(){
};

void ConfigurationServer::setAvailableProjects(int ids[], String names[], int length_){
  int i;
  for (i = 0; i < length_; i++) {
    allProjectIDs[i] = int(ids[i]);
    allProjectsNames[i] = String(names[i]);
    Serial.println ("Registered project " + allProjectsNames[i] + " with ID: " +String(allProjectIDs[i])+ " at position " + String(i));
  }
  allProjectsLength = length_;
}


String ConfigurationServer::buildConfigurationFormTask(int side, SeptimoPrism &device){
  Serial.println("buildConfigurationFormTask");
  Serial.println("device.getSideTask(side):" + device.getSideTask(side));
  Serial.println("device.getSideProject(side):" + String(device.getSideProject(side)));
  
  String formGroup = "";
  //formGroup.concat("<div class=\"form-group\">");
  formGroup.concat("  <label>Side "+String(side)+"</label>");
  formGroup.concat("   <div class=\"row\">");
  formGroup.concat("      <div class=\"input-group mb-3 col-sm-4\">");
  formGroup.concat("        <select class=\"form-control\" id=\"exampleFormControlSelect1\" name=\"project"+String(side)+"\">");
  
  for (int i = 0; i< allProjectsLength; i++) {
    String selected = "";
    if (device.getSideProject(side) == allProjectIDs[i]) selected=" selected ";
    formGroup.concat("          <option "+selected+"value=\""+String(allProjectIDs[i])+"\">"+String(allProjectsNames[i])+"</option>");  
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

void ConfigurationServer::responseWithEmptyOK(WiFiClient client) {
  Serial.println("responseWithEmptyOK");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("</html>");
  
}

void ConfigurationServer::responseWithSaveConfirmation(WiFiClient client) {
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

void ConfigurationServer::responseWithConfigurationScreen(WiFiClient client, SeptimoPrism &device, int currentTask) {
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
    client.println(buildConfigurationFormTask(side, device));
  }
  
  client.println("        <input class=\"btn btn-primary\" type='submit' value='Submit' onClick='funct()'>");
  client.println("      </form>");
  client.println("<SCRIPT TYPE='text/JavaScript'>function funct(){document.getElementById('tasksForm').submit;}</SCRIPT></body>");
  
  client.println("</html>");
}


String ConfigurationServer::extractPOSTfromRequest(String request, WiFiClient client) {
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

void ConfigurationServer::setSidesForInput(String input, SeptimoPrism &device){
    Serial.println("setSidesForInput");
    Serial.println("POST: "+ input);
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

void ConfigurationServer::processIncomingServerRequest(WiFiServer server, SeptimoPrism &device, int currentTask) {
  
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
            responseWithConfigurationScreen(client, device, currentTask);
            break;
        }

        if (c == '\n' && currentLineIsBlank && request.startsWith("GET")) {
            responseWithEmptyOK(client);
            break;
        }
        
        if (c == '\n' && currentLineIsBlank && request.startsWith("POST")) {

          // This corresponds to the porcessing of the form submission.
          String formValues = extractPOSTfromRequest(request,client);
          setSidesForInput(formValues, device);
         
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
