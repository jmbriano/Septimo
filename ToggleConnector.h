/*
  Septimo.cpp - Library to manage the Septimo time tracker device built
  for Arduino MKR1000. Copyright (c) 2011-2014 Arduino.  All right reserved.

  IMPORTANT: WINC1501 Model B (19.5.2) or superior must be installed in MKR1000 
  to allow SHA-256 connection to toggl

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#ifndef TOGGLE_CONNECTOR_H
#define TOGGLE_CONNECTOR_H

#include <Arduino.h>
#include "RestClient.h"


class ToggleConnector {

  public:
    ToggleConnector(WiFiSSLClient &client_);
    int getAvailableProject(int * side_project_ids,String * side_project_names);
    void startActivity(int pid, const char * description);
    void stopActivity();
    
  private:
  
    RestClient restClient;
    void addHeaders();
    
};

#endif /* TOGGLE_CONNECTOR_H */
