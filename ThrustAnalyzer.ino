/*
 Name:		ThrustAnalyzer.ino
 Created:	4/18/2021 9:42:55 PM
 Author:	Ethan Markowski
*/

void(*resetProgram) (void) = 0;

#include <LiquidCrystal.h>
#include "src/SDLogger.h"
#include "src/AnalogButtons.h"
#include "src/Throttle.h"
#include "src/ThrustStand.h"
#include "src/AnalogSensor.h"

enum Pins { SD_CLK_PIN = 52, SD_DO_PIN = 50, SD_DI_PIN = 51, SD_CS_PIN = 53, POT_PIN = A14, BUTTON_PIN = A0, THRUST_DOUT_PIN = 23, THRUST_SCK_PIN = 22, CURRENT_PIN = A13, VOLTAGE_PIN = A15, ESC_PIN = 44 };
enum SensorSmoothing { THRUST_SMOOTHING = 1, CURRENT_SMOOTHING = 30, VOLTAGE_SMOOTHING = 1 };
const float cellMinVoltage = 3.0; // Establishes the minimum voltage cutoff per LIPO cell

enum class ProgramStates { SET_MODE, SET_LIPO_CELL_COUNT, SET_CURRENT_LIMIT, SET_MAX_THROTTLE, SET_NUM_STEPS, SET_TEST_RUNTIME, CHECK_SD_CARD, SETUP_LOG_FILE, ARM_AND_CALIBRATE, TEST_START_SCREEN, RUN_TEST, HIGH_CURRENT_WARNING, LOW_VOLTAGE_WARNING, TEST_SUMMARY };

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
SDLogger sdLogger(SD_CS_PIN);
AnalogButtons button(BUTTON_PIN);
Throttle throttle(ESC_PIN, POT_PIN);
ISensor *thrustStand = new ThrustStand(THRUST_DOUT_PIN, THRUST_SCK_PIN, 105300, THRUST_SMOOTHING);
ISensor *currentSensor = new AnalogSensor(CURRENT_PIN, true, 0.04, CURRENT_SMOOTHING, 50, 1);
ISensor *voltageSensor = new AnalogSensor(VOLTAGE_PIN, false, 0.08975, VOLTAGE_SMOOTHING, 50, 0);

ISensor *sensors[] = { thrustStand, currentSensor, voltageSensor };

ProgramStates state = ProgramStates::SET_MODE;
Buttons userInput;
uint32_t testStartTime;

void setup()
{
    // Set up LCD display
    lcd.begin(16, 2);
    lcd.noCursor();

    // Test program presets
    voltageSensor->SetLowerSafeguard(4 * cellMinVoltage); // Default LIPO battery configuration is set to 4s
    currentSensor->SetUpperSafeguard(50);
}

