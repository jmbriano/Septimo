/*
  Septimo.cpp - Library to manage the Septimo time tracker device built
  for Arduino MKR1000. Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#ifndef SEPTIMO_H
#define SEPTIMO_H

#include <Arduino.h>

class SeptimoPrism {

  public:
    static const int NO_TASK = 0;
    static const int TASK_1 = 1;
    static const int TASK_2 = 2;
    static const int TASK_3 = 3;
    static const int TASK_4 = 4;
    static const int TASK_5 = 5;
    static const int TASK_6 = 6;
    

    SeptimoPrism();
    void begin();
    void configureSide(int side, int projectId, String task);
    String getSideTask(int side);
    int getSideProject(int side);
    void turnAllLedsOn();
    void turnAllLedsOff();
    void turnOffLedForTask(int taskID);
    void turnOnLedForTask(int taskID);
    int getUpfacingTaskAfterTransition(int currentTask);
    int getUpFacingTask();
    
    
  private:
  
    boolean justBooted; // true until there is a transition;
    int sideProjects[7];
    String sideTasks[7];
    int getLedForTaskID(int task);
    int getSensorForTaskID(int task);
    
};

#endif /* SEPTIMO_H */
