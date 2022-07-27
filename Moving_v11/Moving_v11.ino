#include <QTRSensors.h>

// This example is designed for use with six analog QTR sensors. These
// reflectance sensors should be connected to analog pins A0 to A5. The
// sensors' emitter control pin (CTRL or LEDON) can optionally be connected to
// digital pin 2, or you can leave it disconnected and remove the call to
// setEmitterPin().
//
// The setup phase of this example calibrates the sensors for ten seconds and
// turns on the Arduino's LED (usually on pin 13) while calibration is going
// on. During this phase, you should expose each reflectance sensor to the
// lightest and darkest readings they will encounter. For example, if you are
// making a line follower, you should slide the sensors across the line during
// the calibration phase so that each sensor can get a reading of how dark the
// line is and how light the ground is. Improper calibration will result in
// poor readings.
//
// The main loop of the example reads the calibrated sensor values and uses
// them to estimate the position of a line. You can test this by taping a piece
// of 3/4" black electrical tape to a piece of white paper and sliding the
// sensor across it. It prints the sensor values to the serial monitor as
// numbers from 0 (maximum reflectance) to 1000 (minimum reflectance) followed
// by the estimated location of the line as a number from 0 to 5000. 1000 means
// the line is directly under sensor 1, 2000 means directly under sensor 2,
// etc. 0 means the line is directly under sensor 0 or was last seen by sensor
// 0 before being lost. 5000 means the line is directly under sensor 5 or was
// last seen by sensor 5 before being lost.

QTRSensors qtr;

const uint8_t SensorCount = 4;
// create arrays for values of sensors
uint16_t sensorValues[SensorCount];
bool completeLeftTurn = false;
bool lastLeftTurn = false;
bool completeRightTurn = false;
bool lastRightTurn = false;
bool strongBlack[SensorCount];
uint16_t maximumValues[SensorCount];
uint16_t sensorRange[SensorCount];
uint16_t minimumValues[SensorCount];
int nowhereTimer = 0;
int lastMove = 0;

// values of speed for motors
int fullSpeed = 50;
uint8_t stopSpeed = 0;

// movement logic booleans
uint8_t moveLeftCounter = 0;
uint8_t moveRightCounter = 0;
uint8_t moveForwardCounter = 0;

// define Left Front and Back pins, Right Front and Back pins
uint16_t RFpin = 11;
uint16_t RBpin = 10;
uint16_t LFpin = 5;
uint16_t LBpin = 6;

// define robot movements
// slight left turn, while moving
void move_left()
{
    analogWrite(LFpin, fullSpeed * 0.5);
    analogWrite(LBpin, 0);
    analogWrite(RFpin, fullSpeed);
    analogWrite(RBpin, 0);
    // delay(200);
}
// 90-degree T-intersection left
void turn_left()
{
    analogWrite(LFpin, 0);
    analogWrite(LBpin, fullSpeed);
    analogWrite(RFpin, fullSpeed);
    analogWrite(RBpin, 0);
}
// slight right turn,while moving
void move_right()
{
    analogWrite(LFpin, fullSpeed);
    analogWrite(LBpin, 0);
    analogWrite(RBpin, 0);
    analogWrite(RFpin, fullSpeed * 0.5);
    // delay(200);
}
// 90-degree T-intersection turn
void turn_right()
{
    analogWrite(LBpin, 0);
    analogWrite(LFpin, fullSpeed);
    analogWrite(RBpin, fullSpeed);
    analogWrite(RFpin, 0);
}

void move_forward()
{
    analogWrite(LFpin, fullSpeed);
    analogWrite(LBpin, 0);
    analogWrite(RFpin, fullSpeed);
    analogWrite(RBpin, 0);
}

void move_back()
{
    analogWrite(LBpin, fullSpeed);
    analogWrite(LFpin, 0);
    analogWrite(RBpin, fullSpeed);
    analogWrite(RFpin, 0);
}

void move_stop()
{
    analogWrite(LFpin, 0);
    analogWrite(LBpin, 0);
    analogWrite(RFpin, 0);
    analogWrite(RBpin, 0);
}

void check_sensors()
{
    qtr.readLineBlack(sensorValues);
    // check if sensor has strong signal
    for (uint8_t i = 0; i < SensorCount; i++)
    {
        if (sensorValues[i] > sensorRange[i] / 3)
        {
            strongBlack[i] = true;
        }
        else
            strongBlack[i] = false;
    }
}

void find_lane()
{
    // lost line
    if (strongBlack[0] == false and strongBlack[1] == false and strongBlack[2] == false and strongBlack[3] == false and lastLeftTurn == true)
    {
        turn_left();
        // move_left();
    }
    if (strongBlack[0] == false and strongBlack[1] == false and strongBlack[2] == false and strongBlack[3] == false and lastRightTurn == true)
    {
        turn_right();
        // move_right();
    }
    if (strongBlack[0] == false and strongBlack[1] == false and strongBlack[2] == false and strongBlack[3] == false and lastRightTurn == false and lastLeftTurn == false)
    {
        turn_left();
        move_back();
    }
}

