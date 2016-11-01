
#include <PCD8544.h>
#include <MenuPCD8544.h>

#include <myTimer1.h>
#include <myTimer3.h>

#include <DS1307.h>

#define BUTTON1 52
#define BUTTON2 40
#define BUTTON3 48
#define BUTTON4 46
#define BUTTON5 44
#define BUTTON6 42

#define PCD8544_SCLK 22
#define PCD8544_DIN 24
#define PCD8544_DC 26
#define PCD8544_CS 28
#define PCD8544_RST 30
#define PCD8544_LIGHT 32

PCD8544 display = PCD8544(PCD8544_SCLK, PCD8544_DIN, PCD8544_DC, PCD8544_RST, PCD8544_CS, PCD8544_LIGHT);

int cur_cc, cur_cnt, tmp_cnt;
int mode, tmp_mode;
float frq;
long previousMillis = 0;
long previousMillisSec = 0;
int maim_int = 1000;

Timer1 T1 = Timer1(157, 319);
Timer3 T3;

DS1307 rtc(SDA, SCL);

#define  NUMBER_OF_BUTTONS 6
byte buttons[NUMBER_OF_BUTTONS] = {BUTTON1, BUTTON2, BUTTON3, BUTTON4, BUTTON5, BUTTON6};
int lasttime;
int time_pressed[NUMBER_OF_BUTTONS];
byte cur_btn, tmp_btn=0;

#define BTN_NO 0
#define BTN_MENU 4
#define BTN_DOWN 2
#define BTN_UP 1
#define BTN_1 32
#define BTN_2 16
#define BTN_3 8

bool update_scr = false;
bool update_cnt = false;
bool blink_cnt = false;

#define frq_len 50
int frq_arr[] = {
  319, 312, //25 
  306, 300,
  295, 290,
  284, 280,
  274, 270,
  265, 261, //30
  257, 253,
  249, 245,
  241, 238,
  234, 231,
  227, 224, //35
  221, 218,
  215, 212,
  210, 207,
  204, 202,
  199, 197, //40
  194, 192,
  189, 187,
  185, 183,
  181, 179,
  177, 175, //45
  173, 171,
  169, 167,
  166, 164,
  162, 161,
  159, 157 };

int cur_cc2 = 0, cur_cnt2 = 0;
#define frq_len2 6
int frq_arr2[] = {
  //780, //10 
  //520, 
  //389, //20
  311, 
  259, //30
  222, 
  194, //40
  172, 
  155};
  
void show_info();
void taskMenu();
byte CheckButtons();

void mRandom();
void mSweepIn();
void mSweepOut();
void mContinuous();
void mManual();

void mDate();
void mTime();
void mSN();
void mAlert();
void mActivation();

void mOn();
void mOff();
void mDelay();

void mAntiDog();
void mAntiDogTime();

void mBack();
void mExit();



#define NUM_ITEMS_SUBMENU_SELECT 6
const char menuSelectItem1[] PROGMEM = "RANDOM";
const char menuSelectItem2[] PROGMEM = "SWEEP IN";
const char menuSelectItem3[] PROGMEM = "SWEEP OUT";
const char menuSelectItem4[] PROGMEM = "CONTINUOUS";
const char menuSelectItem5[] PROGMEM = "MANUAL";
const char menuSelectItem6[] PROGMEM = "BACK";
const menuItem_t menuSelectItems[NUM_ITEMS_SUBMENU_SELECT] PROGMEM =
{
    { menuSelectItem1, &mRandom     },
    { menuSelectItem2, &mSweepIn    },
    { menuSelectItem3, &mSweepOut   },
    { menuSelectItem4, &mContinuous },
    { menuSelectItem5, &mManual     },
    { menuSelectItem6, &mBack       }   };
const menu_t submenuSelect PROGMEM = { NUM_ITEMS_SUBMENU_SELECT, menuSelectItems };


#define NUM_ITEMS_SUBMENU_SCHEDULE 4
const char menuScheduleItem1[] PROGMEM = "ON";
const char menuScheduleItem2[] PROGMEM = "OFF";
const char menuScheduleItem3[] PROGMEM = "DELAY";
const char menuScheduleItem4[] PROGMEM = "BACK";
const menuItem_t menuScheduleItems[NUM_ITEMS_SUBMENU_SCHEDULE] PROGMEM =
{
    { menuScheduleItem1, &mOn     },
    { menuScheduleItem2, &mOff    },
    { menuScheduleItem3, &mDelay  },
    { menuScheduleItem4, &mBack   }   };
const menu_t submenuSchedule PROGMEM = { NUM_ITEMS_SUBMENU_SCHEDULE, menuScheduleItems };


