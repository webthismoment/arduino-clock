#include <Time.h>
#include <TimeLib.h> //https://github.com/PaulStoffregen/Time
#include "pitches.h"

// A0,A1,A2,A3,A4,A5 == 14,15,16,17,18,19
#define MODE_CLOCK 0
#define MODE_TIMER 1
#define MODE_MODIFY 2
#define pin_led 19
#define pin_buzzer 13
#define pin_sw_mode 14
#define pin_sw1 15
#define pin_sw2 16
#define pin_sw_cursor 17

int prev_sw_state[] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,1, 1,1,1}; //TODO: hashmap으로 변경(14~17만 사용)
int sw_state[] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,1, 1,1,1};
int timer = 0;
int timer_min = 0;
int timer_sec = 0;
int MODE = 0; //0:시계모드, 1:타이모드, 2:시간설정모드
int cur = 0;  //시간설정모드_커서
int curs = 0; //타이머모드_커서
time_t timer_end_time = 0;
boolean is_paused = false;  //타이머 일시정지
boolean is_timer_decreasing = false;  //타이머가 진행중인가?

int noteDuration = 4; // 4분음표로 통일
int noteLength = 250/noteDuration;
int pauseBetweenNotes = noteLength * 1.30;

//function
void showDigit(int, int); //원하는 위치의 세그먼트에 원하는 숫자 display
void displayDigit(int, int, int, int); //세그먼트에 네 숫자 동시에 display
void displayClock(int, int);//시계(앞 두 자리: 시, 뒷 두 자리: 분) display_시계모드
void displayClockBlink(int, int, int);//바꾸고자 하는 자리(시/분) 깜빡이게 하기_시간설정모드
void displayTimer(int);//타이머 display_타이머모드
void timer_off();//타이머 리셋

//전역함수//
boolean isSwitchUp(int i){
  //루프 끝날 때 저장한 스위치 prev 상태와 루프 시작할 때 저장한 스위치 상태를 비교
  if(prev_sw_state[i] == LOW && sw_state[i] == HIGH)
    return true;
  else
    return false;
}

boolean isSwitchDown(int i){
  //루프 끝날 때 저장한 스위치 prev 상태와 루프 시작할 때 저장한 스위치 상태를 비교
  if (prev_sw_state[i] == HIGH && sw_state[i] == LOW)
    return true;
  else
    return false;
}

void timer_off(){
  is_timer_decreasing = false;
  timer_end_time = 0;
  timer = 0;
  timer_min = 0;
  timer_sec = 0;
  digitalWrite(pin_led, LOW);
  noTone(pin_buzzer);
}//타이머 리셋

void setup() {
  //arduino Setup
  Serial.begin(9600);
  for(int i=2;i<=13;i++)
    pinMode(i, OUTPUT); //segment, buzzer_output 설정
  pinMode(14, INPUT_PULLUP);  //MODE 변경(시계, 타이머, 시간설정)
  pinMode(15, INPUT_PULLUP);  //타이머 시작, 알람 울리면 종료
  pinMode(16, INPUT_PULLUP);  //타이머 값 증가, 타이머 일시정지/재개
  pinMode(17, INPUT_PULLUP);  //커서 이동
  pinMode(pin_led, OUTPUT); // LED 
  
  //Program setup
  setTime(14,53,20,7,6,2021);  //NOTE: 테스트 전에 현재 시각으로 설정
  //setTime(hr,min,sec,day,month,yr);
}

