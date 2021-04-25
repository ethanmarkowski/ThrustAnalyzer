/*
 Name:		ThrustAnalyzer.ino
 Created:	4/18/2021 9:42:55 PM
 Author:	Ethan Markowski
*/

void(* resetFunc) (void) = 0;

#include "CurrentSensor.h"
#include <Servo.h>
#include <HX711.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

CurrentSensor currentSensor(A13, 30);
HX711 loadCell;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Servo throttle_output;
File datalog;

#define BUTTONS A0
#define VOLTAGE_SENSE A15
#define THROTTLE_INPUT A14
#define THROTTLE_OUTPUT 44
#define SCALE_DOUT 23
#define SCALE_CLK 22
#define SD_CLK 52
#define SD_DO 50
#define SD_DI 51
#define SD_CS 53

int menu_position=0, test_enabled=0, button_state=0, button_transition=0, menu_h_scroll=0, menu_v_scroll=0, test_mode=0, test_status=0, lipo_type=4, current_limit=150, max_throttle=100, test_runtime=20, sd_enabled, sd_filename_selected=0;
float loadCell_calibration_factor = 105300, voltage, min_voltage=1000, thrust, max_thrust=0, throttle;
char input;
unsigned long start_time;


void setup() {
  Serial.begin(9600);

  //set up LCD display
  lcd.begin(16, 2);
  lcd.noCursor();

  //set up loadCell
  loadCell.begin(SCALE_DOUT, SCALE_CLK);
  loadCell.set_scale();

  //set up SD breakout
  pinMode(53,OUTPUT);

  //create servo object for throttle control output and activate arming signal
  throttle_output.attach(THROTTLE_OUTPUT);
  throttle_output.writeMicroseconds(1000); 
}

