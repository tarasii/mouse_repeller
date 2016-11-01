#ifndef TIMER1_H
#define TIMER1_H


//#define max_divider 321
//#define min_divider 159

#include "Arduino.h"

class Timer1 {
  public:
    Timer1();
    Timer1(int mind, int maxd);
    void Init(int divider);
    void SetDivider(int &divider);
    int GetDivider();
    int GetPrescaller();    
    float GetFrequncy();
  private:
    int current_divider;
    byte current_prescaller;   
    int max_divider = 1024;
    int min_divider = 0;
  };

#endif