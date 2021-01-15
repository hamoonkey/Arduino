#include <Arduino.h>
#include <Ultrasonic.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "Timer.h"

int rotaryValue = 0;
bool waitprint = false;

/*タイマー関連*/
bool watching;
Timer timer;
/*ブザー関連*/
int length = 15;         /* the number of notes */
char notes[] = "ccggaagffeeddc ";
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 350;
/* 超音波センサ関連*/
Ultrasonic Ultrasonic(2);
int startcount=0;
int stopcount=0;
int waterflag=0;  //流れていない時０、流れているとき1
/* ディスプレイ関連 */
U8G2_SSD1306_128X64_NONAME_F_SW_I2C 
  u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
#include <U8x8lib.h>
U8X8_SSD1306_128X64_ALT0_SW_I2C u8x8(SCL,SDA,/* reset=*/ U8X8_PIN_NONE);

/* あゆみさんの環境に合わせて設定するところ*/
//------------------------------------------------------------------------------------------------
/* 超音波センサを使うならこれをコメントアウト*/
bool debug = true;
/* 超音波センサを使うならこれのコメントアウトを外す*/
//bool debug = false;
/*タイマーの時間をここでセット (秒) * 1000 */
#define TIMER_MILLIS 15 * 1000
/* ブザーのピン番号　*/
#define BUZZER_PIN 6
/* LEDのピン番号　*/
#define LED_PIN 5
/*ひねるやつのピン番号*/
#define ROTARY_PIN A6
/*ボタンのピン番号*/
#define BUTTON_PIN 3
//------------------------------------------------------------------------------------------------

#define DILAY_MILLIS 10

// センサーの状態を取得
bool getStateSensor(){
  if(debug){
    rotaryValue = analogRead(ROTARY_PIN);
    if(rotaryValue > 500)
      return true;
    else
      return false;
  }else{
    UltrasoundSensor();
    if (waterflag == 1){
      return true;
    }else{
      return false;
    }
  }
}

