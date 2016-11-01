#include "myTimer1.h"

Timer1::Timer1(){	
  //SetDivider(divider); 
  current_prescaller = _BV(CS10);
}

Timer1::Timer1(int mind, int maxd){  
  current_prescaller = _BV(CS10);
  max_divider = maxd;
  min_divider = mind;
}

void Timer1::Init(int divider){
  TCCR1A = _BV(COM1A0);
  TCCR1B = current_prescaller | _BV(WGM12);	
  SetDivider(divider);
}

void Timer1::SetDivider(int &divider){
	if (divider == current_divider) return;
  if (divider > max_divider) divider = min_divider;
  if (divider < min_divider) divider = max_divider; 

	current_divider = divider;
  OCR1A = divider; //(159-50КГц 321-25КГц)
}

float Timer1::GetFrequncy(){
  return 16000.0/(2.0*GetPrescaller()*(current_divider+1));
}

int Timer1::GetDivider(){
	return current_divider;
}

int Timer1::GetPrescaller(){
  if (current_prescaller == 0) return 0;
  if (current_prescaller == 1) return 1;
  if (current_prescaller == 2) return 8;
  if (current_prescaller == 3) return 64;
  if (current_prescaller == 4) return 256;
  if (current_prescaller == 5) return 1024;}
