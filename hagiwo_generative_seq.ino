#include  <SPI.h>
#include <FastGPIO.h>
//#include <EEPROM.h>

#define SWITCH_PIN1 2
#define SWITCH_PIN2 4
#define GATE_OUT 5
#define LED_OUT 6
#define CLK_PIN 7
#define MAIN_POT_PIN A2
#define SUB_POT_PIN A4

byte stgAgate[2][16] = {
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte stgBgate[2][16] = {
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int stgAcv[2][16]  = {
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int stgBcv[2][16]  = {
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte i = 0;
byte j = 0;
byte k = 0;

byte gate_set = 1;
byte gate_count = 0;
byte old_gate_count = 0;

byte chance_set = 3;//メインツマミの読み取り値。chanceに数字を渡す
byte chance = 1;//chance_setとlengthから決定。抽選で音が変わる数
byte lottery_stepA = 2;//stgA抽選
byte lottery_stepB = 2;//stgB抽選
byte lottery_done = 2;//抽選を頭一回だけに限定する

const int LDAC = 9;                       // ラッチ動作出力ピン

int sub_knob = 0 ;//サブツマミの読み取りAD値
int old_sub_knob = 0;//SW切り替え直後、切り替え前のAD値を反映させないため、古いADを記憶
byte refrain_set = 1;//リフレインの設定回数。AD値から導出。
byte refrain_count = 0;//リフレインした回数をカウント。指定値に達すると0に戻る。
byte length_set = 4;//レングスの数値
int width_max = 1023;
int width_min = 0;

int main_knob = 0 ;//メインツマミの読み取りAD値
byte repeat_set = 0;//リピートの設定回数。AD値から導出。
byte repeat_count = 0;//リピートした回数をカウント。指定値に達すると0に戻る

byte mode_swA = 0;
byte mode_swB = 0;
byte mode_set = 0; //0=length,1=width,2=refrain
byte old_mode_set = 4;//SW切り替え直後、切り替え前のAD値を反映させないため、SW状態を記憶

void setup() {
 pinMode(LDAC, OUTPUT) ;//DAC通信用
 pinMode(SS, OUTPUT) ;//DAC通信用
 pinMode(CLK_PIN, INPUT); //gate_input
 pinMode(GATE_OUT, OUTPUT); //gate_output
 pinMode(LED_OUT, OUTPUT);//LED_output
 pinMode(SWITCH_PIN1, INPUT_PULLUP);//mode_swA
 pinMode(SWITCH_PIN2, INPUT_PULLUP);//mode_swB
 digitalWrite(LED_OUT,HIGH);
 digitalWrite(GATE_OUT,HIGH);
 delay(500);
 digitalWrite(LED_OUT,LOW);
 digitalWrite(GATE_OUT,LOW);
 delay(500);
 digitalWrite(LED_OUT,HIGH);
 digitalWrite(GATE_OUT,HIGH);
 delay(500);
 digitalWrite(LED_OUT,LOW);
 digitalWrite(GATE_OUT,LOW);
 delay(500);
 
 
 
 //
 //  length_set = EEPROM.read(0);//前回終了時のデータ読み出し
 //  refrain_set = EEPROM.read(1);//前回終了時のデータ読み出し
 //  width_max = EEPROM.read(2);//前回終了時のデータ読み出し
 //  width_min = EEPROM.read(3);//前回終了時のデータ読み出し


 SPI.begin();
 SPI.setBitOrder(MSBFIRST) ;          // ビットオーダー
 SPI.setClockDivider(SPI_CLOCK_DIV4) ;// クロック(CLK)をシステムクロックの1/4で使用(16MHz/4)
 SPI.setDataMode(SPI_MODE0) ;         // クロック極性０(LOW)　クロック位相０
 delay(50);

 for (i = 0; i < 2 ; i = i + 1) {
   for (j = 0; j < 16 ; j = j + 1) {
     stgAgate [i][j] = random(2);
     stgBgate [i][j] = random(2);
     stgAcv [i][j] = random(1024);
     stgBcv [i][j] = random(1024);
   }
 }
}


void loop() {

 //-------------外部入力状態の読み取り-----------------
 gate_set = digitalRead(CLK_PIN);//gate_input状態を読み取り
 main_knob = analogRead(MAIN_POT_PIN);
 sub_knob = analogRead(SUB_POT_PIN);
 mode_swA = digitalRead(SWITCH_PIN1);
 mode_swB = digitalRead(SWITCH_PIN2);
 old_mode_set = mode_set;

 if ( mode_swA == 1 && mode_swB == 0 ) {
   mode_set = 0;//length
 }
 else if ( mode_swA == 1 && mode_swB == 1 ) {
   mode_set = 1;//width
 }
 else if ( mode_swA == 0 && mode_swB == 1 ) {
   mode_set = 2; //refrain
 }

 if (old_mode_set != mode_set ) {
   old_sub_knob = sub_knob;
   //    EEPROM.update(0, length_set); //変更データをEEPROMに書き込み
   //    EEPROM.update(1, refrain_set); //変更データをEEPROMに書き込み
   //    EEPROM.update(2, width_max); //変更データをEEPROMに書き込み
   //    EEPROM.update(3, width_min); //変更データをEEPROMに書き込み
 }

 //-------------refrainの設定----------------------

 if ( mode_set == 2 && abs(old_sub_knob - sub_knob ) > 30) {
   old_sub_knob = 1200;//abs(old_sub_knob - sub_knob ) > 30が絶対に成立するため
   if ( sub_knob < 25 ) {
     refrain_set = 1;//refrainしない
   }
   else if ( sub_knob < 313 && sub_knob >= 26 ) {
     refrain_set = 2;//1回繰り返し
   }

   else if ( sub_knob < 624 && sub_knob >= 314 ) {
     refrain_set = 3;//2回繰り返し
   }

   else if ( sub_knob < 873 && sub_knob >= 625 ) {
     refrain_set = 4;//3回繰り返し
   }

   else if ( sub_knob >= 874 ) {
     refrain_set = 8;//7回繰り返し
   }
 }

 //----------------length設定---------------------

 if ( mode_set == 0 && abs(old_sub_knob - sub_knob ) > 30) {
   old_sub_knob = 1200;//abs(old_sub_knob - sub_knob ) > 30が絶対に成立するため
   if ( sub_knob < 25 ) {
     length_set = 4;
   }
   else if ( sub_knob < 313 && sub_knob >= 26 ) {
     length_set = 6;
   }

   else if ( sub_knob < 624 && sub_knob >= 314 ) {
     length_set = 8;
   }

   else if ( sub_knob < 873 && sub_knob >= 625 ) {
     length_set = 12;
   }

   else if ( sub_knob >= 874 ) {
     length_set = 16;
   }
 }

 //-------------widthの設定----------------------

 if ( mode_set == 1 && abs(old_sub_knob - sub_knob ) > 30) {
   old_sub_knob = 1200;//abs(old_sub_knob - sub_knob ) > 30が絶対に成立するため
   width_max = 612 + sub_knob * 4 / 10;
   width_min = 412 - sub_knob * 4 / 10;
 }


 //-------------repeatの設定----------------------
 if ( main_knob < 5 ) {
   repeat_set = 0;//repeatしない
   chance = 0;
 }
 else if ( main_knob < 111 && main_knob >= 6 ) {
   repeat_set = 0;//repeatしない
   chance = 1;
 }
 else if ( main_knob < 214 && main_knob >= 112 ) {
   repeat_set = 0;//repeatしない
   chance = 2;
 }
 else if ( main_knob < 376 && main_knob >= 215 ) {
   repeat_set = 0;//repeatしない
   if ( length_set == 4 || length_set == 6 ) {
     chance = 3;
   }
   else {
     chance = 4;
   }
 }

 else if ( main_knob < 555 && main_knob >= 377 ) {
   repeat_set = 0;//repeatしない
   chance = length_set;
 }
 else if ( main_knob < 700 && main_knob >= 556 ) {
   repeat_set = 1;//repeatしない
   if ( length_set == 4 || length_set == 6 ) {
     chance = 3;
   }
   else {
     chance = 4;
   }
 }
 else if ( main_knob < 861 && main_knob >= 701 ) {
   repeat_set = 1;//repeatしない
   chance = 2;
 }
 else if ( main_knob < 970 && main_knob >= 862 ) {
   repeat_set = 1;//repeatしない
   chance = 1;
 }
 else if ( main_knob < 1024 && main_knob >= 971 ) {
   repeat_set = 1;//repeatしない
   chance = 0;
 }
 //----------------抽選処理開始---------------------
 if (refrain_count == 0 && gate_count == 1 && lottery_done == 0 && repeat_count == 0 ) {
   lottery();//抽選処理開始
   lottery_done = 1;
 }


 //-----------------stgシーケンス出力-----------------

 switch (repeat_count) {

   case 0:
     switch (gate_set) {

       case 0:                   //gate_inputがLOWのとき
         digitalWrite(GATE_OUT, LOW); //gate_outputをLOWにする
         old_gate_count = gate_count;
         break;


       case 1:                   //gate_inputがHIGHのとき
         if (old_gate_count == gate_count) {
           gate_count ++;

           if ( gate_count > length_set ) {
             gate_count = 1;
             refrain_count ++;

             if (refrain_count >= refrain_set ) {//stgAからstgBへの橋渡し
               repeat_count++;
               refrain_count = 0;
               lottery_done = 0;
             }
           }
           WriteRegister(map(stgAcv[0][gate_count - 1], 0, 1023, width_min, width_max));
           digitalWrite(GATE_OUT, stgAgate[0][gate_count - 1]); //CV出力を出してからゲート出力
           analogWrite(6, stgAcv[0][gate_count] / 4);
           break;
         }
         break;
     }

   case 1:
     if (repeat_set == 0 ) {
       repeat_count = 0;
     }
     else {
       switch (gate_set) {

         case 0:                   //gate_inputがLOWのとき
           digitalWrite(5, LOW); //gate_outputをLOWにする
           old_gate_count = gate_count;
           break;


         case 1:                   //gate_inputがHIGHのとき
           if (old_gate_count == gate_count) {
             gate_count ++;

             if ( gate_count > length_set ) {
               gate_count = 1;
               refrain_count ++;

               if (refrain_count >= refrain_set ) {//stgBからstgAへの橋渡し
                 repeat_count++;
                 refrain_count = 0;
                 if ( repeat_count >= 2 ) {
                   repeat_count = 0;
                   lottery_done = 0;
                 }
               }
             }
             WriteRegister(map(stgBcv[0][gate_count - 1], 0, 1023, width_min, width_max));
             digitalWrite(5, stgBgate[0][gate_count - 1]); //CV出力を出してからゲート出力
             analogWrite(LED_OUT, stgBcv[0][gate_count] / 4);
             break;
           }
           break;
       }
     }

     //開発用
     //      Serial.print( sub_knob);
     //      Serial.print(",");
     //      Serial.print( length_set);
     //      Serial.println("");
 }
}

void lottery() {

 if ( chance != 0 ) {
   for (k = 0; k <= chance ; k = k + 1) {
     lottery_stepA = random(length_set);
     stgAgate[0][lottery_stepA] = 1 - stgAgate[0][lottery_stepA];
     stgAcv[0][lottery_stepA] = random(1024);

     lottery_stepB = random(length_set);
     stgBgate[0][lottery_stepB] = 1 - stgBgate[0][lottery_stepB];
     stgBcv[0][lottery_stepB] = random(1024);
   }
 }
}
void WriteRegister(int dat) {
 digitalWrite(LDAC, HIGH) ;
 digitalWrite(SS, LOW) ;
 SPI.transfer((dat >> 6) | 0x30) ;   // Highバイト(0x30=OUTA/BUFなし/1x/シャットダウンなし)
 SPI.transfer((dat << 2) & 0xff) ;        // Lowバイトの出力
 digitalWrite(SS, HIGH) ;
 digitalWrite(LDAC, LOW) ;     // ラッチ信号を出す

}
