#include "ToggleConnector.h"
#include "config/arduino_secrets.h"

#include <ArduinoJson.h>

using namespace std;

ToggleConnector::ToggleConnector(WiFiSSLClient &client_) : restClient("www.toggl.com", client_) {

};


int ToggleConnector::getAvailableProject(int * project_ids,String * project_names){

  int projectIndex = 0;
  String resp;

  // Get Workspace
  addHeaders();
  restClient.get("/api/v8/workspaces", &resp);

  DynamicJsonBuffer jsonBuffer(resp.length());
  JsonArray& workspacesList = jsonBuffer.parseArray(resp);
  if (workspacesList.success()) {
    
    // Iterate each Workspace
    int numberOfWrokspaces = workspacesList.size();
    Serial.println("Workspaces found: "+ String(numberOfWrokspaces));
    for (int i=0;i<numberOfWrokspaces; i++){
      String wid = workspacesList[i]["id"];
     
        Serial.println("Workspace id: " + wid);

        // Get Projects for each obtained Workspace
        char path[50];
        sprintf(path, "/api/v8/workspaces/%s/projects", wid.c_str());
        resp = "";
        addHeaders();
        restClient.get(path, &resp);
        JsonArray& projectList = jsonBuffer.parseArray(resp);
        int numberOfProjectsInWorkspace = projectList.size();
        Serial.println("Projects found: "+ String(numberOfProjectsInWorkspace));
        for (int p=0;p<numberOfProjectsInWorkspace; p++){
          String project = projectList[p];
          JsonObject& projectData = jsonBuffer.parseObject(project);
          String pid = projectData["id"];
          String pName = projectData["name"];
          Serial.println(String(pid) + " - " + String(pName));
          project_ids[projectIndex] = pid.toInt();
          project_names[projectIndex] = pName;
          projectIndex++;
        }
    }
    
  } else {
    Serial.println("workspacesList parseObject() failed");
  }

  return projectIndex;
  
}

void ToggleConnector::startActivity(int pid,const char * description) {

  Serial.println("Starting activity with id '"+String(pid)+"' and description '"+description+"'");
  StaticJsonBuffer<250> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& time_entry = root.createNestedObject("time_entry");
  time_entry["description"] = description;
  time_entry["pid"] = pid;
  time_entry["created_with"] = "Septimo";
  //root.prettyPrintTo(Serial);
  //Serial.println("");

  char body[250];
  root.printTo(body);
  addHeaders();
  int code = restClient.post("/api/v8/time_entries/start", body);
  Serial.println("Result: "+String(code));
  

}

void ToggleConnector::stopActivity(){
  DynamicJsonBuffer jsonBuffer(500);
  String resp = "";
  addHeaders();
  restClient.get("/api/v8/time_entries/current", &resp);

  JsonObject& currentEntry = jsonBuffer.parseObject(resp);
  if (currentEntry.success()) {
    String currentEntryId = currentEntry["data"]["id"];
    if (currentEntryId.length() > 0) {
      String path = "/api/v8/time_entries/"+currentEntryId+"/stop";
      Serial.println("Stoping activity with id '"+currentEntryId+"'");
      addHeaders();
      int code = restClient.put(path.c_str(),"");
      Serial.println("Result: "+String(code));
    } else {
      Serial.println("No current time entry to stop");
    }
  } else {
    Serial.println("currentEntry parseObject() failed");
  }
}


void ToggleConnector::addHeaders(){
  restClient.setHeader(TOGGLE_TOKEN); 
  restClient.setHeader("Accept: */*");
  restClient.setContentType("application/json");
}

