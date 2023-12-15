#include <Wire.h>
#include <Zumo32U4.h>
#include <EEPROM.h>

Zumo32U4Encoders encoders;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4Motors motors;
Zumo32U4OLED oled;
Zumo32U4Buzzer buzzer;


///////////////////
//// Encoders /////
///////////////////

//---Encoders Variabel---//
int32_t countLeft = 0;
int32_t countRight = 0;
//-----------------------//


void setupEncoders() {
  encoders.init();
}

void updateEncoders() {  // TODO: FIX
  long Left = encoders.getCountsAndResetLeft();
  long Right = encoders.getCountsAndResetRight();

  countLeft += Left;
  countRight += Right;
}

////////////////////////
// Calculate Distance //
////////////////////////

//------Global Speed & Distance Variablers ------//
const float WHEEL_DIAMETER_METER = 0.035;  // 35 mm converted to meters
const float wheelCircumference = 2 * PI * WHEEL_DIAMETER_METER;
const float encoderCpr = 909.7;  // Counts per revolution
int prevCountLeft = 0;
int prevCountRight = 0;
//-----------------------------------------------//

float distance = 0;

void updateDistance() {
  float distanceLeft = (countLeft * wheelCircumference) / encoderCpr;
  float distanceRight = (countRight * wheelCircumference) / encoderCpr;

  distance = (distanceRight + distanceLeft) / 2;
}

///////////////////
// Calculate Speed
///////////////////

//----Speed-Variabler----//
float speedLeft = 0;
float speedRight = 0;
const float timeInterval = 1.0;
//-----------------------//

float speed = 0;

void updateSpeed() {
  int deltaCountLeft = countLeft - prevCountLeft;
  int deltaCountRight = countRight - prevCountRight;

  speedLeft = (deltaCountLeft * wheelCircumference) / (encoderCpr * timeInterval);
  speedRight = (deltaCountRight * wheelCircumference) / (encoderCpr * timeInterval);

  prevCountRight = countRight;
  prevCountLeft = countLeft;

  speed = (speedLeft + speedRight) / 2.0;
}


///////////////////
// Average Speed //
///////////////////

//---Average-Speed-Variables---//
const int numMeasurements = 3;
float speedMeasurements[numMeasurements] = { 0 };
int speedIndex = 0;
//-----------------------------//

float averageSpeed = 0;

void updateAverageSpeed() {
  speedMeasurements[speedIndex] = speed;
  speedIndex = (speedIndex + 1) % numMeasurements;

  float sum = 0;
  for (int i = 0; i < numMeasurements; i++) {
    sum += speedMeasurements[i];
  }
  averageSpeed = sum / numMeasurements;
}


///////////////////
//    MaxSpeed   //
///////////////////
float maxSpeed = 0;

void updateMaxSpeed() {
  if (speed > maxSpeed) {
    maxSpeed = speed;
  }
}

////////////////////
//    Over-70-%   //
////////////////////

//------Variabel-70%----//
unsigned long lastOver70Time = 0;
//-----------------------//

int over70Time = 0;

int over70Percent() {
  float seventyPerventMaxSpeed = 0.7 * (maxSpeed/10);

  if (speed > seventyPerventMaxSpeed) {
    if (lastOver70Time == 0) {
      lastOver70Time = millis();
    }
  } else {
    if (lastOver70Time != 0) {
      over70Time += (millis() - lastOver70Time) / 1000;
      lastOver70Time = 0;
    }
  }
  return over70Time;
}



///////////////////
// Discharge Rate /
///////////////////

//-----Discharge-Variables-----//
const float dischargeRateBase = 2.5;
//-----------------------------//

float dischargeRate = 0;


void updateDischargeRate() {
  dischargeRate = abs(averageSpeed * dischargeRateBase);
  //dischargeRate += abs(calculatedDischargeRate);
}

///////////////////
// Charge Rate /
///////////////////

//-----Charge-Variables-----//
const float chargeRateBase = 2.8;
//-----------------------------//


