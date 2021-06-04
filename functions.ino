int pins_digit[] = {2, 3, 4, 5};
int pins_segment[] = { 6, 7, 8, 9, 10, 11, 12};
int digits[][7] = { 
  { 0,0,0,0,0,0,1 }, // 0
  { 1,0,0,1,1,1,1}, // 1
  { 0,0,1,0,0,1,0}, // 2
  { 0,0,0,0,1,1,0}, // 3
  { 1,0,0,1,1,0,0}, // 4
  { 0,1,0,0,1,0,0}, // 5
  { 0,1,0,0,0,0,0}, // 6
  { 0,0,0,1,1,1,1}, // 7
  { 0,0,0,0,0,0,0}, // 8
  { 0,0,0,1,1,0,0}, // 9
  { 1,1,1,1,1,1,1}, // OFF
};

void showDigit(int pos, int num){
  //자릿수(pos)와 표현할 숫자(num)를 넣으면 7segment에 표현한다
  for(int i=0;i<7;i++)
    digitalWrite(pins_segment[i], digits[num][i]);
  
  for(int i=0;i<4;i++){
    if(pos == i)
      digitalWrite(pins_digit[i], HIGH);
    else
      digitalWrite(pins_digit[i], LOW);
  }
}//pos번 째 자리 세그먼트에 num 숫자 display

void displayDigit(int a, int b, int c, int d){
  showDigit(0, a);
  delay(5);
  showDigit(1, b);
  delay(5);
  showDigit(2, c);
  delay(5);
  showDigit(3, d);
  delay(5);
}//세그먼트에 a,b,c,d 숫자 동시에 display

void displayClock(int h, int m){
  int h0= h/10;
  int h1= h%10;
  int m0= m/10;
  int m1= m%10;
  displayDigit(h0, h1, m0, m1);
}//segment 앞 2개에 h를, 뒤 2개에 m을 표현한다_시계모드

void displayClockBlink(int h, int m, int pos){
  int value[] = {h/10, h%10, m/10, m%10};
  for(int i=0;i<4;i++){
    if((i == pos) && ((second() % 2) == 0)){ //해당 위치, 짝수 초(1초마다 깜빡이게 하기 위함)
      showDigit(i,10); //해당 위치의 세그먼트를 off
      delay(5);
    } else {
      showDigit(i,value[i]);
      delay(5);
    }
  }
}//segment 앞2개에 h를, 뒤2개에 m을 표현하고 pos자리를 1초마다 깜빡이게 한다_시간설정모드

void displayTimer(int t){
    
  int a = (t/60)/10;
  int b = (t/60)%10;
  int c = (t%60)/10;
  int d = (t%60)%10;
  
  displayDigit(a,b,c,d);
}//초를 분,초로 표현한다_타이머모드