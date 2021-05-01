/*
 Name:		ThrustAnalyzer.ino
 Created:	4/18/2021 9:42:55 PM
 Author:	Ethan Markowski
*/

void(*resetFunc) (void) = 0;

#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include "src/SDLogger.h"
#include "src/AnalogButtons.h"
#include "src/Throttle.h"
#include "src/ThrustStand.h"
#include "src/AnalogSensor.h"

#define SD_CLK 52
#define SD_DO 50
#define SD_DI 51
#define SD_CS 53

enum Pins { POT_PIN = A14, BUTTON_PIN = A0, THRUST_DOUT_PIN = 23, THRUST_SCK_PIN = 22, CURRENT_PIN = A13, VOLTAGE_PIN = A15, ESC_PIN = 44 };
enum SensorSmoothing { THRUST_SMOOTHING = 1, CURRENT_SMOOTHING = 30, VOLTAGE_SMOOTHING = 1 };

HX711 loadCell;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
AnalogButtons button(BUTTON_PIN);
Throttle throttle(ESC_PIN, POT_PIN);
File datalog;
ISensor *thrustStand = new ThrustStand(THRUST_DOUT_PIN, THRUST_SCK_PIN, 105300, THRUST_SMOOTHING);
ISensor *currentSensor = new AnalogSensor(CURRENT_PIN, true, 0.04, CURRENT_SMOOTHING, 50, 0);
ISensor *voltageSensor = new AnalogSensor(VOLTAGE_PIN, false, 0.08975, VOLTAGE_SMOOTHING, 50, 0);

ISensor *sensors[] = { thrustStand, currentSensor, voltageSensor };

Buttons input;
int menu_position=0, test_status=0, lipo_type=4, current_limit=150, sd_enabled, sd_filename_selected=0;
unsigned long start_time;

void setup() {
  Serial.begin(9600);

  //set up LCD display
  lcd.begin(16, 2);
  lcd.noCursor();

  //set up SD breakout
  pinMode(53,OUTPUT);
}

void loop() {
  if (menu_position!=3){
    menu();
  }
  else if (menu_position==3){
    switch (throttle.GetMode()){
      case 0: auto_thrust_test();
      break;
      case 1: manual_thrust_test();
      break;
    }
  }
}

void auto_thrust_test(void){
  /*Set Maximum Throttle*/
  if (test_status==0){
    set_max_thrust();
  }

  /*Set Test Duration*/
  else if (test_status==1){
    set_test_duration();
  }

  /*Calibrate*/
  else if (test_status==2){
    calibration();
  }

  /*SD Card Setup Screen*/
  else if (test_status==3){
    sd_setup();
  }

  /*Test Start Screen*/
  else if (test_status==4){
    test_start_screen();
  }

  /*Run Test*/
  else if (test_status==5){
    thrust_test();
  }

  /*Display Summary Results*/
  else if (test_status==6){
    thrust_test_summary();
  }
}

void manual_thrust_test(void){
  
  /*Calibrate Thrust Stand and Current Sensor*/
  if (test_status==0){
    calibration();
  }

  /*BRIDGE*/
  else if (test_status==2){
    test_status=0;
  }

  /*BRDIGE*/
  else if (test_status==-1){
    test_status=0;
    menu_position=2;
  }

  /*SD Card Setup Screen*/
  else if (test_status==3){
    sd_setup();
  }
  
  /*Test Start Screen*/
  else if (test_status==4){
    test_start_screen();
  }


  /*Run Test*/
  else if (test_status==5){
    thrust_test();
  }

  /*Display summary results*/
  else if (test_status==6){
    thrust_test_summary();
  }
}

void data_dump(void){
  datalog.print(String(float(millis()-start_time)/1000)+"; ");
  datalog.print(String(throttle.GetThrottle())+"; ");
  datalog.print(String(thrustStand->GetRawValue())+"; ");
  datalog.print(String(currentSensor->GetRawValue())+"; ");
  datalog.print(String(voltageSensor->GetValue())+"; ");
  datalog.println(String(currentSensor->GetRawValue()*voltageSensor->GetValue()));
}

/*Setup menu*/
void menu(void){

  /*Menu Tier*/
  switch(menu_position){

    /*Select Mode*/
    case 0:
    lcd.home();
    lcd.print("Select Mode:");
    lcd.setCursor(0,1);
    switch (throttle.GetMode()){
      case 0:
      lcd.print("1 Auto Thrust");
      break;
      case 1:
      lcd.print("2 Manual Thrust");
      break;
    }

    input = button.GetInput();
    if (input==Buttons::UP&&throttle.GetMode()<1){
      lcd.clear();
    }
    else if (input==Buttons::DOWN&&throttle.GetMode()>0){
      lcd.clear();
    }
    else if (input==Buttons::RIGHT){
      menu_position=1;
      lcd.clear();
    }
    break;

    /*Set LIPO battery type */
    case 1:
    lcd.home();
    lcd.print("Set LIPO type:");
    lcd.setCursor(0,1);
    lcd.print(lipo_type);
    lcd.print("s ");

    input = button.GetInput();
    if (input==Buttons::UP&&lipo_type<12){
      lipo_type++;
    }
    else if (input==Buttons::DOWN&&lipo_type>1){
      lipo_type--;
    }
    else if (input==Buttons::LEFT){
      menu_position--;
      lcd.clear();
    }
    else if (input==Buttons::RIGHT){
      menu_position++;
      lcd.clear();
    }
    break;

    /* Set Current Limit*/
    case 2:
    lcd.home();
    lcd.print("Set Max Limit:");
    lcd.setCursor(0,1);
    lcd.print(current_limit);
    lcd.print("A  ");

    input = button.GetInput();
    if (input==Buttons::UP&&current_limit<150){
      current_limit+=5;
    }
    else if (input==Buttons::DOWN&&current_limit>5){
      current_limit-=5;
    }
    else if (input==Buttons::LEFT){
      menu_position--;
      lcd.clear();
    }
    else if (input==Buttons::RIGHT){
      menu_position++;
      lcd.clear();
    }
    break;   
  }
}