void loop() 
{
    // Retrieve a button input
    userInput = button.GetInput();

    // Program states
    switch (state)
    {
        // Select between manual and automatic throttle control
        case ProgramStates::SET_MODE:
            setMode();
            if (userInput == Buttons::RIGHT) { state = ProgramStates::SET_LIPO_CELL_COUNT; lcd.clear(); }
            break;
        
        // Set number of cells in LIPO battery
        case ProgramStates::SET_LIPO_CELL_COUNT:
            setLipoCellCount();
            if (userInput == Buttons::RIGHT) { state = ProgramStates::SET_CURRENT_LIMIT; lcd.clear(); }
            else if (userInput == Buttons::LEFT) { state = ProgramStates::SET_MODE; lcd.clear(); }
            break;
        
        // Set current limit
        case ProgramStates::SET_CURRENT_LIMIT:
            setCurrentLimit();
            if (userInput == Buttons::RIGHT)
            {
                switch (throttle.GetMode())
                {
                    case ThrottleHelper::ThrottleModes::POTINPUT: state = ProgramStates::CHECK_SD_CARD; lcd.clear(); break;
                    case ThrottleHelper::ThrottleModes::AUTO: state = ProgramStates::SET_MAX_THROTTLE; lcd.clear(); break;
                }
            }

            else if (userInput == Buttons::LEFT) { state = ProgramStates::SET_LIPO_CELL_COUNT; lcd.clear(); }
            break;

        // Set max throttle parameter for auto throttle control
        case ProgramStates::SET_MAX_THROTTLE:
            setMaxThrottle();
            if (userInput == Buttons::RIGHT) { state = ProgramStates::SET_NUM_STEPS; lcd.clear(); }
            else if (userInput == Buttons::LEFT) { state = ProgramStates::SET_CURRENT_LIMIT; lcd.clear(); }
            break;

        // Set number of throttle steps for auto throttle control
        case ProgramStates::SET_NUM_STEPS:
            setNumSteps();
            if (userInput == Buttons::RIGHT) { state = ProgramStates::SET_TEST_RUNTIME; lcd.clear(); }
            else if (userInput == Buttons::LEFT) { state = ProgramStates::SET_MAX_THROTTLE; lcd.clear(); }
            break;

        // Set runtime for auto throttle control
        case ProgramStates::SET_TEST_RUNTIME:
            setTestRuntime();
            if (userInput == Buttons::RIGHT) { state = ProgramStates::CHECK_SD_CARD; lcd.clear(); }
            else if (userInput == Buttons::LEFT) { state = ProgramStates::SET_MAX_THROTTLE; lcd.clear(); }
            break;

        // Check SD card. Allow user to skip SD logger setup to run a test without data logging
        case ProgramStates::CHECK_SD_CARD:
            checkSDCard();
            if (userInput == Buttons::START && sdLogger.CheckCard())
            {
                sdLogger.Enable();
                state = ProgramStates::SETUP_LOG_FILE;
                lcd.clear();
            }

            else if (userInput == Buttons::RIGHT)
            {
                sdLogger.Disable();
                state = ProgramStates::ARM_AND_CALIBRATE;
                lcd.clear();
            }

            else if (userInput == Buttons::LEFT)
            {
                switch (throttle.GetMode())
                {
                    case ThrottleHelper::ThrottleModes::POTINPUT: state = ProgramStates::SET_CURRENT_LIMIT; lcd.clear(); break;
                    case ThrottleHelper::ThrottleModes::AUTO: state = ProgramStates::SET_TEST_RUNTIME; lcd.clear(); break;
                }
            }

            break;

        // Set up log file for the SD data logger
        case ProgramStates::SETUP_LOG_FILE:
            setupLogFile();
            if (userInput == Buttons::RIGHT && !sdLogger.GetFilename().equals("")) { state = ProgramStates::ARM_AND_CALIBRATE; lcd.clear(); }
            else if (userInput == Buttons::LEFT)
            {
                switch (throttle.GetMode())
                {
                    case ThrottleHelper::ThrottleModes::POTINPUT: state = ProgramStates::SET_CURRENT_LIMIT; lcd.clear(); break;
                    case ThrottleHelper::ThrottleModes::AUTO: state = ProgramStates::SET_TEST_RUNTIME; lcd.clear(); break;
                }
            }

            break;

        // Calibrate sensors and arm ESC
        case ProgramStates::ARM_AND_CALIBRATE:
            armAndCalibrate();
            if (userInput == Buttons::START)
            {
                for (auto sensor : sensors) { sensor->Calibrate(); }
                throttle.Arm();
                state = ProgramStates::TEST_START_SCREEN;
                lcd.clear();
            }

            else if (userInput == Buttons::LEFT)
            {
                (sdLogger.GetIsEnabled()) ? state = ProgramStates::SETUP_LOG_FILE : state = ProgramStates::CHECK_SD_CARD;
                lcd.clear();
            }
            break;

        // Test start screen
        case ProgramStates::TEST_START_SCREEN:
            testStartScreen();
            if (userInput == Buttons::START)
            {
                testStartTime = millis();
                state = ProgramStates::RUN_TEST;
                lcd.clear();
            }

            else if (userInput == Buttons::LEFT) 
            { 
                throttle.Disarm();
                state = ProgramStates::ARM_AND_CALIBRATE;
                lcd.clear();
            }

            break;

        // Run test
        case ProgramStates::RUN_TEST:
            runTest();

            if (userInput == Buttons::START)
            {
                throttle.Disarm();
                if (sdLogger.GetIsEnabled()) { sdLogger.Save(); }
                state = ProgramStates::TEST_SUMMARY;
                lcd.clear();
            }

            // Automatic test completion when throttle is in auto mode
            else if (throttle.GetCompletionStatus())
            {
                    throttle.Disarm();
                    if (sdLogger.GetIsEnabled()) { sdLogger.Save(); }
                    state = ProgramStates::TEST_SUMMARY;
                    lcd.clear();
            }

            else if (currentSensor->GetMaxValue() >= currentSensor->GetUpperSafeguard())
            {
                throttle.Disarm();
                if (sdLogger.GetIsEnabled()) { sdLogger.Save(); }
                state = ProgramStates::HIGH_CURRENT_WARNING;
                lcd.clear();
            }

            else if (voltageSensor->GetMinValue() <= voltageSensor->GetLowerSafeguard())
            {
                throttle.Disarm();
                if (sdLogger.GetIsEnabled()) { sdLogger.Save(); }
                state = ProgramStates::LOW_VOLTAGE_WARNING;
                lcd.clear();
            }

            break;

        // High current warning
        case ProgramStates::HIGH_CURRENT_WARNING:
            highCurrentWarning();
            if (userInput == Buttons::START) { state = ProgramStates::TEST_SUMMARY; lcd.clear(); }
            break;

        // Low voltage warning
        case ProgramStates::LOW_VOLTAGE_WARNING:
            lowVoltageWarning();
            if (userInput == Buttons::START) { state = ProgramStates::TEST_SUMMARY; lcd.clear(); }
            break;

        // Test results summary
        case ProgramStates::TEST_SUMMARY:
            testSummary();
            if (userInput == Buttons::START) { resetProgram(); }
            break;
    }
}

