#ifndef TIMER3_H
#define TIMER3_H


#include "Arduino.h"


class Timer3 {
  public:
    Timer3();
    Timer3(int mind, int maxd);
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