/*Thrust test states*/
void set_max_thrust(void){
  lcd.home();
  lcd.print("Set Max Throttle");
  lcd.setCursor(0,1);
  lcd.print(throttle.GetAutoMaxThrottle());
  lcd.print("%  ");

  input = button.GetInput();
}

void set_test_duration(void){
  lcd.home();
  lcd.print("Set Test Runtime");
  lcd.setCursor(0,1);
  lcd.print(throttle.GetAutoRunTime());
  lcd.print("s ");

  input = button.GetInput();
}

void calibration(void){
  lcd.home();
  lcd.print("Calibration:");
  lcd.setCursor(0,1);
  lcd.print("Press Start");

  input = button.GetInput();

  if (input==Buttons::SELECT){
    thrustStand->Calibrate();
    currentSensor->Calibrate();

    test_status=3;
    lcd.clear();
  }
  else if (input==Buttons::LEFT){
    test_status--;
    lcd.clear();
  }
}

void sd_setup(void){
  if (SD.begin()){
    lcd.home();
    lcd.print("SD Initialized");
    lcd.setCursor(0,1);
    lcd.print("Press Start");
    sd_enabled=1;
  }
  else if (!SD.begin()){
    lcd.home();
    lcd.print("No SD Detected");
    lcd.setCursor(0,1);
    lcd.print("Press Start");
    sd_enabled=0;
  }

  Serial.println(sd_enabled);

  input = button.GetInput();

  if (input==Buttons::SELECT){
    if (sd_enabled==1){
      int i=1;
      while (sd_filename_selected==0){
        if (!SD.exists("data_" + String(i) + ".txt")){
          datalog = SD.open("data_" + String(i) + ".txt", FILE_WRITE);
          sd_filename_selected=1;
          datalog.println("time(s); throttle(%); thrust(lb); current(A); voltage(V); power(W)");
        }
        else {
          i++;
        }
      }
    }

    test_status=4;
    lcd.clear();
  }

  else if (input==Buttons::LEFT){
    test_status--;
    lcd.clear();
  }
}

void test_start_screen(void){
    throttle.Run();
        
  lcd.home();
  lcd.print("Test Ready");
  lcd.setCursor(0,1);
  lcd.print("Press Start");

  input = button.GetInput();

  if (input==Buttons::LEFT){
    test_status--;
    lcd.clear();
  }
  else if (input==Buttons::SELECT){
    test_status=5;
    lcd.clear();
    start_time=millis();
  }
}

void thrust_test(void){
  input = button.GetInput(); 

  if (input==Buttons::SELECT){
      throttle.Disarm(); //disable throttle
    test_status=6;          
    lcd.clear();
  }

  //writes data to file if SD file storage is enabled
  if (sd_enabled==1){
    data_dump();
  }
    
  currentSensor->Update();
  voltageSensor->Update();

  lcd.home();

  //thrust reading display
  lcd.print(thrustStand->GetValue(), 2);
  lcd.print("lb  ");

  //throttle percentage display
  lcd.setCursor(9,0);
  lcd.print((int)throttle.GetThrottle() * 100, 0);
  lcd.print("%   ");

  //Current draw display
  lcd.setCursor(0,1);
  lcd.print(float(round(2*currentSensor->GetValue()))/2,1);
  lcd.print("A   ");

  //power draw display
  lcd.setCursor(9,1);
  //lcd.print();
  lcd.print(round(voltageSensor->GetValue()*currentSensor->GetValue()));
  lcd.print("W   ");
}

void thrust_test_summary(void){
  lcd.home();

  //max thrust display
  lcd.print(thrustStand->GetMaxValue(), 2);
  lcd.print("lb  ");

  //efficiency at max power
  lcd.setCursor(8,0);
  lcd.print(453.592* thrustStand->GetMaxValue() /(currentSensor->GetMaxValue()*voltageSensor->GetMinValue()));
  lcd.print("g/W");

  //max current draw display
  lcd.setCursor(0,1);
  lcd.print(currentSensor->GetMaxValue(),2);
  lcd.print("A    ");

  //max power display
  lcd.setCursor(8,1);
  lcd.print(currentSensor->GetMaxValue()*voltageSensor->GetMinValue(),0);
  lcd.print("W   ");

  input = button.GetInput();

  if (input==Buttons::SELECT){
    if (sd_enabled==1){
      datalog.close(); //closes and saves datalog file on SD
    }
    resetFunc(); //reset program
  }
}