#define NUM_ITEMS_SUBMENU_SETTINGS 6
const char menuSettingsItem1[] PROGMEM = "DATE";
const char menuSettingsItem2[] PROGMEM = "TIME";
const char menuSettingsItem3[] PROGMEM = "ALERT";
const char menuSettingsItem4[] PROGMEM = "S/N";
const char menuSettingsItem5[] PROGMEM = "ACTIVANION";
const char menuSettingsItem6[] PROGMEM = "BACK";
const menuItem_t menuSettingsItems[NUM_ITEMS_SUBMENU_SETTINGS] PROGMEM =
{
    { menuSettingsItem1, &mDate        },
    { menuSettingsItem2, &mTime        },
    { menuSettingsItem3, &mSN          },
    { menuSettingsItem4, &mAlert       },
    { menuSettingsItem5, &mActivation  },
    { menuSettingsItem6, &mBack        }   };
const menu_t submenuSettings PROGMEM = { NUM_ITEMS_SUBMENU_SETTINGS, menuSettingsItems };


#define NUM_ITEMS_SUBMENU_ANTIDOG 3
const char menuAntiDogItem1[] PROGMEM = "FREQUENCY";
const char menuAntiDogItem2[] PROGMEM = "DELAY";
const char menuAntiDogItem3[] PROGMEM = "BACK";
const menuItem_t menuAntiDogItems[NUM_ITEMS_SUBMENU_ANTIDOG] PROGMEM =
{
    { menuAntiDogItem1, &mAntiDog     },
    { menuAntiDogItem2, &mAntiDogTime },
    { menuAntiDogItem3, &mBack        }   };
const menu_t submenuAntiDog PROGMEM = { NUM_ITEMS_SUBMENU_ANTIDOG, menuAntiDogItems };


#define NUM_MENU_ITEMS 5
const char menuMainItem1[] PROGMEM = ":SELECT";
const char menuMainItem2[] PROGMEM = ":SCHEDULE";
const char menuMainItem3[] PROGMEM = ":SETTINGS";
const char menuMainItem4[] PROGMEM = ":ANTI DOG";
const char menuMainItem5[] PROGMEM = "EXIT";
const menuItem_t menuMainItems[NUM_MENU_ITEMS] PROGMEM =
{
    { menuMainItem1, (menuFunc_t) &submenuSelect    },
    { menuMainItem2, (menuFunc_t) &submenuSchedule  },
    { menuMainItem3, (menuFunc_t) &submenuSettings  },
    { menuMainItem4, (menuFunc_t) &submenuAntiDog   },
    { menuMainItem5, &mExit     },
};
const menu_t menuRoot PROGMEM = { NUM_MENU_ITEMS, menuMainItems };

MenuPCD8544 menuMain(&menuRoot);




void setup() {
  byte i;
  
  // put your setup code here, to run once:
  display.begin();              // Инициализация дисплея
  display.setContrast(60);      // Устанавливаем контраст
  display.clear();       // Очищаем дисплей
  //display.display();

  cur_cc = 159;
  pinMode(11, OUTPUT);
  T1.Init(cur_cc);

  tone(10, 20);

  pinMode(5, OUTPUT);
  T3.Init(800);

  // button init
  for (i=0; i < NUMBER_OF_BUTTONS; i++)
  {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  } 
  
  Serial.begin(9600);
  Serial.println("Start");         

  analogReference(INTERNAL2V56);
   
  display.print("hello");

  delay(500);

  rtc.begin();
  rtc.halt(false);

  //rtc.setTime(14,17,0);
  //rtc.setDate(9,10,2016);
  
  update_scr = true;

  mode = 4;
}

void loop() {
  menuFunc_t doit = NULL;
  char keyCode = 0;

  cur_btn = CheckButtons();
  if (cur_btn != tmp_btn){
    if (cur_btn != 0) {
      update_scr = true;
      if (mode==5) update_cnt = true;
    }  
    tmp_btn = cur_btn; 
  }

  if (mode==6 || mode==7) maim_int = 500;
  else maim_int = 1000;
  
  if (millis() - previousMillisSec > maim_int){
    previousMillisSec = millis(); 
    update_scr = true;
    if (mode>0 && mode<5) update_cnt = true;
  }

  if (update_cnt){
    update_cnt = false;
    if (mode==1) {
      cur_cnt = random(frq_len-1);
    } else  if (mode==2) {
      cur_cnt++;
    } else if (mode==3) {
      cur_cnt--;
    } else if (mode==4) {
      if (cur_cnt < (frq_len / 2 ))
        cur_cnt = (frq_len-1) - (cur_cnt + 1);
      else if (cur_cnt > (frq_len / 2 ))
        cur_cnt = (frq_len-1) - (cur_cnt);
      else
        cur_cnt = 0;
    
    } else if (mode==5) {
      
      if (cur_btn & BTN_UP){
        cur_cnt++;
      }
      if (cur_btn & BTN_DOWN){
        cur_cnt--;
      }
    }
  }

  if (update_scr){
    update_scr = false;

    if (mode > 0 && mode < 6)
      show_info();
    else if (mode == 6)
      set_date();
    else if (mode == 7)
      set_time();
    else {
      taskMenu();
    }
  }
}

