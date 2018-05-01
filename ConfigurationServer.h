/*
  ConfigurationServer.cpp - Library to manage the Septimo time tracker configuration web interface.
  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#ifndef SEPTIMO_CONFIG_SERVER_H
#define SEPTIMO_CONFIG_SERVER_H

#include <Arduino.h>
#include <WiFi101.h>
#include "SeptimoPrism.h"

class ConfigurationServer {

  public:
    
    ConfigurationServer();
    void setAvailableProjects(int ids[], String names[], int lengtlength_h);
    void processIncomingServerRequest(WiFiServer server, SeptimoPrism &device, int currentTask);

  private:

    int allProjectIDs[40];
    String allProjectsNames[40];
    int allProjectsLength;
  
    String buildConfigurationFormTask(int side, SeptimoPrism &device);
    void responseWithEmptyOK(WiFiClient client);
    void responseWithSaveConfirmation(WiFiClient client);
    void responseWithConfigurationScreen(WiFiClient client, SeptimoPrism &device, int currentTask);
    String extractPOSTfromRequest(String request, WiFiClient client);
    void setSidesForInput(String input, SeptimoPrism &device);
    
    
};

#endif /* SEPTIMO_CONFIG_SERVER_H */