void follow_line()
{
    if (moveForwardCounter == 0 and moveLeftCounter == 0 and moveRightCounter == 0)
    {
        find_lane();
    }
    // oxxo
    if (strongBlack[1] == true and strongBlack[2] == true and strongBlack[0] == false and strongBlack[3] == false)
    {
        move_forward();
        completeLeftTurn = false;
        completeRightTurn = false;
        nowhereTimer = 0;
    }
    // xxoo
    if (strongBlack[1] == true and strongBlack[0] == true and strongBlack[2] == false and strongBlack[3] == false)
    {
        move_left();
        completeLeftTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = true;
        lastRightTurn = false;
    }
    // oxoo
    if (strongBlack[1] == true and strongBlack[0] == false and strongBlack[2] == false and strongBlack[3] == false)
    {
        move_left();
        completeLeftTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = true;
        lastRightTurn = false;
    }
    // xooo
    if (strongBlack[0] == true and strongBlack[1] == false and strongBlack[2] == false and strongBlack[3] == false)
    {
        move_left();
        completeLeftTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = true;
        lastRightTurn = false;
    }

    // ooxx
    if (strongBlack[2] == true and strongBlack[3] == true and strongBlack[0] == false and strongBlack[1] == false)
    {
        move_right();
        completeRightTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = false;
        lastRightTurn = true;
    }
    // ooxo
    if (strongBlack[2] == true and strongBlack[3] == false and strongBlack[0] == false and strongBlack[1] == false)
    {
        move_right();
        completeRightTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = false;
        lastRightTurn = true;
    }
    // ooox
    if (strongBlack[3] == true and strongBlack[2] == false and strongBlack[0] == false and strongBlack[1] == false)
    {
        move_right();
        completeRightTurn = true;
        nowhereTimer = 0;
        lastLeftTurn = false;
        lastRightTurn = true;
    }

    // xxxo Swift left
    while (strongBlack[0] == true and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == false)
    {
        completeLeftTurn = true;
        lastLeftTurn = true;
        lastRightTurn = false;
    completeLeftturn:
        move_left();
        delay(250);
        turn_left();
        delay(250);
        // delay(250);
        check_sensors();

        // if xxxx during turn
        if (strongBlack[0] == true and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == true and completeLeftTurn == true)
        {
            turn_left();
            delay(250);
            goto completeLeftturn;
        }
        if (strongBlack[0] == false and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == true and completeLeftTurn == false)
        {
            completeLeftTurn = false;
            nowhereTimer = 0;
            break;
        }
    }
    // oxxx Swift right
    while (strongBlack[0] == false and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == true)
    {
        completeRightTurn = true;
        lastLeftTurn = false;
        lastRightTurn = true;
    completeRightturn:
        move_right();
        delay(250);
        turn_right();
        delay(250);
        check_sensors();
        // if xxxx during turn
        if (strongBlack[0] == true and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == true and completeRightTurn == true)
        {
            turn_right();
            delay(250);
            goto completeRightturn;
        }

        if (strongBlack[0] == false and strongBlack[1] == true and strongBlack[2] == true and strongBlack[3] == true and completeRightTurn == false)
        {
            completeRightTurn = false;
            nowhereTimer = 0;
            break;
        }
    }
    find_lane();
}
// initial delay before starting robot movements and measurements
void setup()
{
    // configure the sensors
    qtr.setTypeAnalog();
    qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3}, SensorCount);
    qtr.setEmitterPin(3);

    // initiate motor controlling pins
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(10, OUTPUT);

    delay(3000);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

    // analogRead() takes about 0.1 ms on an AVR.
    // 0.1 ms per sensor * 4 samples per sensor read (default) * 4 sensors
    // * 10 reads per calibrate() call = ~16 ms per calibrate() call.
    // Call calibrate() 140 times to make calibration take about 2.24 seconds.
    for (uint16_t i = 0; i < 140; i++)
    {
        qtr.calibrate();
        turn_left();
    }
    move_stop();
    digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

    // print the calibration minimum values measured when emitters were on
    Serial.begin(9600);
    for (uint8_t i = 0; i < SensorCount; i++)
    {
        minimumValues[i] = qtr.calibrationOn.minimum[i];
        Serial.print(qtr.calibrationOn.minimum[i]);
        Serial.print(' ');
    }
    Serial.println();

    // print the calibration maximum values measured when emitters were on
    for (uint8_t i = 0; i < SensorCount; i++)
    {
        maximumValues[i] = qtr.calibrationOn.maximum[i];
        sensorRange[i] = maximumValues[i] - minimumValues[i];
        Serial.print(qtr.calibrationOn.maximum[i]);
        Serial.print(' ');
    }
    Serial.println();
    Serial.println();
    delay(1000);
}

void loop()
{
    // read calibrated sensor values and obtain a measure of the line position
    // from 0 to 5000 (for a white line, use readLineWhite() instead)
    uint16_t position = qtr.readLineBlack(sensorValues);
    check_sensors();
    // print the sensor values as numbers from 0 to 1000, where 0 means maximum
    // reflectance and 1000 means minimum reflectance, followed by the line
    // position
    for (uint8_t i = 0; i < SensorCount; i++)
    {
        // Serial.print(SensorCount);
        Serial.print(sensorValues[i]);
        Serial.print('\t');
    }
    Serial.println();
    follow_line();
    // Serial.println(position);
    /*  else
      {
        nowhereTimer=nowhereTimer+millis();
        Serial.println(nowhereTimer);
      }*/
}