void loop() {
  if (menu_position!=3){
    menu();
  }
  else if (menu_position==3){
    switch (test_mode){
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

void button_input(void){
  input='0';
  
  /*Read buttons and store inputs*/
  button_state=analogRead(BUTTONS);
  if (button_transition==0&&button_state<900){
    if (button_state<50){
      input='r';
    }
    else if (button_state<300){
      input='u';
    }
    else if (button_state<500){
      input='d';
    }
    else if (button_state<700){
      input='l';
    }
    else if (button_state<900){
      input='s';
    }
    button_transition=1;
  }
  else if (button_state>900){
    button_transition=0;
  }
}

void calibrate_thrust_stand(void){
  loadCell.tare();
  loadCell.set_scale(loadCell_calibration_factor); //Adjust to this calibration factor
}

void thrust_reading(void){
  thrust=loadCell.get_units();
  if (thrust<0){
    thrust=0;
  }
  if (thrust>max_thrust){
    max_thrust=thrust;
  }
}

void voltage_reading(void){
  voltage = float(analogRead(VOLTAGE_SENSE))/1024/0.01795-0.01795;
  if (voltage<min_voltage){
    min_voltage=voltage;
  }
}

void current_safeguard(void){
  if (currentSensor.GetMaxValue() >= current_limit){
    throttle_output.writeMicroseconds(1000); //disable throttle
    test_status=6;
    lcd.clear();

    while (input!='s'){
      lcd.home();
      lcd.print("Overcurrent:");
      lcd.setCursor(0,1);
      lcd.print("Press Start");

      button_input();
    }

    lcd.clear();
  }
}

void voltage_safeguard(void){
  if (min_voltage<=lipo_type*3.2){
    throttle_output.writeMicroseconds(1000); //disable throttle
    test_status=6;
    lcd.clear();

    while (input!='s'){
      lcd.home();
      lcd.print("Low voltage");
      lcd.setCursor(0,1);
      lcd.print("Press Start");

      button_input();
    }
    lcd.clear();
  }
}

void manual_throttle(void){
  throttle=float(analogRead(THROTTLE_INPUT))/1023*100;
  throttle_output.writeMicroseconds(throttle/100*950 + 1050); 
}

void automatic_throttle(void){
  throttle=float((millis()-start_time))/1000/(test_runtime-5)*max_throttle;
  throttle=constrain(throttle,0,max_throttle);
  throttle_output.writeMicroseconds(throttle/100*950 + 1050);
  if (float(millis()-start_time)/1000>=test_runtime){
    throttle_output.writeMicroseconds(1000); //disable throttle
    test_status=6;
    lcd.clear();
  }
}

void data_dump(void){
  datalog.print(String(float(millis()-start_time)/1000)+"; ");
  datalog.print(String(throttle)+"; ");
  datalog.print(String(thrust)+"; ");
  datalog.print(String(currentSensor.GetRawValue())+"; ");
  datalog.print(String(voltage)+"; ");
  datalog.println(String(currentSensor.GetRawValue()*voltage));
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
    switch (test_mode){
      case 0:
      lcd.print("1 Auto Thrust");
      break;
      case 1:
      lcd.print("2 Manual Thrust");
      break;
    }

    button_input();
    if (input=='u'&&test_mode<1){
      test_mode++;
      lcd.clear();
    }
    else if (input=='d'&&test_mode>0){
      test_mode--;
      lcd.clear();
    }
    else if (input=='r'){
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

    button_input();
    if (input=='u'&&lipo_type<12){
      lipo_type++;
    }
    else if (input=='d'&&lipo_type>1){
      lipo_type--;
    }
    else if (input=='l'){
      menu_position--;
      lcd.clear();
    }
    else if (input=='r'){
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

    button_input();
    if (input=='u'&&current_limit<150){
      current_limit+=5;
    }
    else if (input=='d'&&current_limit>5){
      current_limit-=5;
    }
    else if (input=='l'){
      menu_position--;
      lcd.clear();
    }
    else if (input=='r'){
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
  lcd.print(max_throttle);
  lcd.print("%  ");

  button_input();

  if (input=='u'&&max_throttle<100){
    max_throttle+=5;
  }
  else if (input=='d'&&max_throttle>5){
     max_throttle-=5;
  }
  else if (input=='r'){
    test_status=1;
    lcd.clear();
  }
  else if (input=='l'){
    menu_position--;
    lcd.clear();
  }
}

void set_test_duration(void){
  lcd.home();
  lcd.print("Set Test Runtime");
  lcd.setCursor(0,1);
  lcd.print(test_runtime);
  lcd.print("s ");

  button_input();

  if (input=='u'&&test_runtime<60){
    test_runtime+=5;
  }
  else if (input=='d'&&test_runtime>10){
    test_runtime-=5;
  }
  else if (input=='r'){
    test_status=2;
    lcd.clear();
  }
  else if (input=='l'){
    test_status--;
    lcd.clear();
  }
}

void calibration(void){
  lcd.home();
  lcd.print("Calibration:");
  lcd.setCursor(0,1);
  lcd.print("Press Start");

  button_input();

  if (input=='s'){
    calibrate_thrust_stand();
    currentSensor.Calibrate();

    test_status=3;
    lcd.clear();
  }
  else if (input=='l'){
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

  button_input();

  if (input=='s'){
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

  else if (input=='l'){
    test_status--;
    lcd.clear();
  }
}

void test_start_screen(void){
  throttle_output.writeMicroseconds(1000);
        
  lcd.home();
  lcd.print("Test Ready");
  lcd.setCursor(0,1);
  lcd.print("Press Start");

  button_input();

  if (input=='l'){
    test_status--;
    lcd.clear();
  }
  else if (input=='s'){
    test_status=5;
    lcd.clear();
    start_time=millis();
  }
}

void thrust_test(void){
  button_input(); 

  if (input=='s'){
    throttle_output.writeMicroseconds(1000); //disable throttle
    test_status=6;          
    lcd.clear();
  }
    
  if (test_mode==0){
    automatic_throttle();
  }
  else if (test_mode==1){
    manual_throttle();
  }

  //writes data to file if SD file storage is enabled
  if (sd_enabled==1){
    data_dump();
  }
    
  thrust_reading();
  currentSensor.Read();
  voltage_reading();
  current_safeguard();
  voltage_safeguard();

  lcd.home();

  //thrust reading display
  lcd.print(thrust, 2);
  lcd.print("lb  ");

  //throttle percentage display
  lcd.setCursor(9,0);
  lcd.print(throttle, 0);
  lcd.print("%   ");

  //Current draw display
  lcd.setCursor(0,1);
  lcd.print(float(round(2*currentSensor.GetValue()))/2,1);
  lcd.print("A   ");

  //power draw display
  lcd.setCursor(9,1);
  //lcd.print();
  lcd.print(round(voltage*currentSensor.GetValue()));
  lcd.print("W   ");
}

void thrust_test_summary(void){
  lcd.home();

  //max thrust display
  lcd.print(max_thrust, 2);
  lcd.print("lb  ");

  //efficiency at max power
  lcd.setCursor(8,0);
  lcd.print(453.592*max_thrust/(currentSensor.GetMaxValue()*min_voltage));
  lcd.print("g/W");

  //max current draw display
  lcd.setCursor(0,1);
  lcd.print(currentSensor.GetMaxValue(),2);
  lcd.print("A    ");

  //max power display
  lcd.setCursor(8,1);
  lcd.print(currentSensor.GetMaxValue()*min_voltage,0);
  lcd.print("W   ");

  button_input();

  if (input=='s'){
    if (sd_enabled==1){
      datalog.close(); //closes and saves datalog file on SD
    }
    resetFunc(); //reset program
  }
}