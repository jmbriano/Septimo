#include "SeptimoPrism.h"
// Each side of Septimo is associated with a project and a task
using namespace std;

// When a new side is facing up, it will wait the following number of second before triggering the task change
const int TRANSITION_CONFIRMATION_SECS = 5;
    

// Sensors to indentify the side facing down
int SENSOR_NO_TASK = A0;
int SENSOR_TASK_1 = A1;
int SENSOR_TASK_2 = A2;
int SENSOR_TASK_3 = A3;
int SENSOR_TASK_4 = A4;
int SENSOR_TASK_5 = A5;
int SENSOR_TASK_6 = A6;

// LEDs used to indicate the current task
int LED_NO_TASK = 0;
int LED_TASK_1 = 1;
int LED_TASK_2 = 2;
int LED_TASK_3 = 3;
int LED_TASK_4 = 4;
int LED_TASK_5 = 5;
int LED_TASK_6 = 6;


SeptimoPrism::SeptimoPrism(){

};


void SeptimoPrism::begin(){
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

  justBooted = true;
  
}

int SeptimoPrism::getLedForTaskID(int task){
  if (TASK_1 == task) return LED_TASK_1;
  if (TASK_2 == task) return LED_TASK_2;
  if (TASK_3 == task) return LED_TASK_3;
  if (TASK_4 == task) return LED_TASK_4;
  if (TASK_5 == task) return LED_TASK_5;
  if (TASK_6 == task) return LED_TASK_6;
  if (NO_TASK == task) return LED_NO_TASK;
}

int SeptimoPrism::getSensorForTaskID(int task){
  if (TASK_1 == task) return SENSOR_TASK_1;
  if (TASK_2 == task) return SENSOR_TASK_2;
  if (TASK_3 == task) return SENSOR_TASK_3;
  if (TASK_4 == task) return SENSOR_TASK_4;
  if (TASK_5 == task) return SENSOR_TASK_5;
  if (TASK_6 == task) return SENSOR_TASK_6;
  if (NO_TASK == task) return SENSOR_NO_TASK;
}


// This function stores the project ID and the task description associated to the given side.
void SeptimoPrism::configureSide(int side, int projectId, String task) {
  Serial.println ("Setting side: "+String(side)+" to Project: " + projectId + " and Task: "+ task);
  sideProjects[side] = projectId;
  sideTasks[side] = task;
}

// This function returns the Task description for the requested side.
String SeptimoPrism::getSideTask(int side){
  return sideTasks[side];
}

// This function returns the Project ID for the requested side.
int SeptimoPrism::getSideProject(int side){
  return sideProjects[side];
}


void SeptimoPrism::turnAllLedsOff(){
  turnOffLedForTask(NO_TASK);
  turnOffLedForTask(TASK_1);
  turnOffLedForTask(TASK_2);
  turnOffLedForTask(TASK_3);
  turnOffLedForTask(TASK_4);
  turnOffLedForTask(TASK_5);
  turnOffLedForTask(TASK_6);
}

void SeptimoPrism::turnAllLedsOn(){
  turnOnLedForTask(NO_TASK);
  turnOnLedForTask(TASK_1);
  turnOnLedForTask(TASK_2);
  turnOnLedForTask(TASK_3);
  turnOnLedForTask(TASK_4);
  turnOnLedForTask(TASK_5);
  turnOnLedForTask(TASK_6);
}

void SeptimoPrism::turnOffLedForTask(int taskID){
  digitalWrite(getLedForTaskID(taskID),LOW);
  
}

void SeptimoPrism::turnOnLedForTask(int taskID){
  digitalWrite(getLedForTaskID(taskID),HIGH);
  
}

// This function reads all sensors to find the darkest one (smaller input) and return the task associated to that sensor. Returns values between 1 and 7
int SeptimoPrism::getUpFacingTask(){
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
int SeptimoPrism::getUpfacingTaskAfterTransition(int currentTask){

  int newUpfacing = getUpFacingTask();
  if (newUpfacing != currentTask) {
    
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
        justBooted = false;
        return newUpfacing;
      } 
    }
    
  }
  return currentTask;
}
