#pragma once

bool shouldStartSetup();
void doSetup();
void setupWIFI();
void setupWebserver();
void handleNotFound();
void displayData();
void setReadyForInternalSensorUpdate();
void setReadyForTimeUpdate();
double berechneTT(double t, double RH);
void setExternalSensorInvalid();
void updateInternalSensor();
void updateTime();
double RHtoDP(double t, double RH);
void flash();
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
void handleRoot();
void handleJsonData();
void updateExternalSensor();