void setup() { 
  Serial.begin(115200);
  pinMode(ROTARY_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  u8g2.begin();
  pinMode(BUTTON_PIN, INPUT);

  watching = false;

  // 時間設定
  timer.set(TIMER_MILLIS);
  timer.start();

  digitalWrite(LED_PIN, LOW);
}

int page = 0;
void loop() {
  if(watching){
    /* 見守り中 */
    digitalWrite(LED_PIN, HIGH);
    if(getStateSensor()){
      /* 手洗い中 */
      if (timer.isStop) {
        /* 一時停止なら再開 */
        timer.restart();
        Serial.println("restart");
        analogWrite(BUZZER_PIN, 0);
      }
      // タイマーの残り時間取得
      timer.getTime();
      if (timer.isOver){
        /* 時間切れ */
        Serial.println("good!!!!!!!!!!");
        // ディスプレイ表示
        displayGood();
        // 音楽開始
        ToneOutput(6);
        // むりやりフラグ変更(超音波センサ用)
        startcount=0;
        stopcount=0;
        waterflag=0;
        watching = false;
      }else {
        Serial.println(timer.times);
        displayTimer();
      }
    }else{
      /* 手を洗っていない */
      timer.getTime();
      if (timer.isOver){
        /* 手洗い完了してるなら */
        watching = false;
        timer.reset();
      }else{
        /* サボっているなら指導*/
        if (!timer.isStop){
          Serial.println("bad!!!!!!!!!!");
          analogWrite(BUZZER_PIN, 500);
          displayBad();
          timer.stop();
        }
      }
    }
  }else{
    // 待機中
    digitalWrite(LED_PIN, LOW);
    if (getStateSensor()){
      /* 手洗い開始 */
      watching = true;
      waitprint = false;
      Serial.println("start watching");
      // タイマースタート
      timer.start();
    }else{
      // waitprintのフラグでもうwaitingを出力したかを確認
      // 一度のみにしておきたいため
      if (!waitprint){
        Serial.println("waiting");
        waitprint = true;
      }
    }
  }
  delay(DILAY_MILLIS);
  
  //ディスプレイ表示
  page0();//ボタン押したら設定画面(PAGE 1)へ
  switch(page){
    case 1:
      page1();
      break;
    case 2:
      page2();
      break;
    case 3:
      page3();
      break;
    case 4:
      page4();
      break;      
  }
  
}

/*ブザー関連関数*/
void ToneOutput(int PIN){
  for(int i = 0; i < length; i++) {
        if(notes[i] == ' ') {
            delay(beats[i] * tempo);
        } else {
            PlayNote(notes[i], beats[i] * tempo,PIN);
        }
        delay(tempo / 2);    //音と音の間の時間
    }
  }
void PlayTone(int tone, int duration,int PIN) {
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(PIN, HIGH);
        delayMicroseconds(tone);
        digitalWrite(PIN, LOW);
        delayMicroseconds(tone);
    }
}
void PlayNote(char note, int duration,int PIN) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
    
    //予備int tones[] = { 131, 147, 165, 175, 196, 220, 247, 700 };
    
    for (int i = 0; i < 8; i++) {
        if (names[i] == note) {
            PlayTone(tones[i], duration,PIN);
        }
    }
}
/* 超音波センサ関連*/
void UltrasoundSensor()
{
  long RangeInCentimeters;
  RangeInCentimeters = Ultrasonic.MeasureInCentimeters();
  //Serial.println("The distance to obstacles in front is: ");
  //Serial.print(RangeInCentimeters);//0~400cm
  //Serial.println(" cm");
  
  if(RangeInCentimeters < 25 && waterflag==0){//startcountの増加   
      startcount++;
  }
  else if(RangeInCentimeters >=25 && waterflag==1){//stopcountの増加
      stopcount++;
  }
  else {//誤差を調整
      startcount=0;
      stopcount=0;
  }
  // 1.5秒なら
//  int threshold = 15000 / DILAY_MILLIS;
  int threshold = 10;
  if(startcount==15&&waterflag==0){//水がでる時
      waterflag=1; 
      startcount=0;
  }
  else if(stopcount==15&&waterflag==1){//水が止まる時
      waterflag=0;
      stopcount=0;
          }
          // return waterflag;
  //ここにタイマー機能の追加  //０で水が流れている、１で水が流れてない
}

/*ディスプレイ関連*/
void displayGood(){
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  u8g2.drawStr(0,10,"GOOD!!");    // write something to the internal memory
  u8g2.sendBuffer();                    // transfer internal memory to the display
}
void displayBad(){
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  u8g2.drawStr(0,10,"BAD!!");    // write something to the internal memory
  u8g2.sendBuffer();                    // transfer internal memory to the display
}
void displayTimer(){
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  u8g2.drawStr(0,10,timer.times);    // write something to the internal memory
  u8g2.sendBuffer(); 
}

/*設定画面操作*/
int buttonPreState = 0;
void page0(){//平常時
  Serial.println("PAGE 0");

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonPreState == 0 && buttonState == 1){//押した瞬間PAGE 1へ
    Serial.println("button");
    page = 1;
    analogWrite(BUZZER_PIN, 500);
    delay(100);
    analogWrite(BUZZER_PIN, 0);
    u8g2.clearBuffer();
  }
  buttonPreState = buttonState;
}


void page1(){//設定したい項目を選択
  Serial.println("PAGE 1");
  while(page==1){ 
    int rotaryValue = map(analogRead(ROTARY_PIN), 0,1023, 0,2);
    u8x8.clearLine(rotaryValue);
    u8x8.setCursor(0,0);
    u8x8.print("-CLOCK");
    u8x8.setCursor(0,1);
    u8x8.print("-MUSIC");
    u8x8.setCursor(0,2);//追加
    u8x8.print("-Buzzer or Light");//追加
    
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonPreState == 0 && buttonState == 1){//押した瞬間
      Serial.println("button");
      switch(rotaryValue){
        case 0://PAGE 2へ
          page = 2;
          break;
         case 1://PAGE 3へ
          page = 3;
          break;  
         case 2://PAGE 4へ追加
          page = 4;
          break;      
      }
      analogWrite(BUZZER_PIN, 500);
      delay(100);
      analogWrite(BUZZER_PIN, 0);
      u8x8.clearDisplay();
    }
    buttonPreState = buttonState;
  }
}