void setMode()
{
    lcd.setCursor(0, 0);
    lcd.print("Mode:");
    lcd.setCursor(0, 1);
    lcd.print(throttle.GetModeString());

    if (userInput == Buttons::UP) { throttle.IncrementMode(); lcd.clear(); }
    else if (userInput == Buttons::DOWN) { throttle.DecrementMode(); lcd.clear(); }
}

void setLipoCellCount()
{
    // Back calculates the LIPO cell count based on the voltage sensor's lower safeguard value 
    uint8_t lipoNumCells = voltageSensor->GetLowerSafeguard() / cellMinVoltage;

    lcd.setCursor(0, 0);
    lcd.print("LIPO cell count:");
    lcd.setCursor(0, 1);
    lcd.print(String(lipoNumCells) + "s");

    // Allows user to change the battery cell count setting and sets the voltage sensor's lower safeguard to the corresponding low voltage cutoff level
    if (userInput == Buttons::UP) { voltageSensor->SetLowerSafeguard((lipoNumCells + 1) * cellMinVoltage); lcd.clear(); }
    else if (userInput == Buttons::DOWN) { voltageSensor->SetLowerSafeguard(max(lipoNumCells - 1, 1) * cellMinVoltage); lcd.clear(); } // max function prohibits cell counts < 1
}

void setCurrentLimit()
{
    float currentLimit = currentSensor->GetUpperSafeguard();

    lcd.setCursor(0, 0);
    lcd.print("Current limit:");
    lcd.setCursor(0, 1);
    lcd.print(String(currentLimit) + " A");

    // Allow user to increase and decrease the current limit by single Amp increments when the parameter is less than 10 amps
    // When the setting is 10 amps or more, the increment interval becomes 5 amps
    if (userInput == Buttons::UP) { currentSensor->SetUpperSafeguard(incrementParameter(currentLimit)); lcd.clear(); }

    else if (userInput == Buttons::DOWN) { currentSensor->SetUpperSafeguard(decrementParameter(currentLimit)); lcd.clear(); }
}

void setMaxThrottle()
{
    float maxThrottle = throttle.GetAutoMaxThrottle();

    lcd.setCursor(0, 0);
    lcd.print("Max throttle:");
    lcd.setCursor(0, 1);
    lcd.print(String(maxThrottle * 100) + "%");

    // Allow user to increase and decrease the throttle limit by one percent point when the parameter is less than 10%
    // When the setting is above 10%, the increment interval becomes 5 percent points
    if (userInput == Buttons::UP) { throttle.SetAutoMaxThrottle(incrementParameter(maxThrottle * 100) / 100); lcd.clear(); }
    else if (userInput == Buttons::DOWN) { throttle.SetAutoMaxThrottle(decrementParameter(maxThrottle * 100) / 100); lcd.clear(); }
}

void setNumSteps()
{
    uint8_t numSteps = throttle.GetAutoThrottleNumSteps();

    lcd.setCursor(0, 0);
    lcd.print("Throttle steps:");
    lcd.setCursor(0, 1);
    lcd.print(numSteps);

    if (userInput == Buttons::UP) { throttle.SetAutoThrottleNumSteps(numSteps + 1); lcd.clear(); }
    else if (userInput == Buttons::DOWN) { throttle.SetAutoThrottleNumSteps(numSteps - 1); lcd.clear(); }
}

