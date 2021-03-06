#include "STC/STC12C5A60S2.H"
#include <stdio.h>
#include <intrins.h>
#include <math.h>
#include <stdlib.h>

#define FOSC    22118400L 
#define BAUD    9600

#define DataPort P2
sbit rs=P0^0;
sbit e=P0^2;
sbit rw=P0^1;
/*Define ADC operation const for ADC_CONTR*/
#define ADC_POWER   0x80            //ADC power control bit
#define ADC_FLAG    0x10            //ADC complete flag
#define ADC_START   0x08            //ADC start control bit
#define ADC_SPEEDLL 0x00            //420 clocks
#define ADC_SPEEDL  0x20            //280 clocks
#define ADC_SPEEDH  0x40            //140 clocks
#define ADC_SPEEDHH 0x60            //70 clocks
#define sample_size 100

#define THERMISTORNOMINAL 4700    // resistance at 25 degrees C  
#define TEMPERATURENOMINAL 25		// temp. for nominal resistance (almost always 25 C)
#define BCOEFFICIENT 3500  // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 9860  // the value of the 'other' resistor

////////////////////////////WIRINGS///////////////////////////////

//STC <--> LCD:
//39 <--> 4
//38 <--> 5
//37 <--> 6
//	 and
//28 <--> 14
//27 <--> 13
//26 <--> 12
//25 <--> 11
//    .
//		.
//		.
//21 <--> 7
//
//LCD power pins: 2 and 15
//LCD ground pins: 1 and 16
//
//pot <--> LCD: 2 <--> 3
//
//pot ground pin: 3
//pot power pin: 1

////////////////////////////WIRINGS///////////////////////////////

void InitUart();
void InitADC();
unsigned int GetADCResult(char ch);

void DelayUs2x(unsigned char t){   
 while(--t);}

 void delay(unsigned int t){ //????1mS
	while(t--){ 
   DelayUs2x(245);
	 DelayUs2x(245);}}

bit LCD_Check_Busy(void) { 
	_nop_();_nop_();_nop_();_nop_();
	DataPort=0xFF; rs=0; rw=1; e=0; 
 	_nop_();_nop_();_nop_();_nop_();
 	e=1; return (bit)(DataPort & 0x80);}

void write_command (unsigned char com){
	while(LCD_Check_Busy()); rs=0; rw=0;
	_nop_();_nop_();_nop_();_nop_(); DataPort=com;
	_nop_();_nop_();_nop_();_nop_(); e=1;
	_nop_();_nop_();_nop_();_nop_(); e=0;}

void write_data (unsigned char info){
	while(LCD_Check_Busy()); rs=1; rw=0;
	_nop_();_nop_();_nop_();_nop_(); DataPort=info;
	_nop_();_nop_();_nop_();_nop_(); e=1;
	_nop_();_nop_();_nop_();_nop_(); e=0;}

void write_str(unsigned char *s) {         
	while (*s){write_data( *s); s ++;}}

void main(){	
	unsigned char steinhart_string[10];
	float steinhart;
	unsigned char cur=0;
	unsigned int xdata samples1[sample_size];
	unsigned int i;
	InitUart(); InitADC();
	write_command(0x38);
	write_command(0x08);
	write_command(0x01);
	write_command(0x06); 
	write_command(0x0c);
	write_command(0x80);
	write_str("Galvo Temp");
	write_command(0x80+0x40);
	while (1){
		samples1[cur]=GetADCResult(1); delay(10); /*printf("%u\n",samples1[cur]);*/ cur++;
		if(cur==(unsigned char)sample_size){
			steinhart=0;
			for(i=0;i<sample_size;i++){
				steinhart+=samples1[i];}
			steinhart /= sample_size;

//			printf("avg: %f\n",steinhart);

			steinhart = 1023 / steinhart;	
			steinhart = ((float)SERIESRESISTOR) / steinhart;
//			printf("resist: %f\n",steinhart);
				
			steinhart = steinhart / (float)THERMISTORNOMINAL;     // (R/Ro)
			steinhart = log(steinhart);                  // ln(R/Ro)
			steinhart /= (float)BCOEFFICIENT;                   // 1/B * ln(R/Ro)
			steinhart += 1.0 / ((float)TEMPERATURENOMINAL + 273.15); // + (1/To)
			steinhart = 1.0 / steinhart;                 // Invert
			steinhart -= 273.15;                         // convert to C

			
			write_command(0x80+0x40);		
			
			for(i=0;i<10;i++){
				steinhart_string[cur]=' ';}
				
			sprintf(steinhart_string,"%2.1fC",steinhart);
//			printf(steinhart_string);
			
			write_str(steinhart_string);
//			printf("########\n");
				
			cur=0; 
			
		}
	}
} 

unsigned int GetADCResult(char ch){
  ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
  _nop_(); _nop_(); _nop_(); _nop_(); //Must wait before inquiry
  while (!(ADC_CONTR & ADC_FLAG));    //Wait complete flag
  ADC_CONTR &= ~ADC_FLAG;             //Close ADC
  return (ADC_RES<<2)+ADC_RESL;}                    //Return ADC result
	
void InitUart(){
	SCON=0x5a; TMOD=0x20; TH1=TL1=0xFD; TR1=1;}  //8 bit data ,no parity bit  //T1 as 8-bit auto reload  //Set Uart baudrate  //T1 start running    

void InitADC(){
  P1ASF=0x9D; ADC_RES=0xBD; ADC_CONTR=ADC_POWER|ADC_SPEEDLL;} //Open 8 channels ADC function  //Clear previous result  //ADC power-on and delay           
