/* 
 * 注意: コードを実行する前に、debugModeを確認してください。
 * 
 * - パルス
 *   水道メータから、1Lあたりに送られる
 *   
 * - パルスの間隔
 *   パルスの間隔はpulseIntervalをmsで指定します。
 *   この間隔より短い場合はノイズとして、処理は何も行いません。
 *   コードの※1を参照
 * 
 * - パルスのカウント
 *   パルスを割り込みでカウントします。
 *   コードの※2を参照
 *   
 * - パルスの割り込み  
 *   attachInterruptで割り込みの処理を指定指定しています。
 *   attachInterruptについて、詳しい説明は下記を参照。
 *   http://www.lapis-semi.com/lazurite-jp/contents/reference/attachInterrupt.html
 *   コードの※3を参照
 *   
 * - sleepの間隔  
 *   カウンタの値を外部に送信してから、次に送信するまでの時間（sleepInterval）をmsで指定します。
 *   1hは3600000msなので、1時間間隔を開ける場合は3600000を指定
 *   コードの※4を参照
 *   
 * - カウンター値の送信
 *   throwData()関数の中に、カウンター値の送信処理を記述してください。
 *   コードの※5を参照
 *   
 * - カウンター値送信中のパルスについて
 *   カウンター値は、送信された直後にリセットされます。
 *   そのため、送信が始まってから終わるまでに送られたパルスは、次のカウントに含まれます。
 *   
 *   ---------------------
 *     pulse
 *     pulse
 *     【1度目の送信開始】
 *     pulse <- 送信が開始した後のパルスは、2度目の送信に持ち越し
 *     【1度目の送信終了】
 *     pulse
 *     pulse
 *     pulse
 *     【2度目の送信開始】
 *     【2度目の送信終了】
 *   ---------------------
 *   上記の場合、1度目の送信でカウンタ値は2
 *   2度目の送信でカウンタ値は4となります。
 *   
 * - sleepの処理
 *   arduinoではsleep関数がないので、delayを使用しています。
 *   Lazuriteのsleepを使う場合は、delayをsleepに書き換えてください。
 *   コードの※6を参照
 *   
 * - 出力データのカウント値の桁数について  
 *   出力時に、00001のように0で桁数を揃えたい場合、
 *   throwData()関数内の処理を変更してください。
 *   
 *   現状は下記のようになっている
 *   char strCount[5];
 *   sprintf(strCount, "%05d", pulseCount); 
 *   
 *   4桁にしたい場合は、下記のように変更
 *   char strCount[4];
 *   sprintf(strCount, "%04d", pulseCount); 
 *  
 */

// デバッグモード

boolean debugMode = false;  

// パルスのカウンタ
// sleepTimeの間隔でカウンタが0にリセットされる

int pulseCount = 0;              

// 2ピンから入力

int inputPin   = 2;

// 13ピンに出力

int outputPin  = 13;

// 5ピンから出力　Wi-SUNモジュールをsleepから復帰させるのに、リセットする

int restPin  = 5;

// ※4: sleepの間隔(ms)
// 1hは3600000msなので、1時間間隔を開ける場合は3600000を指定

int sleepInterval = 10000;

// パルスの長さ（ms）

int pulseInterval = 1000;

// 最新の割り込みからの経過時間を保存するための変数

unsigned long previousPulseTime = 0;

// afterInterrupt()の関数が生成される前にsetup()内で使用しているため、
// 事前に関数の宣言をしておく必要がある。

void afterInterrupt();

