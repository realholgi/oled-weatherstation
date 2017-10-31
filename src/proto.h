//
// Created by Holger Eiboeck on 31.10.17.
//

#ifndef WETTERSTATION_PROTO_H
#define WETTERSTATION_PROTO_H

bool shouldStartSetup();

void doSetup();

void setupWIFI();

void setupWebserver();

void handleNotFound();

void getRemoteTempHumi(byte *data);

void drawOtaProgress(unsigned int progress, unsigned int total);

void drawOtaEnd();

void uploadData();

void displayData();

void setReadyForInternalSensorUpdate();

void setReadyForTimeUpdate();

void setReadyForUploadData();

double berechneTT(double t, double RH);

void setExternalSensorInvalid();

bool shouldStartSetup();

void updateInternalSensor();

void setExternalSensorInvalid();

void updateTime();

double RHtoDP(double t, double RH);

boolean loadConfig();

void flash();

void saveConfigCallback();

void configModeCallback(WiFiManager *myWiFiManager);

void handleRoot();

void handleJsonData();

#endif //WETTERSTATION_PROTO_H
