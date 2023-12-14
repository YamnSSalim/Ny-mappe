#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Encoders encoders;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4Motors motors;
Zumo32U4OLED oled;

///////////////////
//// Encoders /////
///////////////////

//---Encoders Variabel---//
int32_t countLeft = 0;
int32_t countRight = 0;

void setupEncoders()
{
    encoders.init();
}

void updateEncoders()
{ // TODO: FIX
    long Left = encoders.getCountsAndResetLeft();
    long Right = encoders.getCountsAndResetRight();

    countLeft += Left;
    countRight += Right;
}

////////////////////////
// Calculate Distance //
////////////////////////

//------Global Speed & Distance Variablers ------//
const float WHEEL_DIAMETER_METER = 0.035; // 35 mm converted to meters
const float wheelCircumference = 2 * PI * WHEEL_DIAMETER_METER;
const float encoderCpr = 909.7; // Counts per revolution
int prevCountLeft = 0;
int prevCountRight = 0;
//-----------------------------------------------//

float distance = 0;

void updateDistance()
{
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

void updateSpeed()
{
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
const int numMeasurements = 10;
float speedMeasurements[numMeasurements] = {0};
int speedIndex = 0;
//-----------------------------//

float averageSpeed = 0;

void updateAverageSpeed()
{
    speedMeasurements[speedIndex] = speed;
    speedIndex = (speedIndex + 1) % numMeasurements;

    float sum = 0;
    for (int i = 0; i < numMeasurements; i++)
    {
        sum += speedMeasurements[i];
    }
    averageSpeed = sum / numMeasurements;
}

///////////////////
////OLED-Screen////
///////////////////

void setupOLED()
{
    oled.init();
    oled.clear();
}

void updateSpeedOMeterScreen()
{
    oled.setLayout11x4();
    oled.gotoXY(2.5, 0);
    oled.print("SpeedY");

    oled.gotoXY(1, 2);

    oled.print("m/s: ");
    oled.print(speed);

    oled.gotoXY(2, 3);

    oled.print("m: ");
    oled.print(distance);
}

///////////////////
// Discharge Rate /
///////////////////

//-----Discharge-Variables-----//
const float dischargeRateBase = 0.05;
//-----------------------------//

float dischargeRate = 0;

void updateDischargeRate()
{
    float calculatedDischargeRate = dischargeRateBase * averageSpeed * distance;

    dischargeRate += calculatedDischargeRate / 1.5;
}

///////////////////
// Charge Rate ////
///////////////////

//-----Discharge-Variables-----//
const float chargeRateBase = 0.05;
//-----------------------------//

float chargeRate = 0;

void updateChargeRate()
{
    float calculatedChargeRate = chargeRateBase * averageSpeed * distance;

    chargeRate += calculatedChargeRate / 2;
}

///////////////////
////// Alarm //////
///////////////////

int alarm = 0;

void updateAlarm()
{
}

///////////////////
// Battery Level //
///////////////////

//-----Battery-Variables-----//
const float maxBatteryLevel = 100.0;
const float minBatteryLevel = 0.0;
const float lowBatteryThreshold = 20.0;
//-----------------------------//

//-----------Flag-MOVEMENT-------------//
bool shouldMove = true;
//-------------------------------------//

//-----------Charging-Cycles-------------//
int chargingCycles = 0;
//-------------------------------------//

float batteryLevel = 100.0;

void updateBatteryLevel()
{ // Edit
    if (speed > 0)
    {
        batteryLevel -= dischargeRate;

        if (batteryLevel < lowBatteryThreshold)
        {
            batteryLevel = lowBatteryThreshold;
        }
        if (batteryLevel > maxBatteryLevel)
        {
            batteryLevel = maxBatteryLevel;
        }
    }
}

///////////////////
//// Main Loop ////
///////////////////

void setup()
{
    Serial.begin(9600);
    buttonA.waitForPress();
    setupOLED();
    setupEncoders();
}

void loop()
{
    if (shouldMove)
    {
        motors.setSpeeds(150, 150);
    }

    updateEncoders(); // Fixed

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

    updateDistance(); // Fixed

    /*
    //-------Distance-OUTPUT-------//
    Serial.println("Distance: ");
    Serial.println(distance);
    Serial.println("--------------");
    //-----------------------------//
  */

    updateSpeed(); // Fixed

    /*
    //-------Speed-OUTPUT-------//
    Serial.println("Speed: ");
    Serial.println(speed);
    Serial.println("--------------");
    //-----------------------------//
  */

    updateAverageSpeed(); // Fixed
    /*
//-------Averagespeed-OUTPUT-------//
Serial.println("Averagespeed: ");
Serial.println(averageSpeed);
Serial.println("--------------");
//-----------------------------//
*/

    updateDischargeRate(); // Fixed

    /*
    //-------Discharge-OUTPUT-------//
    Serial.println("DischargeRate: ");
    Serial.println(dischargeRate);
    Serial.println("--------------");
    //-----------------------------//
  */

    updateChargeRate(); // Fixed

    /*
    //-------Discharge-OUTPUT-------//
    Serial.println("ChargeRate: ");
    Serial.println(chargeRate);
    Serial.println("--------------");
    //-----------------------------//
  */
    updateSpeedOMeterScreen();

    updateBatteryLevel();
    Serial.println("BatteryLevel: ");
    Serial.println(batteryLevel);
    Serial.println("---------------");
    Serial.println("ChargingCylce: ");
    Serial.println(chargingCycles);

    //---------------------------------------TEST-logic------------------------------------//

    delay(500);
}