void taskMenu()
{
  menuFunc_t doit = NULL;
  char keyCode = 0;

  if (cur_btn & BTN_UP) keyCode = 'U';
  if (cur_btn & BTN_DOWN) keyCode = 'D';
  if (cur_btn & BTN_MENU) keyCode = 'S';

  if (keyCode){
    int8_t selected = menuMain.navigate(keyCode, &doit);
    if (selected){
      menuMain.begin(&display); 
      if (doit != NULL){
        (*doit)();
      }                 
    }
  }
  keyCode = 0;
}

void mExit(){
  mode = tmp_mode;
  display.clear();
  update_scr = true;
}

void mRandom(){ 
  mode = 1; 
  display.clear();
}

void mSweepIn(){ 
  mode = 2; 
  display.clear();
}

void mSweepOut(){ 
  mode = 3;
  display.clear();
}

void mContinuous(){ 
  mode = 4;
  display.clear();
}

void mManual(){ 
  mode = 5;
  display.clear();
}
void mBack(){}

void mOn(){}
void mOff(){}
void mDelay(){}

void mDate(){
  mode = 6;
  display.clear();
}
void mTime(){
  mode = 7;
  display.clear();
}
void mSN(){}
void mAlert(){}
void mActivation(){}
void mAntiDog(){}
void mAntiDogTime(){}

byte CheckButtons(){
  byte  i, curr, res = 0;
  int curtime;

  //curtime = millis();
  //if (lasttime + 10 < curtime) lasttime = curtime;
  //else return tmp_btn; 
  
  for (i=0; i < NUMBER_OF_BUTTONS; i++)
  {
    curr = digitalRead(buttons[i]);
    
    if (curr == LOW) time_pressed[i]++; 
    else time_pressed[i] = 0;

    if (time_pressed[i] > 80) res += (1 << i);
    //if (time_pressed[i] > 100) time_pressed[i] = 0;
  }   
  
  return res; 
}


void show_info(){
  if (mode != 0){
    if (cur_cnt < 0) cur_cnt = frq_len-1;
    if (cur_cnt >= frq_len) cur_cnt = 0;

    cur_cc2 = frq_arr2[cur_cnt2 / 10];
    cur_cnt2 ++;
    if (cur_cnt2 / 10 >= frq_len2) cur_cnt2 = 0;
    
    cur_cc = frq_arr[tmp_cnt];
    T1.SetDivider(cur_cc);
    frq = T1.GetFrequncy();
    display.setCursor(0, 0);
    display.print(frq);
    display.print(" KHz ");

    T3.SetDivider(cur_cc2);
    frq = T3.GetFrequncy();
    display.setCursor(0, 1);
    display.print(frq * 1000);
    display.print(" Hz ");

    display.setCursor(0, 2);
    display.print(rtc.getDateStr()); 
    display.setCursor(0, 3);
    display.print(rtc.getTimeStr());

    if (cur_cnt != tmp_cnt){
      tmp_cnt = cur_cnt;
      frq = T1.GetFrequncy();
      //Serial.println(cur_cc);     
      Serial.println(frq);
    }          

    if (cur_btn & 4) {
      menuMain.begin(&display);
      tmp_mode = mode;        
      mode = 0;
      cur_btn = 0;
    }
  }
}

void set_date(){
  if (mode != 6) return;

  if (blink_cnt)
    display.clear();
  else {  
    display.setCursor(0, 0);
    display.print(rtc.getDateStr());
  } 
  blink_cnt = !blink_cnt;
  
  if (cur_btn & 4) {
    menuMain.begin(&display);
    tmp_mode = mode;        
    mode = 0;
    cur_btn = 0;
  }
}

void set_time(){
  if (mode != 7) return;
  
  if (blink_cnt)
    display.clear();
  else {  
    display.setCursor(0, 0);
    display.print(rtc.getTimeStr());
  } 
  blink_cnt = !blink_cnt;

  if (cur_btn & 4) {
    menuMain.begin(&display);
    tmp_mode = mode;        
    mode = 0;
    cur_btn = 0;
  }
}
  

