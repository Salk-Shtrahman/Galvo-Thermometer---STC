#include <LiquidCrystal.h>
#include <SPI.h>

// heat pad control pin
#define HEATPADPIN 3
// thermistor1
#define THERMISTORPIN A0  
// set the temperature
#define SETPOINT 38.5
#define HYSTERESIS_RANGE .125
// resistance at 25 degrees C
#define THERMISTORNOMINAL 4700      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 25
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3500
// the value of the 'other' resistor
#define SERIESRESISTOR 9790  

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int samples[500];
char poopy1[4];
char poopy2[4];
char poopy3[4]={'#','#','.','#'};
char poopy4[4]={'#','#','.','#'};
unsigned char PWM=75;
float steinhart_new;
float steinhart_old;
unsigned char counter=0;

void setup(void) { 
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  dtostrf(SETPOINT,3,1,poopy1);
  lcd.print("SET POINT:  ");
  lcd.print(poopy1);
  lcd.setCursor(1,3);
  lcd.print("ACTUAL T:");
  
  Serial.begin(9600);
  
  analogReference(EXTERNAL);
  pinMode(HEATPADPIN,OUTPUT);
  digitalWrite(HEATPADPIN,HIGH);
  pinMode(THERMISTORPIN, INPUT);
}
 
void loop(void) {
  uint8_t i;
  float average;
  // take N samples in a row, with a slight delay
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   average += samples[i];
   delay(1);
  }
  average /= NUMSAMPLES;
 
  Serial.print("\nanalog: "); 
  Serial.println(average);
 
  // convert the value to resistance
  ////////////////////////average = 1023 / average - 1;
  average=1023/average;
  average = SERIESRESISTOR / average;
  Serial.print("resist: "); 
  Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  Serial.print("temp: "); 
  Serial.println(steinhart);
  
//  if (steinhart<=(float)SETPOINT){digitalWrite(HEATPADPIN,HIGH); Serial.println("Heat On");}
//  else{digitalWrite(HEATPADPIN,LOW); Serial.println("Heat Off");}
  
  steinhart_new=steinhart;
  if (steinhart_new <(float)SETPOINT-HYSTERESIS_RANGE) {digitalWrite(HEATPADPIN,HIGH);}
  else if (steinhart_new < (float)SETPOINT+HYSTERESIS_RANGE && steinhart_new > (float)SETPOINT-HYSTERESIS_RANGE) {
    if (steinhart_new < steinhart_old && PWM<255 && steinhart_new <(float)SETPOINT) {PWM++;}
    else if (steinhart_new > steinhart_old && PWM>0 && steinhart_new >(float)SETPOINT) {PWM--;}
    analogWrite(HEATPADPIN,PWM);}
  else if (steinhart_new > (float)SETPOINT+HYSTERESIS_RANGE) {digitalWrite(HEATPADPIN,LOW);}
  steinhart_old=steinhart_new;

  Serial.print("PWM: "); 
  Serial.println(PWM);
  
  counter++;
  if(counter>5){
    counter=0;
    dtostrf(steinhart,3,1,poopy2);
    lcd.setCursor(12,1);
    lcd.print(poopy2);
  //  lcd.setCursor(6,1);
  //  lcd.print(poopy3);
  //  lcd.setCursor(12,1);
  //  lcd.print(poopy4);
  }
}