void setup() {
  
  if (debugMode) {    

    // デバッグ用で、シリアルモニタにprintlnする場合

    Serial.begin(9600); 
    
  } else {
    Serial.begin(115200); 

    //水道BOX毎にIDが異なる。10台分設定
    
    Serial.println("SKSREG S1 12345678abcdef08");
    delay(100);

    //Ch33(922.5MHz)を選択　//Chは全ての台数を同じにする
    
    Serial.println("SKSREG S2 21");
    delay(100);
    
    //PAN ID 0x8888を選択//PANは全ての台数を同じにする
    
    Serial.println("SKSREG S3 8888");
    delay(100);
  }

  // ピンモードの指定
  
  pinMode(inputPin,INPUT);
  pinMode(outputPin,OUTPUT);
  pinMode(restPin, OUTPUT);

  // ※3: 割り込み時の処理を指定。
  // 第一引数を"0"と指定することで、ピン2を外部割り込みとして使用
  // 第二引数で割り込みが発生したときにcallする関数を指定
  // 第三引数で割り込みを発生させるためのモードを指定します。
  //   指定可能なモードは次の通りです。
  //   LOW: ローレベルのときに割り込みが発生します。
  //   RISING:  信号の立ち上がりエッジで割り込みが発生します。
  //   FALLING: 信号の立下りエッジで割込みが発生します。
  //   CHANGE: 信号が変化したときに割り込みが発生します。
  
  attachInterrupt(0, afterInterrupt, RISING);
}

void loop() {
  // ※6: ここではdelayを使っているが、Lazuriteではsleepにする

  delay(sleepInterval);
  
  // sleepから抜けた時の処理

  afterAwake();
}

void afterInterrupt() {    
    
  if ((millis() - previousPulseTime) < pulseInterval) {   

    // ※1: 一定時間内に割り込みが入った場合、その割り込みは無視される
    // ここに一定時間内に割り込みが入った場合の処理
    // デバッグ用に、一定時間内の割り込み時に文字列を出力    

    Serial.println("LESS THAN 1000ms!!!");
    
  } else {
    previousPulseTime = millis();    

    // ※2: カウンタをインクリメント

    countUp();

    // ここにパルス毎の処理
    // デバッグ用にカウントを表示

    // Lazurite非互換
    
    Serial.println("count is ... " + pulseCount);
    
    // Serial.println(pulseCount);
  }
}

void countUp() {
  pulseCount++;
}

void afterAwake() {
  // カウンタの値を送信する

  throwData();

  // カウンタをリセット

  resetCount();
}

void resetCount() {
  pulseCount = 0;
}

void throwData() {
  // ※5: 本来であれば、ここで外部に結果を送信するが
  // ここではデバック用にシリアルモニタに出力させるようになっている。
  
  // To store 4 digits number string
  // ID2電子水道メータアダプターVer0.1.txtのサンプルのように下記の宣言をcountUp()の外で行うと、
  // カウンタをインクリメントしても出力がインクリメントされない。
  // そのためcountUp()内で下記を宣言すること。

  char strCount[5];

  // 出力結果の数値に、0のパディングを入れたいのでsprintfで結果を整形

  // ※ Lazurite非互換
  // sprintf(strCount, "%05d", pulseCount); 
  
  //モジュールリセット開始

  digitalWrite(restPin, HIGH);
  delay(1000);
  digitalWrite(restPin, LOW);
  delay(1000);
  digitalWrite(restPin, HIGH);
  delay(1000);

  //モジュールリセット終了

  // サンプル出力1
  // 文字列の連結は "+" を使うが、数値を扱う場合Stringクラスにしないといけないので下記のような書き方になる。
  
  // Serial.println("COUNT IS " + String(strCount));

  // サンプル出力2
  // SKSENDTOを使ったサンプルの出力

  // ※ Lazurite非互換
  // Serial.println("SKSENDTO 1 FE80:0000:0000:0000:1034:5678:ABCD:EF01 0E1A 0 0005 " + String(strCount));  
  
  Serial.print("SKSENDTO 1 FE80:0000:0000:0000:1034:5678:ABCD:EF01 0E1A 0 0005 ");
  Serial.println(strCount);
    
  //カウント値を送信後sleepする
  
  Serial.println("SKDSLEEP");  
  delay(1000);

}