int clockHour = 7;
int clockMinute = 38;
void page2(){//現在時刻の設定
  Serial.println("PAGE 2");
  int HorM = 0;
  while(page==2){
    if(HorM == 0){
      u8x8.clearLine(0);
      clockHour = map(analogRead(ROTARY_PIN), 0,1023, 0,23);
      u8x8.setCursor(0,0);
      u8x8.print(clockHour);
      u8x8.print(":");
      u8x8.print(clockMinute);
  
      int buttonState = digitalRead(BUTTON_PIN);
      if (buttonPreState == 0 && buttonState == 1){
        Serial.println("button");
        HorM++;
        analogWrite(BUZZER_PIN, 500);
        delay(100);
        analogWrite(BUZZER_PIN, 0);
      }
      buttonPreState = buttonState;
    }else{
      u8x8.clearLine(0);
      clockMinute = map(analogRead(ROTARY_PIN), 0,1023, 0,59);
      u8x8.setCursor(0,0);
      u8x8.print(clockHour);
      u8x8.print(":");
      u8x8.print(clockMinute);
  
      int buttonState = digitalRead(BUTTON_PIN);
      if (buttonPreState == 0 && buttonState == 1){
        Serial.println("button");
        page = 0;
        HorM--;
        analogWrite(BUZZER_PIN, 500);
        delay(100);
        analogWrite(BUZZER_PIN, 0);
        u8x8.clearDisplay();
      }
      buttonPreState = buttonState;
    }
  }
}


void page3(){//音楽の設定
  Serial.println("PAGE 3");
  while(page==3){
    int rotaryValue = map(analogRead(ROTARY_PIN), 0,1023, 0,2);
    u8x8.clearLine(rotaryValue);
    u8x8.setCursor(0,0);
    u8x8.print("-Twincle Star");
    u8x8.setCursor(0,1);
    u8x8.print("-Paprika");
    u8x8.setCursor(0,2);
    u8x8.print("-sample2");
    
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonPreState == 0 && buttonState == 1){//押した瞬間
      Serial.println("button");
      page = 0;
      analogWrite(BUZZER_PIN, 500);
      delay(100);
      analogWrite(BUZZER_PIN, 0);
      u8x8.clearDisplay();
    }
    buttonPreState = buttonState;
  }
}

void page4(){//LEDかブザーの選択
  Serial.println("PAGE 4");
  while(page==4){
    int rotaryValue = map(analogRead(ROTARY_PIN), 0,1023, 0,1);
    u8x8.clearLine(rotaryValue);
    if(rotaryValue==1){
        digitalWrite(LED_PIN,HIGH);
        analogWrite(BUZZER_PIN, 0);
     }else if(rotaryValue==0){
        digitalWrite(LED_PIN,LOW);
        analogWrite(BUZZER_PIN, 500);//音楽鳴らす
     }
    u8x8.setCursor(0,0);
    u8x8.print("-Buzzer");
    u8x8.setCursor(0,1);
    u8x8.print("-Light");
  
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonPreState == 0 && buttonState == 1){//押した瞬間
      Serial.println("button");
      page = 0;
      digitalWrite(LED_PIN,LOW);
      analogWrite(BUZZER_PIN, 0);
      analogWrite(BUZZER_PIN, 500);
      delay(100);
      analogWrite(BUZZER_PIN, 0);
      u8x8.clearDisplay();
    }
    buttonPreState = buttonState;
  }
}