float chargeRate = 0;

void updateChargeRate() {
  chargeRate = abs(averageSpeed * chargeRateBase);
  //chargeRate += abs(calculatedChargeRate);
}


///////////////////
//    Movement  ///
///////////////////


void driveForwards() {
  motors.setSpeeds(150, 150);
}

void driveBackwards() {
  motors.setSpeeds(-150, -150);
}

///////////////////
////// Alarm //////
///////////////////

void emergencyAlarm() {
  buzzer.playFrequency(1500, 150, 15);
}

void twentyRateAlarm() {
  buzzer.playFrequency(500, 200, 15);
}

////////////////////////
//// Hidden-Feature ////
////////////////////////

//-----Hidden-Features-Variables-----//
bool HiddenFeatureActivated = false;
//-----------------------------------//

void updateHiddenFeature() {
  if (buttonB.isPressed()) {
    HiddenFeatureActivated = true;
    Serial.println("HiddenFeatureActivated - Charging for 2 minutes");
  } else {
    HiddenFeatureActivated = false;
    Serial.println("HiddenFeatureDeActivated");
  }
}


///////////////////
// Battery Level //
///////////////////

//-----Battery-Variables-----//
const float maxBatteryLevel = 100.0;
const float minBatteryLevel = 0.0;
const float lowBatteryThreshold = 20.0;
//-----------------------------//

//----------ChargingCycles-Variables--------//
int chargingCycles = 0;
//------------------------------------------//


float batteryLevel = 50.0;

void updateBatteryLevel() {
  if (HiddenFeatureActivated == true) {
    driveBackwards();
    chargingCycles++;

    Serial.println("ChargingCycle: ");
    Serial.println(chargeRate);
    batteryLevel += chargeRate;

    Serial.println("Charging: ");
    Serial.println(batteryLevel);

    if (batteryLevel >= 35.0) {

      twentyRateAlarm();
      delay(300);
    }
  } else {
    driveForwards();
    batteryLevel -= dischargeRate;
    Serial.println("Discharging: ");
    Serial.println(batteryLevel);
  }
}


///////////////////////////
//Emergency-Charging-Mode//
///////////////////////////


void emergencyChargingMode() {
  if (batteryLevel < 20) {
    Serial.println("Low battery! Please recharge.");
    emergencyAlarm();
    if (buttonC.isPressed()) {
      batteryLevel += chargeRate * 10;
    }
  }
}
////////////////////////////
//     Battery-Health     //
////////////////////////////

//------Battery-Health-Variables-----//
float batteryFactor = 0.0;
float maxBatteryHealth = 100.0;
float miniBatteryHealth = 0.0;
float chargingCyclesFactor = (chargingCycles * 1.0) / 100.0;
float capacityFactor = (batteryLevel - miniBatteryHealth) / (maxBatteryLevel - minBatteryLevel);
int mistake = 0;
int error = 1; 
int updateMistake = 0;
//-----------------------------------//
int battery_health = 0;

void updateBatteryHealth() {
  updateMistake = random(100);
  if (mistake = updateMistake){
    error +=2; 

    battery_health -= 50;
  }

  batteryFactor = (chargingCyclesFactor + capacityFactor) / 2.0;
  battery_health = maxBatteryHealth - (batteryFactor * (maxBatteryHealth - minBatteryLevel));

  battery_health -= 1; 
}


//////////////////
////  EEPROM  ////
//////////////////


//EEPROM.write(0,battery_health); //FeilKode - EEPROM fungere ikke 


///////////////////
////OLED-Screen////
///////////////////

void setupOLED() {
  oled.init();
  oled.clear();
}

void defaultScreen() {
  oled.clear();
  oled.setLayout21x8();
  oled.gotoXY(2, 0);
  oled.print("SpeedY");

  oled.gotoXY(0, 2);

  oled.print("Speed: ");
  oled.print(speed);
  oled.print("m/s");


  oled.gotoXY(0, 3);

  oled.print("m: ");
  oled.print(distance);
  oled.print("m");
}