void setTestRuntime()
{
    uint32_t runtime = throttle.GetAutoRunTime() / 1000; // in units of seconds

    lcd.setCursor(0, 0);
    lcd.print("Test runtime:");
    lcd.setCursor(0, 1);
    lcd.print(String(runtime) + " s");

    // Allow user to increase and decrease the test runtime by one second when the parameter is less than 10 seconds
    // When the setting is above 10 s, the increment interval becomes 5 second
    if (userInput == Buttons::UP) { throttle.SetAutoRunTime(incrementParameter(runtime) * 1000); lcd.clear();}
    else if (userInput == Buttons::DOWN) { throttle.SetAutoRunTime(decrementParameter(runtime) * 1000); lcd.clear();}
}

void checkSDCard()
{
    switch (sdLogger.CheckCard())
    {
        case false:
            lcd.setCursor(0, 0);
            lcd.print("No card detected");
            lcd.setCursor(0, 1);
            lcd.print("                ");
            break;

        case true:
            lcd.setCursor(0, 0);
            lcd.print("Card detected   ");
            lcd.setCursor(0, 1);
            lcd.print("Press Start     ");
            break;
    }
}

void setupLogFile()
{
    if (sdLogger.GetFilename().equals(""))
    {
        sdLogger.CreateNewFile();
        String dataHeaders[] = { "Time (s)", "Throttle (%)", "Thrust (lb)", "Current (A)", "Voltage (V)", "Power (W)" };
        sdLogger.Log(dataHeaders, 6);
    }

    lcd.setCursor(0, 0);
    lcd.print("Log filename:");
    lcd.setCursor(0, 1);
    lcd.print(sdLogger.GetFilename());
}

void armAndCalibrate()
{
    lcd.setCursor(0, 0);
    lcd.print("Arm & calibrate:");
    lcd.setCursor(0, 1);
    lcd.print("Press Start");
}

void testStartScreen()
{
    lcd.setCursor(0, 0);
    lcd.print("Test ready:");
    lcd.setCursor(0, 1);
    lcd.print("Press Start");
}

void runTest()
{
    float runtime = (float)(millis() - testStartTime) / 1000;
    throttle.Run();
    for (auto sensor : sensors) { sensor->Update(); }

    // Print results to LCD display
    lcd.setCursor(0, 0);
    lcd.print(String(thrustStand->GetValue(), 2) + "lb  ");
    lcd.setCursor(9, 0);
    lcd.print(String(throttle.GetThrottle() * 100, 0) + "%   ");
    lcd.setCursor(0, 1);
    lcd.print(String(max(currentSensor->GetValue(), 1), 0) + "A   ");
    lcd.setCursor(9, 1);
    lcd.print(String(max(round(voltageSensor->GetValue() * currentSensor->GetValue()), 0) + "W   "));

    // Log data to SD card if enabled
    if (sdLogger.GetIsEnabled())
    {
        String data[] = { String(runtime), String(throttle.GetThrottle()), String(thrustStand->GetRawValue()), String(currentSensor->GetRawValue()), String(voltageSensor->GetRawValue()), String(currentSensor->GetRawValue() * voltageSensor->GetRawValue()) };
        sdLogger.Log(data, 6);
    }
}

void highCurrentWarning()
{
    lcd.setCursor(0, 0);
    lcd.print("Warning:");
    lcd.setCursor(0, 1);
    lcd.print("High current");
}

void lowVoltageWarning()
{
    lcd.setCursor(0, 0);
    lcd.print("Warning:");
    lcd.setCursor(0, 1);
    lcd.print("Low voltage");
}

void testSummary()
{
    lcd.setCursor(0, 0);
    lcd.print(String(thrustStand->GetMaxValue(), 2) + "lb");
    lcd.setCursor(8,0);
    lcd.print(String(453.592 * thrustStand->GetMaxValue() / (currentSensor->GetMaxValue() * voltageSensor->GetMinValue())) + "g/W");
    lcd.setCursor(0,1);
    lcd.print(String(currentSensor->GetMaxValue(),2) + "A");
    lcd.setCursor(8,1);
    lcd.print(String(currentSensor->GetMaxValue() * voltageSensor->GetMinValue(), 0) + "W");
}

// Increments and returns a parameter. If the parameter is equal to or greater than 10, the parameter is incremented by 5. Otherwise, the parameter is incremented by 1.
float incrementParameter(const float &value)
{
    if (value >= 10) { return value + 5; }
    else { return value + 1; }
}

// Decrements and returns a parameter. If the parameter is equal to or greater than 15, the parameter is decremented by 5. Otherwise, the parameter is decremented by 1.
float decrementParameter(const float &value)
{
    if (value >= 15) { return value - 5; }
    else { return value - 1; }
}