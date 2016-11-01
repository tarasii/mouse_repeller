#include "myTimer3.h"

Timer3::Timer3(){	
  //SetDivider(divider); 
  current_prescaller = _BV(CS32) | _BV(CS30);
}

Timer3::Timer3(int mind, int maxd){  
  current_prescaller = _BV(CS10);
  max_divider = maxd;
  min_divider = mind;
}

void Timer3::Init(int divider){
  TCCR3A = _BV(COM3A0);
  TCCR3B = current_prescaller | _BV(WGM32);	
  SetDivider(divider);
}

void Timer3::SetDivider(int &divider){
	if (divider == current_divider) return;
  if (divider > max_divider) divider = min_divider;
  if (divider < min_divider) divider = max_divider; 

	current_divider = divider;
  OCR3A = divider; //(159-50КГц 321-25КГц)
}

float Timer3::GetFrequncy(){
  int prescaller = GetPrescaller();
  if (prescaller == 0) return 0;
  return 16000.0/(2.0*prescaller*(current_divider+1));
}

int Timer3::GetDivider(){
	return current_divider;
}

int Timer3::GetPrescaller(){
  if (current_prescaller == 0) return 0;
  if (current_prescaller == 1) return 1;
  if (current_prescaller == 2) return 8;
  if (current_prescaller == 3) return 64;
  if (current_prescaller == 4) return 256;
  if (current_prescaller == 5) return 1024;}