void secondaryScreen() {
  oled.clear();
  oled.setLayout21x8();

  oled.gotoXY(2, 0);
  oled.print("Average");

  oled.gotoXY(0, 2);
  oled.print("Max_Speed: ");
  oled.print(maxSpeed);
  oled.print("m/s");

  oled.gotoXY(0, 3);
  oled.print("Avg_Speed: ");
  oled.print(averageSpeed);
  oled.print("m/s");

  oled.gotoXY(0, 4);
  oled.print("O_70:");
  oled.print(over70Time);
  oled.print("s");
}

void thirdScreen() {
  oled.clear();
  oled.setLayout21x8();
  oled.gotoXY(2, 0);
  oled.print("B_Status");


  oled.gotoXY(0, 2);
  oled.print("ChargeC: ");
  oled.print(chargingCycles);
  oled.print("%");


  oled.gotoXY(0, 3);
  oled.print("B_Level: ");
  oled.print(batteryLevel);
  oled.print("%");


  oled.gotoXY(0, 4);
  oled.print("B_Health: ");
  oled.print(battery_health);
  oled.print("%");
}

unsigned long lastScreenChange = 0;
int screenCounter = 0;

void updateSpeedOMeterScreen() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastScreenChange >= 3000) {
    lastScreenChange = currentMillis;
    screenCounter++;

    if (screenCounter % 3 == 0) {
      defaultScreen();
    } else if (screenCounter % 3 == 1) {
      secondaryScreen();
    } else {
      thirdScreen();
    }
  }
}


///////////////////
//// Main Loop ////
///////////////////

void setup() {
  Serial.begin(9600);
  buttonA.waitForPress();
  setupOLED();
  setupEncoders();
}

void loop() {
  driveForwards();
  updateEncoders();  // Fixed

  /*
    //-----Encoder-OUTPUT-------//
    Serial.println("--------------");
    Serial.println("CountLeft: ");
    Serial.println(countLeft);
    Serial.println("CountRight: ");
    Serial.println(countRight);
    Serial.println("--------------");
    //--------------------------//
   */

  updateDistance();  // Fixed

  /* 
    //-------Distance-OUTPUT-------//
    Serial.println("Distance: ");
    Serial.println(distance);
    Serial.println("--------------");
    //-----------------------------//
 */

  updateSpeed();  // Fixed

  /*
    //-------Speed-OUTPUT-------//
    Serial.println("Speed: ");
    Serial.println(speed);
    Serial.println("--------------");
    //-----------------------------//
  */

  updateAverageSpeed();  // Fixed
                         /*
//-------Averagespeed-OUTPUT-------//
Serial.println("Averagespeed: ");
Serial.println(averageSpeed);
Serial.println("--------------");
//-----------------------------//
*/

  updateMaxSpeed();
/*
  Serial.println("maxSpeed: ");
  Serial.println(maxSpeed);
*/

  over70Percent();


  updateDischargeRate();  // Fixed

  /*
    //-------Discharge-OUTPUT-------//
    Serial.println("DischargeRate: ");
    Serial.println(dischargeRate);
    Serial.println("--------------");
    //-----------------------------//
*/

  updateChargeRate();  //Fixed
                       /*
    //-------Charge-OUTPUT---------//
     Serial.println("ChargeRate: ");
    Serial.println(chargeRate);
    Serial.println("--------------");
*/


  updateSpeedOMeterScreen();

  /*
  
  //Serial.println("ChargingCylce: ");
  //Serial.println(chargingCycles);
*/

  updateHiddenFeature();
  updateBatteryLevel();

  
  //-------Battery-OUTPUT--------// 
  Serial.println("BatteryLevel: ");
  Serial.println(batteryLevel);
  Serial.println("---------------");



  emergencyChargingMode();

  updateBatteryHealth();





  //---------------------------------------TEST-logic------------------------------------//

  delay(500);
}