void loop() {
  for(int i=14;i<=17;i++)
    sw_state[i] = digitalRead(i);
  
  if(isSwitchDown(pin_sw_mode)){
    MODE = (MODE + 1) % 3;  
  }//스위치0 누르면 모드 변환(시계모드, 타이머모드, 시간설정모드)
  
  switch(MODE){
    case MODE_CLOCK: //시계모드
      displayClock(hour(), minute()); //시, 분 display
      break;

    case MODE_TIMER: //타이머모드
      if(is_timer_decreasing)
        displayTimer(timer); //타이머 display
      else
        displayClockBlink(timer_min, timer_sec, curs);

      if(isSwitchDown(pin_sw_cursor))
      {
        curs = (curs + 1) % 4;
      }
      
      if(isSwitchDown(pin_sw1)){ 
        if(is_timer_decreasing == false && timer > 0 ){
          timer_end_time = now() + timer;//timer_end_time에 현재 시간값(now)+타이머 설정한 값 쓰기 // now(): eturns the current time as seconds since Jan 1 1970
          is_timer_decreasing = true;
          is_paused = false;
        }//타이머 시작 전, 타이머 값 증가시킨 경우(타이머 설정 중)면 타이머 시작(이 때 timer_end_time > 0)
        else{
          timer_off();
        }//타이머 시작 후면 타이머 리셋
      }//스위치1 눌렀을 때
      
      if(isSwitchDown(pin_sw2)){
        if (is_timer_decreasing){
          is_paused = !is_paused;
        } //타이머 시작 후면 일시정지/재개 기능
        else
        {
          if(curs == 0)
            timer_min += 10;
          else if(curs == 1)
            timer_min += 1;
          else if(curs == 2)
            timer_sec += 10;
          else if(curs == 3)
            timer_sec += 1;

          if(timer_min > 99)
            timer_min = 0;
          if(timer_sec >= 60)
            timer_sec = 0;
          timer = int(timer_min)*60 + int(timer_sec);
          // Serial.println(timer);
        }
      }//스위치2 눌렀을 때
      break;

    case MODE_MODIFY:   // 시간설정모드
      int hour_ = hour(); //0~23
      int minute_ = minute(); //0~59
      displayClockBlink(hour_, minute_, cur+1); //설정된 커서 위치의 세그먼트 깜빡임

      if (isSwitchDown(pin_sw_cursor)) {
        cur = (cur + 1) % 3;
      }//스위치3 누르면 커서 위치 바뀜
      
      if (isSwitchDown(pin_sw2)) {
        if(cur == 0) //커서 위치가 1이면_2번 세그먼트
          hour_ += 1; 
        else if(cur == 1) //커서 위치가 2이면_3번 세그먼트
          minute_ += 10;  
        else if(cur == 2)//커서 위치가 3이면_4번 세그먼트
          minute_ += 1;

        if(minute_ >= 60)
          hour_ -= 1;
        setTime(hour_, minute_, second(), day(), month(), year()); //스위치2 누를 때마다 시간 설정 업데이트
      }//스위치2 눌렀을 때
      
      break;
  }//모드 변환 끝
  
  //모드 구속없이 알람 진행
  if(is_timer_decreasing) //타이머 시작 후
  {
    if (is_paused) //타이머 일시정지하면
      timer_end_time = now() + timer;//
    else //타이머 시작 후(일시정지 없을 때) 또는 일시정지 후 재개 시
      timer = timer_end_time - now();

    if(timer <= 0)//타이머 종료하면
    {
      digitalWrite(pin_led,HIGH);//LED on
      // 멜로디 재생
      for (int thisNote = 0; thisNote < sizeof(melody)/sizeof(int); thisNote++) 
      {
        tone(pin_buzzer, melody[thisNote],noteLength);
        delay(pauseBetweenNotes); //BUZZER on
        if(digitalRead(pin_sw1) == LOW)
        {
          timer_off();
          break;
        } //스위치1 눌렀을 때 타이머 리셋_알람 종료
      }
    } //알람 끝
  }
  
  // if(timer > 0) {
  //   digitalWrite(pin_led, LOW);
  //   noTone(pin_buzzer);
  // } //알람 울리지 않아야 할 경우에는 LED, BUZZER off

  //switch 상태저장
  for(int i=14;i<=17;i++){
    prev_sw_state[i] = sw_state[i];
  }
}//loop 끝
