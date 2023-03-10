#define VER "Program version 20230101"
//**************************************************************************************************
/*
  電力モニター Watt_Monitor
  ■主要部品
  ・ATmega1284P-PU 3.3V 12MHz水晶で動作
  ・DS1302(動作電圧：2.0V～5.5V)
  ・Aitendo 3.2inch LCD(M032C1289TP)(動作電圧：3.3V)(320 * 240)
  (16ビット⇨8ビットへの変更方法)(Aitendoから引用)
  デフォルトは16ビットです。R4を（半田こて）取り外して→R5に取り付けて8ビットモードになります。8ビットの接続：DB0〜DB7
  ・秋月 高精度電流センサー [SR-3702-150N/14Z]
  　適用電流：～80A AC
  　変流比:3000対1(測定電流30Aに対して、0.01A(負荷抵抗100Ωなら、0.01A*100Ω=1Vを出力))
  ・秋月 ＳＤカードスロットＤＩＰ化モジュール(Cixi Borui Electronic Technology製) -> 秋月　AE-SD

  ■機能
  ・電力モニターの電源電圧
  ・センサー毎の電流値
  ・センサー毎の30分間の積算電力量
  ・センサー毎の30分間の最大電力
  ・前回リセット時からの各センサー合計の時間帯別積算電力量
  ・各センサー合計の瞬間電力
  等を計測(1秒毎)して表示(2秒毎)
  ・各種設定
  ・SDカードに30分毎にデータを記録
  ・SDカード内のファイル一覧を表示・ファイル削除
  FlashAir使用時は、
  ・パソコン・スマホからWiFi越しにSDカードに記録されているデータを参照・コピー
  ・1日1回インターネットから標準時刻を取得し時計を修正

  ■IDE (1.8.5)(1.8.13)(1.8.19)
  　・スケッチと同じフォルダに入れておくファイル
  　　FlashAir関係　iSdio.cpp, iSdio.h
  　　SDカード関係 　Sd2Card.h, Sd2CardExt.cpp,Sd2CardExt.h, Sd2PinMap.h,SdInfo.h
  　　フォント関係　　GroteskBold32x64.c,SevenSeg_XXXL_Num.c, SixteenSegment32x48.c　
  　・ツール　　ボード：OSWIN ATMega1284P 12MHz using Optiboot (Bobuino pinlayout)
  　・スケッチ　書込装置を使って書き込む

  ■電圧・電流・電力の計算方法
  　なんでも作っちゃう、かも。
  　Arduino電力計 - 回路図とスケッチ
  　http://arms22.blog91.fc2.com/blog-entry-420.html

  ■変更履歴
  測定単位毎に表示するように変更
  printStringで始まる関数の引数の順序を変更
  setup()関数 電源を入れた日時をSDに記録
  printBarGraph()関数 blinkRevLed() をやめて、消費電力のバーグラフをLCDに表示
  FlashAirのiSDIOコマンドを利用し日本標準時を取得
  loop()関数 SCREEN OFF中にもresetIntegratedValues()が実行されるように修正
  PIN_REV_LED 変数削除
  PIN_LCD_RSET の値を14から31に修正
  ユニバーサル基板化に備えて各ピン配置を変更
  ユニバーサル基板化に合わせて各ピン配置を変更
  setup()関数 DS1302の関数が漏れていたため追加
  loop()関数 setTimeToJst()の実行時刻をを毎日2:15に変更
  printWattHourTimeperiod()関数 Total kWh が99999を超えたら00000に戻るように変更
  積算電力量関係の数値の表示桁数を変更
  previous で始まる文字列変数のサイズをconst int 変数で指定
  printBlackout()関数 停電時、電圧表示を"BLACKOUT"にして2秒毎にビープ音を鳴らすよう変更
  停電・復電時にSDカードにイベント情報を書き込むよう変更
  電源ON、リセット、時刻同期時にもSDカードにイベント情報を書き込むよう変更
  setTimeToJst()関数 syncTimeWithJST()に名前変更
  時刻同期が失敗した場合にもSDカードにイベント情報を書き込むよう変更
  冗長な部分を関数化(samplingVA(), changeValue())して整理
  printJST()関数 文字色を時刻同期結果に応じて変わるよう変更
  iSDIO_sendHttpRequest()関数 レスポンス待ち秒数を7秒から10秒に変更
  関数の記述位置を種類別に整理
  Arduino IDE Ver.1.8.9でコンパイルエラー発生 Ver.1.8.5まで戻す
  2020/6/14時点で iSDIO_waitHttpResponse() がエラーを返すようになったため
   時刻取得先のNTPサーバを変更　ntp-a1.nict.go.jp -> ntp-b1.nict.go.jp
  syncTimeWithJST()関数 B系のNTPサーバから時刻取得できない場合、A系で再実行するように変更
  config()関数 UP押下時の動作がDOWNだったのを修正
  beep()関数 ビープ音を鳴らすか鳴らさないかの設定を追加
  EEPROM関係変数宣言部 EEPROMの読み出し部分のアドレス間違いを修正
  adjustDateTime()関数 WiFiが切れた時の再接続に使用するために、SYNCの動作にFlashAirの初期化を追加
  saveEventToSdCard()関数 引数にFマクロを使用
  adjustDateTime()関数 SYNC動作内のFlashAirの電源OFF時間を1->5秒に変更
  resetIntegratedValues()関数 Since 月/日 時:分:秒を年/月/日 時:分に変更
  turnOnScreen() turnOffScreen() printMainWindow()関数バックライト消灯前に画面を黒一色にするよう変更
  ↓loop()内のsyncTimeWithJST()が失敗する現象の原因?
  「http/httpsを利用した時刻配信」の時限停止のお知らせ
   https://jjy.nict.go.jp/schedule.html
  ・8月の停止予定
   8月 3日(月)～ 7日(金) 11：00～14：00　3時間停止
   8月10日(月)～14日(金) 11：00～14：00　3時間停止
   8月17日(月)～21日(金) 10：00～15：00　5時間停止
   8月22日(土)、23日(日) 11：00～14：00　3時間停止
   8月24日(月)～30日(日) 10：00～15：00　5時間停止
  ・9月の停止予定　(2020/08/28)
   8月31日(月)～9月13日(日) 00：10～03：00　約3時間停止 <---原因?
   9月14日(月)～9月27日(日) 21：00～23：50　約3時間停止
   9月28日(月)～9月30日(水) 03：00～06：00　3時間停止
  暫定的に、loop()内のsyncTimeWithJST()の実行時刻を00:05に変更
  loop()関数 タイマーで消灯した状態で停電した場合に画面が表示されてすぐ消灯するのを修正
  loop() printBlackout()関数 上記消灯中に復電した場合に、画面が表示されてすぐ消灯するのを修正
  printBlackout()関数 停電検出機能をcalcWattHour()に移動
  calcWattHour()関数 電源電圧低下検出機能を追加
  printSyncTimeInfo()関数 printJST()の名前を変更
  printSyncTimeInfo()関数 時刻取得の成功・失敗に関わらず、最後の取得実行日時を表示
  printSyncTimeInfo()関数 nextSequenceIdを16進数4桁で表示
  requestFlashAirReset()関数 fNeedsResetFlashAirをtrueにセット
  (fNeedsResetFlashAir: 最初の時刻取得の際にFlashAirをリセット操作するフラグ)
  printSyncTimeInfo()関数 fNeedsResetFlashAirがtrueの場合、nextSequenceIdの後ろに'R'を表示させる
  calcWattHour()関数 停電・電圧低下から復帰した際に、requestFlashAirReset()を実行
  resetFlashAir()関数 FlashAirのリセット操作を一つの関数にまとめる
  syncTimeWithJST()関数 fNeedsResetFlashAirがtrueの場合、resetFlashAir()を実行
  syncTimeWithJST()関数 A系での再実行を廃止
  printClock()関数他 時計表示モードを追加
  時計表示中に時刻取得し時刻表示に戻った際に一部の文字が表示されない現象を解消
  時計表示中に停電中のまま3分経過すると、時計表示に戻り[BLACKOUT]の赤文字が残るのを解消
  画面モードにSCREEN_CLOCKを追加
  自動で時刻合わせする時刻を設定できるようにする
  起動時に時刻合わせが失敗しても、自動的に時刻合わせする
  自動的に時刻合わせするかしないかを設定できるようにする
  getDayOfWeek()関数　年月日から曜日を計算する関数を追加
  iSDIO_readHttpResponse()関数　時刻取得先を NICT から WorldTiemAPI に変更
  iSDIO_readHttpResponse()関数からサーバー依存の部分を取り、返されるデータをそのまま返すように変更
  syncTimeWithJST()関数をサーバー別に関数を分けて、syncTimeWith()関数から呼び出し
  サーバー別関数に、サーバーから返されたデータから時刻情報を抽出する処理を追加
  adjustIntlClock()関数 時刻取得からRTCへの時刻設定までの遅延を補正する関数を追加
  iSDIO_status()関数 DHCPサーバーからIPアドレスが割り当てされなかった場合にエラー判定するように修正
  syncTimeWithWorldTimeApi()関数 時刻取得に失敗した時に時刻合わせをしないようにするエラー判定を修正
  syncTimeWithNict()関数 時刻取得に失敗した時に時刻合わせをしないようにするエラー判定を修正
  iSDIO_readHttpResponse()関数 HTTPレスポンスコードが200以外はfalseを返すように修正
  calcWatt()関数 200V回路のセンサーの測定値を正しく計算していなかったのを修正
  printEx()関数　制御文字は表示しないようにする
  saveDataToSdCard()関数 各センサー毎の使用量の合計もSDカードに記録する
  calcWatt()関数 200V回路のセンサーは20Wではなく40W以下を測定誤差として切り捨てる
  saveEventToSdCard()関数 時刻同期成功時のサーバー名が記録されなかったのを記録されるよう修正
  config()関数 "Auto sync hour"を"Auto sync Hour"に訂正
  getCardStatus()関数　FlashAirの通信機能の初期化に失敗しても続行するように修正
  saveDataToSdCard()関数 積算値(wattHourTimeperiod)もSDカードに記録する
  saveDataToSdCard()関数 SDカードに保存失敗した場合に次の保存までデータを積算するよう修正
  reset()関数 30分毎の使用量をSDカードに記録するよう修正
  saveDataToSdCard()関数 保存する積算値(wattHourTimeperiod)の端数を四捨五入
  CTの測定値を補正する計算式が間違っていたため修正
  センサー毎の処理をfor文にまとめて処理
  各変数の宣言・初期化を見直し
  printBarGraph()関数 紫色表示を15kW以上から13kW以上に変更
  printBarGraph()関数 値が16kW以上の場合に15kWの位置が紫色から赤色になるよう変更
  printBarGraph()関数 バー1目盛の表示倍率(100%->1kW)を設定できるよう変更
  printBarGraph()関数 表示倍率ではなくフルスケールのkWを設定できるよう変更
  printMainWindow()関数 ボタン TIME ADJUST の表記を TIME に変更
  printMainWindow()関数 INTEGRATED の表記を INTEGATION VALUES に変更
  printElapsedDays()関数 積算日数の表示位置を変更
  printButton()関数 表示するテキストの位置の計算式を変更
  printButton()関数 メニューボタンの座標、ボタン内テキストを変更
  printClock()関数 SDカードの書込エラー発生時、時計表示の背景色が赤色になる不具合を修正
  syncTimeWithWorldTimeApi()関数 時刻取得に失敗した時にエラーコードが2回表示されるのを修正
  printMainWindow()関数 INTEGRATION VALUES の表記を INTEGRATED VALUES に変更
  printSdStatus()関数 エラー時の表示色を変更
  表示色を変更
  　リアルタイム:白、30分毎の積算値:青、30分毎のピーク値:ピンク、リセットしてからの積算値:黄、その他:緑
  adjustDateTime()関数 UP押下時の動作がDOWNだったのを修正
  いくつかの変数の名前変更
  syncTimeWith()関数 syncTime()に名前変更
  reset()関数 30分毎の使用量をSDカードに記録する処理をresetIntegratedValues()関数に移動
  saveEventToSdCard()関数 引数を追加
  loop()関数 resetIntegratedValues()関数をsaveDataToSdCard()関数より先に実行するように修正
  loop()関数 saveDataToSdCard()関数の再入防止方法を修正
  iSDIO_readHttpResponse()関数 200 OKの場合はイベント記録なし、それ以外はイベント記録するよう修正
  config()関数 FlashAirのIPアドレスを表示
  いくつかの変数の名前変更
  printBarGraph()関数 フルスケールを超えたらkW表示を赤字にする
  printBarGraph()関数 フルスケールを超えたら全てのバーを赤色にするよう変更
  setup()関数からloop()関数の間に追加した関数の記述位置を種類別に整理
  nextSequenceIdを16進数で表示する部分の書式指定を修正　誤%X→正%lX
  時刻同期結果のイベント記録にnextSequenceIdの情報を追加
  時刻同期失敗の時にもイベント記録にサーバー名の情報を追加
  指定したサーバーを引数にしてsyncTime()関数を呼び出し時刻同期を実行するように修正
  タイマー割り込み(Timer1)を使用
  いくつかの変数の名前変更
  2022/4/10 タイマー割り込みルーチンの中に、サンプリングと電力量計算の処理を入れる
  2022/4/12 タイマー割り込みの影響かDS1302から取得する時刻が不安定なためシステム時刻を使用するよう修正
  2022/4/12 システム時刻の遅れ誤差が大きいので定期的にRTCに合わせるよう修正
  2022/4/13 システム時刻をRTCに合わせる間隔を短く(1分に1回(30秒の時)に)修正
  2022/4/13 メインに使用する時計を外付と内蔵時計機能のどちらにするか選択できるよう修正
  2022/4/14 SDカード内のファイル一覧表示にてファイルサイズの桁数を最大7->8桁に、空き容量を最大9->10桁に修正
  2022/4/14 DS1302にアクセスする際は一時的に割り込みを禁止する　-> DS1302から取得する時刻が安定
  2022/4/15 printFileList()関数 32GBのSDカードの空き容量が正しく表示されないのを修正
  2022/4/16 printBlackout()関数 beep音が濁るためTimerFreeToneライブラリを使用(それでも割り込みの瞬間に音が途切れる)
  2022/4/17 timer1()関数 電力量が積算されないのを修正
  2022/4/22 resetFlashAir()関数 "FlashAir Reset"を"FlashAir reset"に修正
  2022/4/24 syncTimeOnceADay()関数 停電中は時刻合わせをしないよう修正
  2022/5/11 incrElapsedDays()関数の位置をsaveWattHourTimeperiod()の前に移動
  2022/5/11 setTime()関数 内蔵時計機能使用時にsetTime()関数を誤って再帰的に呼び出していたため修正
  2022/5/11 setTime()他RTC関係の関数の誤りを修正
  2022/7/10 resetIntegratedValues()関数 watt_sum = 0.0 を追加
  2022/7/11 いくつかの変数、定数の名前変更
  2022/7/16 DEFAULT_CT1_PERCENTAGE,DEFAULT_CT2_PERCENTAGE 134 -> 140
  2022/8/21 resetIntegratedValues()関数 saveIntegratedValuesToSdCardに名前変更
  2022/8/21 累積値のリセットをresetIntegratedValues()関数にまとめる
  2022/8/28 saveIntegratedValuesToSdCard()関数 計測を一時停止してリセットした累積値をすぐにSDカードに記録する
  2022/8/28 saveWattHourTimeperiod()とincrElapsedDays()の実行順を逆にする
  2022/8/28 beep()関数 サウンドOFFの時にもdelayを実行する
  2022/9/14 saveWattHourTimeperiod()関数 自動リセット日の00:00には記録しないように修正
  2022/9/15 saveWattHourTimeperiod()関数 自動リセット日の00:00に記録する処理をここに移動
  2022/10/11 saveWattHourTimeperiod()関数 currTimeのコロン漏れを修正
  2022/12/2 getButtonNum()関数 BTN_PRESSEDを追加してボタン以外のタッチでも消灯タイマーをリセットする
  2022/12/13 resetSystem()関数 システムリセット(ソフトリセット)の機能を追加
  2022/12/13 systemReset()関数 システムリセットの画面を追加
  2022/12/16 systemReset()関数 表示内容を変更
  2022/12/17 config()関数 "Clock"と"Sync JST"を変更
  2023/1/1 getCardStatus()関数 表示内容を追加
*/
// **************************************************************************************************
// ATmega1284P-PU　ピン配置(Bobuino pin layout)
// 　https://maniacbug.wordpress.com/2011/11/27/arduino-on-atmega1284p-4/
//   maniacbug-mighty-1284p-68ed99c.zip
// 　(C:\Program Files\Arduino\hardware\arduino\avr\variants\bobuino\pins_arduino.h)
const int ON = 1;
const int OFF = 0;
const int PROVIDER_WORLD_TIME_API = 0;
const int PROVIDER_NICT = 1;
// 汎用の文字列格納用バッファのサイズ
const int BUFFER_LEN_FULL = 41; // 最大で画面一杯　320 ÷ 8(SmallFontの文字幅) + '\0' = 41
const int BUFFER_LEN_HALF = 21; // 最大で画面半分　160 ÷ 8(SmallFontの文字幅) + '\0' = 21
// SDカードに保存するイベント情報格納用バッファのサイズ
const int BUFFER_LEN_EVENT = 100;

// 環境に合わせて修正が必要な設定値
// 商用電源周波数(Hz)
const unsigned long POWER_FREQ = 50L;
// calcWatt()関数に使用する定数(実測に基づく係数)
const float VT_CONVERSION_RATIO = 71.29; // VTの出力電圧を実際の電圧に換算する比率(%)
const float CT1_LOAD_RESISTANCE = 99.8;  // CT(センサーNo.1)の負荷抵抗(Ω)
const float CT2_LOAD_RESISTANCE = 100.2; // CT(センサーNo.2)の負荷抵抗(Ω)
const float CT3_LOAD_RESISTANCE = 100.3; // CT(センサーNo.3)の負荷抵抗(Ω)

// 各パラメーターの初期値
const int DEFAULT_PROVIDER = PROVIDER_WORLD_TIME_API;
// EEPROMに保存する初期値
// 1.EEPROMから読み出した値が有効な範囲にない場合に使用する
// 2.CONFIGのLOAD DEFAULTを実行した場合に使用する
// 自動消灯設定時間(分) 0～99
// 最後の操作から一定時間が経過したら表示を消す 0にすると消灯しない
const byte DEFAULT_SCREEN_OFF_TIME = 3;
// 毎月自動で積算値をリセットする日付(日) 0～28
// 0にすると自動リセットしない
const byte DEFAULT_AUTO_RESET_DATE = 11;
// 測定対象の電圧(V) 0 or 100 or 200
// 0にすると測定しない
const byte DEFAULT_CT1_VOLTAGE = 100;
const byte DEFAULT_CT2_VOLTAGE = 100;
const byte DEFAULT_CT3_VOLTAGE = 200;
// 測定値の誤差修正率(%) 1～255
const byte DEFAULT_VT_PERCENTAGE = 89;
const byte DEFAULT_CT1_PERCENTAGE = 140;
const byte DEFAULT_CT2_PERCENTAGE = 140;
const byte DEFAULT_CT3_PERCENTAGE = 176;
// 音を鳴らす ON or 鳴らさない OFF
const byte DEFAULT_SOUND = ON;
// 時計を表示 ON or 表示しない OFF
const byte DEFAULT_CLOCK = OFF;
#define FLASH_AIR // FlashAir使用の場合に定義
// 自動で時刻合わせするかどうか
const byte DEFAULT_AUTO_TIME_SYNC = ON;
// 自動で時刻合わせする時刻
const byte DEFAULT_TIME_SYNC_HOUR = 21;
const byte DEFAULT_TIME_SYNC_MIN = 5;
// バーグラフのフルスケール(kW)
const byte DEFAULT_BAR_GRAPH_FULLSCALE = 15;

// EEPROM関係
#include <EEPROM.h>
byte eepromScreenOffTime;
byte eepromAutoResetDate;
byte eepromCtVoltage[4];
byte eepromVtPercentage, eepromCtPercentage[4];
byte eepromSound, eepromClock;
byte eepromAutoTimeSync;
byte eepromTimeSyncHour, eepromTimeSyncMin;
byte eepromBarGraphFullscale;
const int EEPROM_SCREEN_OFF_TIME = 0;
const int EEPROM_AUTO_RESET_DATE = 1;
const int EEPROM_VT_PERCENTAGE   = 2;
const int EEPROM_CT1_VOLTAGE     = 3;
const int EEPROM_CT1_PERCENTAGE  = 4;
const int EEPROM_CT2_VOLTAGE     = 5;
const int EEPROM_CT2_PERCENTAGE  = 6;
const int EEPROM_CT3_VOLTAGE     = 7;
const int EEPROM_CT3_PERCENTAGE  = 8;
const int EEPROM_SOUND           = 9;
const int EEPROM_CLOCK           = 10;
const int EEPROM_AUTO_SYNC_TIME  = 11;
const int EEPROM_TIME_SYNC_HOUR  = 12;
const int EEPROM_TIME_SYNC_MIN   = 13;
const int EEPROM_BAR_GRAPH_FULLSCALE = 14;
const byte EEPROM_MODE_MAX = EEPROM_BAR_GRAPH_FULLSCALE;

// 時刻関係
#include <TimeLib.h>
char currTime[6]; //"hh:mm" + '\0'
char prevTime[6]; //"hh:mm" + '\0'
const int TIME_YEAR    = 0;
const int TIME_MONTH   = 1;
const int TIME_DAY     = 2;
const int TIME_DAYNAME = 3;
const int TIME_HOUR    = 4;
const int TIME_MINUTE  = 5;
const int TIME_SECOND  = 6;

// DS1302関係
// https://github.com/msparks/arduino-ds1302/
// arduino-ds1302-master.zip
#include <DS1302.h>
namespace {
const int PIN_DS1302_CE = 29;   // DS1302 Pin 5(CE)   -> Arduino Pin pin 29
const int PIN_DS1302_IO = 28;   // DS1302 Pin 6(I/O)  -> Arduino Pin pin 28
const int PIN_DS1302_SCLK = 27; // DS1302 Pin 7(SCLK) -> Arduino Pin pin 27
DS1302 ds1302(PIN_DS1302_CE, PIN_DS1302_IO, PIN_DS1302_SCLK);
}
const int CLOCK_DS1302   = 0; // DS1302をメインの時計として使用
const int CLOCK_INTERNAL = 1; // 内蔵時計機能をメインの時計として使用
int clockMode = CLOCK_DS1302;

// タイマー関係
// https://github.com/thomasfredericks/Metro-Arduino-Wiring
// Metro-Arduino-Wiring-master.zip
#include <Metro.h>
Metro scrOffTimerInterval = Metro(60000); // タイマー用変数増加間隔(ms)
Metro updateScreenInterval = Metro(2000); // 画面表示更新間隔(ms)
int scrOffTimer1; // 画面自動消灯タイマー用
int scrOffTimer2; // 画面自動消灯タイマー用(停電時)
const int TIMER_RESET = -2; //scrOffTimer2の状態を表す
const int TIMER_STOP  = -1; //scrOffTimer2の状態を表す

// タイマー割り込み関係
// https://github.com/PaulStoffregen/TimerOne
#include <TimerOne.h>
// mighty-1284p-master\variants\bobuino\pins_arduino.h
//  TIMER1A D8  (PD5)pin.19 -> LCD CS
//  TIMER1B D30 (PD4)pin.18 -> LCD RESET
volatile int timer1_state;     //状態制御用変数
const int STATE_STOP      = 0; //計測停止
const int STATE_SENSOR1   = 1; //電流センサー No.1のサンプリング
const int STATE_SENSOR2   = 2; //電流センサー No.2のサンプリング
const int STATE_SENSOR3   = 3; //電流センサー No.3のサンプリング
const int STATE_CALCULATE = 4; //電力の計算

// LCD(M032C1289TP)関係
// http://www.rinkydinkelectronics.com/library.php?id=51
// UTFT.zip (UTFT_VERSION  282)(UTFT_VERSION  283)
// http://www.rinkydinkelectronics.com/library.php?id=92
// URTouch.zip
// 空きメモリーを増やすために、memorysaver.h を修正
// C:\Users\user\Documents\Arduino\libraries\UTFT\memorysaver.h 修正
// //#define DISABLE_SSD1289 以外の行をアンコメント(//を削除して有効化)
#include <memorysaver.h>
#include <UTFT.h>
#include <URTouch.h>
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
// http://www.rinkydinkelectronics.com/r_fonts.php
//extern uint8_t SixteenSegment32x48[];
extern uint8_t GroteskBold32x64[];
extern uint8_t SevenSeg_XXXL_Num[];
/*  ピン配置
  LCD        Aruduino         LCD          Aruduino
  1 GND   -> GND              21 DB0     -> 14 (D0)
  2 VCC   -> VCC              22 DB1     -> 15 (D1)
  3 nc                        23 DB2     -> 16 (D2)
  4 RS    -> 21 (D31)         24 DB3     -> 17 (D3)
  5 WR    -> 20 (D9)          25 DB4     ->  1 (D4)
  6 RD    -> VCC              26 DB5     ->  2 (D5)
  7 DB8                       27 DB6     ->  3 (D6)
  8 DB9                       28 DB7     ->  4 (D7)
  9 DB10                      29 D_CLK   -> 26 (D26)
  10 DB11                     30 D_CS    -> 25 (D25)
  11 DB12                     31 D_DIN   -> 24 (D24)
  12 DB13                     32 D_BUSY
  13 DB14                     33 D_OUT   -> 23 (D23)
  14 DB15                     34 D_Penirq-> 22 (D22)
  15 CS    -> 19 (D8)         35 SD_OUT
  16 nc                       36 SD_SC
  17 RESET -> 18 (D30)        37 SD_DIN
  18 nc                       38 SD_CS
  19 LED-A -> 10k-2SC1815...  39 nc
  20 nc                       40 nc
*/
const int PIN_LCD_LED = 20; //バックライトの電源制御用
const int PIN_LCD_RS = 31;
const int PIN_LCD_WR = 9;
const int PIN_LCD_CS = 8;
const int PIN_LCD_RSET = 30;
UTFT myGLCD(SSD1289_8, PIN_LCD_RS, PIN_LCD_WR, PIN_LCD_CS, PIN_LCD_RSET);
const int PIN_TP_CLK = 26;
const int PIN_TP_CS = 25;
const int PIN_TP_DIN = 24;
const int PIN_TP_OUT = 23;
const int PIN_TP_PENIRQ = 22;
URTouch  myTouch(PIN_TP_CLK, PIN_TP_CS, PIN_TP_DIN, PIN_TP_OUT, PIN_TP_PENIRQ);
// メインウィンドウの状態
int currScreenMode;
int curX, curY;
const int SCREEN_SETUP  = 0; // 初期化処理中モード
const int SCREEN_ON     = 1; // 画面表示状態/再表示指示(printMainWindow(SCREEN_ON)を実行することでバックライトのLEDをオンにして再表示する)
const int SCREEN_OFF    = 2; // 画面非表示状態/非表示指示(printMainWindow(SCREEN_OFF)を実行することでバックライトのLEDをオフにして非表示にする)
const int SCREEN_REDRAW = 3; // 画面再描画指示モード
const int SCREEN_CLOCK  = 4; // 時計表示モード

// printStringExEx()関係
const long VGA_NO_CHANGE = VGA_TRANSPARENT;
const int FONT_NO_CHANGE             = 0;
const int FONT_SMALL_FONT            = 1;
const int FONT_BIG_FONT              = 2;
const int FONT_SEVEN_SEG_NUM_FONT    = 3;
const int FONT_SIXTEEN_SEGMENT_32X48 = 4;
const int FONT_GROTESK_BOLD_32X64    = 5;
const int FONT_SEVEN_SEG_XXXL_NUM    = 6;

// ボタン・行の状態(選択/非選択)
const int STS_NORMAL  = 0; // 非選択
const int STS_INVERSE = 1; // 選択
// ボタンID
const int BTN_NOT_PRESSED = 0;
const int BTN_RESET       = 1;
const int BTN_ADJUST_TIME = 2;
const int BTN_CONFIG      = 3;
const int BTN_SDCARD      = 4;
const int BTN_SCREEN_OFF  = 5;
const int BTN_SCREEN_ON   = 6;
const int BTN_PRESSED     = 9;
void printStringExEx(char* s1, char* s2, unsigned int digit, int x, int y, int font, unsigned long c = VGA_NO_CHANGE);
void printBarGraphFrame(int screenMode = SCREEN_ON);
void printBarGraph(int screenMode = SCREEN_ON);
void printSyncTimeInfo(int screenMode = SCREEN_ON);
void printElapsedDays(int screenMode = SCREEN_ON);
void printWattHourTimeperiod(int screenMode = SCREEN_ON);
void printClock(int screenMode = SCREEN_CLOCK);
void clearScreen(int ms = 1000); // ms: 画面クリアまでの待ち時間(ミリ秒)

// 電力量関係
// 端子定義
const int PIN_AREF    = 0; // 基準の電圧(電源電圧の中点=1.65V)
const int PIN_VOLTAGE = 1; // 電圧計測用
const int PIN_SENSOR1 = 2; // 電流センサー No.1
const int PIN_SENSOR2 = 3; // 電流センサー No.2
const int PIN_SENSOR3 = 4; // 電流センサー No.3
const int PIN_SENSOR[4] = {0, PIN_SENSOR1, PIN_SENSOR2, PIN_SENSOR3};
// マイコンの電源電圧(V)
const float VCC = 3.3;
// 実効電圧、実効電流、有効電力
const unsigned int NUMBER_OF_SAMPLES = 25; // １サイクルあたりのサンプル数
unsigned long SAMPLING_PERIOD = 1000000L / (POWER_FREQ * (unsigned long)NUMBER_OF_SAMPLES); // サンプリング間隔(マイクロ秒)
int VASamples[NUMBER_OF_SAMPLES * 4];// サンプリング用バッファ
const float KVT = VT_CONVERSION_RATIO; // VTの出力電圧から実際の電圧に換算する比率
// CTの出力電圧から実際の電流に換算する比率
// CT負荷抵抗 * 係数 / 巻き数
const float KCT1 = CT1_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT2 = CT2_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT3 = CT3_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT[4] = {0, KCT1, KCT2, KCT3};
volatile float vrms[4];
volatile float irms[4];
volatile float irms_max[4];
volatile float watt[4];
volatile float watt_max[4];
int prevLevel; // バーグラフの前回指示値を格納
volatile int watt_samples;
volatile float watt_hour, watt_hour1, watt_hour2, watt_hour3;
volatile float watt_sum, watt_sum1, watt_sum2, watt_sum3;
volatile float wattHourTimeperiod1, wattHourTimeperiod2, wattHourTimeperiod3, wattHourTimeperiod4;
volatile float wattHourTimeperiod;
// 積算値リセット関係
char resetTime[BUFFER_LEN_HALF];
int elapsedDays; //リセット日からの経過日数+1
boolean needsSaveIntegratedValues = false; // trueの時、積算値をリセットする
// 必要な文字のみ表示更新するための変数
const int WATTHOUR_LEN = 6;
char prevWattHour1[WATTHOUR_LEN + 1];
char prevWattHour2[WATTHOUR_LEN + 1];
char prevWattHour3[WATTHOUR_LEN + 1];
char prevWattHour[WATTHOUR_LEN + 1];
const int WATTHOURTIMEPERIOD_LEN = 5;
char prevWattHourTimeperiod[WATTHOURTIMEPERIOD_LEN + 1];
const int WATTHOURTIMEPERIOD1_LEN = 11;
char prevWattHourTimeperiod1[WATTHOURTIMEPERIOD1_LEN + 1];
char prevWattHourTimeperiod2[WATTHOURTIMEPERIOD1_LEN + 1];
char prevWattHourTimeperiod3[WATTHOURTIMEPERIOD1_LEN + 1];
char prevWattHourTimeperiod4[WATTHOURTIMEPERIOD1_LEN + 1];
const int ELAPSEDDAYS_LEN = 9;
char prevElapsedDays[ELAPSEDDAYS_LEN + 1];
int prevDate = -1;
int prevSec = -1;
char prevWattHourPerDay[4];
char prevDateTime[BUFFER_LEN_HALF];
char prevClockHour[3];
char prevClockMin[3];
char prevClockDate[11];
volatile boolean hasBlackout = false;
volatile boolean hasBrownout = false;
const char BLACKOUT[] = "BLACKOUT";

// SDカード関連
#include <SPI.h>
#include <SD.h>
// SDカード総容量取得のための関数 SD.card.cardSize() を実行するために
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.hを修正
// class SDClass の変数宣言のうち
// ・Sd2Card card;
// ・SdVolume volume;
// ・SdFile root;
// をprivate:からpublic:に移す
//
// SDカード抜き差しによるSD.open()失敗の場合の対策
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.cpp修正
// SDClass::begin()のreturn行の前に次の行を追加
// if (root.isOpen()) root.close();
// SD card attached to SPI bus as follows:
//  MOSI - D11 //SDカードモジュール CMD -> Arduino Pin 6
//  MISO - D12 //SDカードモジュール D0  -> Arduino Pin 7
//  CLK  - D13 //SDカードモジュール CLK -> Arduino Pin 8
//  CS   - D10 //SDカードモジュール D3  -> Arduino Pin 5
const int PIN_SDCARD_CS  = 10;
const int PIN_SS         = 10; // 変更不可
const int PIN_SDCARD_VDD = A5; // 電源制御
const int MAX_DATASTRING_LEN = 501; // SDカードへ１回に書き込むデータの最大サイズ(500文字 + '\0'(1文字) = 501)
const int MAX_FILE_NAME_LEN = 13;  // ファイル名 8+3形式(12桁)+'\0'
const int MAX_FILE_SIZE_LEN = 11;  // long型変数の最大桁数(10桁)+'\0'
// (SDカードの状態)
int sdSts;
// SD_STS_MSGを増やす場合はsdStsMsg変数の初期化も修正する
const int SD_CARD_PRESENT     = 0; // SDカードが挿入されている
const int SD_SUCCESS          = 0; const char SD_STS_MSG0[] = "Succeeded       ";
const int SD_CARD_NOT_PRESENT = 1; const char SD_STS_MSG1[] = "Card not present"; // SDカードが挿入されていない
const int SD_OPEN_ERROR       = 2; const char SD_STS_MSG2[] = "Card open error "; // ファイルのオープン失敗
const int SD_WRITING          = 3; const char SD_STS_MSG3[] = "Writing to card "; // ファイル書き込み中
const int SD_NO_FILES         = 4; const char SD_STS_MSG4[] = "File not found  "; // ファイルが見つからない
const int SD_WRITE_ERROR      = 5; const char SD_STS_MSG5[] = "Card write error"; // ファイルの書き込み失敗
const int SD_STS_MSG_BLANK    = 6; const char SD_STS_MSG6[] = "                "; // メッセージ消去用
const char *sdStsMsg[7] = {
  SD_STS_MSG0,
  SD_STS_MSG1,
  SD_STS_MSG2,
  SD_STS_MSG3,
  SD_STS_MSG4,
  SD_STS_MSG5,
  SD_STS_MSG6
};

// (ファイル書き込み関係)
const int SAVING_INTERVAL = 30; // データの記録間隔（分）
char sdLastUpdatedTime[BUFFER_LEN_HALF];
// ファイル一覧作成のための変数
struct FILELIST {
  int num; // 通し番号(1～)
  char attribute; // ファイル属性(' ':ルートファイル、'+':ルートファイル以外)
  char name[MAX_FILE_NAME_LEN];
  char size[MAX_FILE_SIZE_LEN];
};
struct FILELIST** fileList; // SD内のファイル情報を格納する変数
int numberOfFiles; // SD内のファイル数
uint32_t fileUsage; // SD内のファイルサイズの合計
const int FILENUM_DELETE_ALL  = -1; // SD CARD画面で[DELETE ALL]を押した時にdeleteFile()関数に渡す引数(任意の負数)
// printMode(saveEventToSdCard()内で使用する画面上の文字列表示の指定)
const int PRINTMODE_WINDOW  = 0; // 指定位置に表示
const int PRINTMODE_LINE    = 1; // 現在のカーソル位置に表示
const int PRINTMODE_NOPRINT = 2; // 表示しない
void saveEventToSdCard(String event, int screenMode = SCREEN_ON, int printMode = PRINTMODE_WINDOW);
void saveDataToSdCard(int sccreenMode = SCREEN_ON);
void printSdStatus(int screenMode = SCREEN_ON, int printMode = PRINTMODE_WINDOW);
void printResetTime(int screenMode = SCREEN_ON);

// FlashAir関係
// https://flashair-developers.com/ja/ -> 閉鎖
// https://flashair-developers.github.io/website/
#ifdef FLASH_AIR
#include "iSdio.h"
//#include "utility/Sd2CardExt.h"
#include "Sd2CardExt.h"
Sd2CardExt card;
uint8_t iSDIO_buf[512];
uint32_t nextSequenceId = 0;
char jst[] = "----/--/-- --:--:--";
int syncDay, syncHour, syncMin;
boolean syncTime(int provider = DEFAULT_PROVIDER);
char httpCommandArg1[20];
char httpCommandArg2[120];
boolean isSynchronized;
boolean existsFlashAir = false;
char syncTimeInfo[] = "--/-- --:-- ---- ";
boolean needsResetFlashAir; // trueの時、次の時刻取得の際にFlashAirをリセットする
void resetFlashAir(int sec = 5);
void onOffFlashAir(int sec = 5);
const int PROTOCOL_HTTP = 0x21;
const int PROTOCOL_HTTPS = 0x23;
const int MAX_HTTP_RESPONSE_LEN = 2048 - 24;
unsigned long prevMillis;
char flashAirIP[16];
boolean strtokEx(char *s, const char *s1, const char *s2, char *subString, size_t subStringSize);
boolean strtokEx(String s, const char *s1, char *subString, size_t subStringSize);

// 圧電スピーカー関係
const int PIN_SPEAKER = 21;
const int BEEP_OFF   = 1;
const int BEEP_ON    = 2;
const int BEEP_LOW   = 3;
const int BEEP_HIGH  = 4;
const int BEEP_PIPO  = 5;
// タイマーを使用しないtone代替関数ライブラリ
// https://bitbucket.org/teckel12/arduino-timer-free-tone/wiki/Home
#include <TimerFreeTone.h>
const int BEEP_800MS = 9;

// ************************************************************************************
// FLASH AIR制御関数
boolean iSDIO_status() {
  char buf[BUFFER_LEN_HALF];

  flashAirIP[0] = '\0';
  printEx(F("\nRead iSDIO Status Register"));
  // Read iSDIO Status Register (E7 1.10 2.2.2.1)
  memset(iSDIO_buf, 0, 0x200);
  if (!card.readExtMemory(1, 1, 0x400, 0x200, iSDIO_buf)) {
    return false;
  }
  // Show values in the common status area.
  printEx(F("\n == iSDIO Status Registers == "));
  printEx(F("\n [0400h] Command Write Status: "));
  if (iSDIO_buf[0x000] & 0x01) printEx(F("CWU "));
  if (iSDIO_buf[0x000] & 0x02) printEx(F("CWA "));
  printEx(F("\n [0420h] iSDIO Status: "));
  if (iSDIO_buf[0x020] & 0x01) printEx(F("CRU "));
  if (iSDIO_buf[0x020] & 0x02) printEx(F("ESU "));
  if (iSDIO_buf[0x020] & 0x04) printEx(F("MCU "));
  if (iSDIO_buf[0x020] & 0x08) printEx(F("ASU "));
  printEx(F("\n [0422h] iSDIO Int Enable: "));
  if (iSDIO_buf[0x022] & 0x01) printEx(F("CRU_ENA "));
  if (iSDIO_buf[0x022] & 0x02) printEx(F("ESU_ENA "));
  if (iSDIO_buf[0x022] & 0x04) printEx(F("MCU_ENA "));
  if (iSDIO_buf[0x022] & 0x08) printEx(F("ASU_ENA "));
  printEx(F("\n [0424h] Error Status: "));
  if (iSDIO_buf[0x024] & 0x01) printEx(F("CRE "));
  if (iSDIO_buf[0x024] & 0x02) printEx(F("CWE "));
  if (iSDIO_buf[0x024] & 0x04) printEx(F("RRE "));
  if (iSDIO_buf[0x024] & 0x08) printEx(F("APE "));
  printEx(F("\n [0426h] Memory Status: "));
  if (iSDIO_buf[0x026] & 0x01) printEx(F("MEX "));
  if (iSDIO_buf[0x026] & 0x02) printEx(F("FAT "));
  for (int i = 0; i < 8; ++i) {
    uint8_t addr = 0x40 + i * 0x14;
    printEx(F("\n [04"));
    printByte(addr);
    printEx(F("h] Command Response Status #"));
    sprintf(buf, "%d", i + 1);
    printEx(buf);
    printEx(F(": "));
    if (iSDIO_buf[addr] & 0x01) {
      printEx(F("id = "));
      sprintf(buf, "%d", get_u16(iSDIO_buf + addr + 2));
      printEx(buf);
      printEx(F(", sequence id = "));
      sprintf(buf, "%d", get_u16(iSDIO_buf + addr + 4));
      printEx(buf);
      printEx(F(", status = "));
      switch (iSDIO_buf[addr + 8]) {
        case 0x00: printEx(F("Initial")); break;
        case 0x01: printEx(F("Command Processing")); break;
        case 0x02: printEx(F("Command Rejected")); break;
        case 0x03: printEx(F("Process Succeeded")); break;
        case 0x04: printEx(F("Process Terminated")); break;
        default:
          printEx(F("Process Failed "));
          sprintf(buf, "%X", iSDIO_buf[addr + 8]);
          printEx(buf);
          break;
      }
    } else {
      printEx(F("Not registered"));
    }
  }
  // Show values in the application status area.
  printEx(F("\n == Wireless LAN Status Registers =="));
  printEx(F("\n [0500h] DLNA Status: "));
  if (iSDIO_buf[0x100] & 0x01) printEx(F("ULR "));
  if (iSDIO_buf[0x100] & 0x02) printEx(F("DLU "));
  if (iSDIO_buf[0x100] & 0x04) printEx(F("CBR "));
  if (iSDIO_buf[0x100] & 0x08) printEx(F("CDR "));
  printEx(F("\n [0501h] P2P Status: "));
  if (iSDIO_buf[0x101] & 0x01) printEx(F("ILU "));
  if (iSDIO_buf[0x101] & 0x02) printEx(F("FLU "));
  printEx(F("\n [0502h] PTP Status: "));
  if (iSDIO_buf[0x102] & 0x01) printEx(F("RPO "));
  if (iSDIO_buf[0x102] & 0x02) printEx(F("RPD "));
  if (iSDIO_buf[0x102] & 0x04) printEx(F("RPC "));
  if (iSDIO_buf[0x102] & 0x08) printEx(F("CPI "));
  if (iSDIO_buf[0x102] & 0x10) printEx(F("DPI "));
  if (iSDIO_buf[0x102] & 0x20) printEx(F("CIL "));
  printEx(F("\n [0504h] Application: "));
  printEx(String((char)iSDIO_buf[0x104]));
  printEx(F("\n [0506h] WLAN: "));
  if ((iSDIO_buf[0x106] & 0x01) == 0x00) printEx(F("No Scan, "));
  if ((iSDIO_buf[0x106] & 0x01) == 0x01) printEx(F("Scanning, "));
  if ((iSDIO_buf[0x106] & 0x06) == 0x00) printEx(F("No WPS, "));
  if ((iSDIO_buf[0x106] & 0x06) == 0x02) printEx(F("WPS with PIN, "));
  if ((iSDIO_buf[0x106] & 0x06) == 0x04) printEx(F("WPS with PBC, "));
  if ((iSDIO_buf[0x106] & 0x08) == 0x00) printEx(F("Group Client, "));
  if ((iSDIO_buf[0x106] & 0x08) == 0x08) printEx(F("Group Owner "));
  if ((iSDIO_buf[0x106] & 0x10) == 0x00) printEx(F("STA, "));
  if ((iSDIO_buf[0x106] & 0x10) == 0x10) printEx(F("AP, "));
  if ((iSDIO_buf[0x106] & 0x60) == 0x00) printEx(F("Initial, "));
  if ((iSDIO_buf[0x106] & 0x60) == 0x20) printEx(F("Infrastructure, "));
  if ((iSDIO_buf[0x106] & 0x60) == 0x40) printEx(F("Wi-Fi Direct, "));
  if ((iSDIO_buf[0x106] & 0x80) == 0x00) printEx(F("No Connection, "));
  if ((iSDIO_buf[0x106] & 0x80) == 0x80) printEx(F("Connected, "));
  printEx(F("\n [0508h] SSID: "));
  for (int i = 0; i < 32 && iSDIO_buf[0x108 + i] != 0; ++i) {
    printEx(String((char)iSDIO_buf[0x108 + i]));
  }
  printEx(F("\n [0528h] Encryption Mode: "));
  switch (iSDIO_buf[0x128]) {
    case 0 : printEx(F("Open System and no encryption")); break;
    case 1 : printEx(F("Open System and WEP")); break;
    case 2 : printEx(F("Shared Key and WEP")); break;
    case 3 : printEx(F("WPA-PSK and TKIP")); break;
    case 4 : printEx(F("WPA-PSK and AES")); break;
    case 5 : printEx(F("WPA2-PSK and TKIP")); break;
    case 6 : printEx(F("WPA2-PSK and AES")); break;
    default: printEx(F("Unknown"));
  }
  printEx(F("\n [0529h] Signal Strength: "));
  sprintf(buf, "%d", iSDIO_buf[0x129]);
  printEx(buf);
  printEx(F("\n [052Ah] Channel: "));
  if (iSDIO_buf[0x12A] == 0) {
    printEx(F("No connection"));
  } else {
    sprintf(buf, "%d", iSDIO_buf[0x12A]);
    printEx(buf);
  }
  printEx(F("\n [0530h] MAC Address: "));
  printBytes(iSDIO_buf + 0x130, 6);
  printEx(F("\n [0540h] ID: "));
  for (int i = 0; i < 16 && iSDIO_buf[0x140 + i] != 0; ++i) {
    printEx(String((char)iSDIO_buf[0x140 + i]));
  }
  printEx(F("\n [0550h] IP Address: "));
  //printIPAddress(iSDIO_buf + 0x150);
  convIPAddressToString(iSDIO_buf + 0x150, flashAirIP);
  printEx(flashAirIP);
  if (strcmp(flashAirIP, "192.168.0.1") == 0) {
    flashAirIP[0] = '\0';
  }
  if (strcmp(flashAirIP, "0.0.0.0") == 0) {
    flashAirIP[0] = '\0';
  }
  printEx(F("\n [0554h] Subnet Mask: "));
  printIPAddress(iSDIO_buf + 0x154);
  printEx(F("\n [0558h] Default Gateway: "));
  printIPAddress(iSDIO_buf + 0x158);
  printEx(F("\n [055Ch] Preferred DNS Server: "));
  printIPAddress(iSDIO_buf + 0x15C);
  printEx(F("\n [0560h] Alternate DNS Server: "));
  printIPAddress(iSDIO_buf + 0x160);
  printEx(F("\n [0564h] Proxy Server: "));
  if ((iSDIO_buf[0x164] & 0x01) == 0x00) printEx(F("Disabled"));
  if ((iSDIO_buf[0x164] & 0x01) == 0x01) printEx(F("Enabled"));
  printEx(F("\n [0570h] Date: "));
  sprintf(buf, "%04d-%02d-%02d", iSDIO_buf[0x171] + 1980, iSDIO_buf[0x170] >> 4, iSDIO_buf[0x170] & 0xF);
  printEx(buf);
  printEx(F("\n [0572h] Time: "));
  sprintf(buf, "%02d:%02d:%02d", iSDIO_buf[0x173] >> 3, iSDIO_buf[0x172] << 3 | iSDIO_buf[0x170] >> 3, (iSDIO_buf[0x172] & 0x1F) * 2);
  printEx(buf);
  printEx(F("\n [0574h] HTTP Status: "));
  sprintf(buf, "%d", iSDIO_buf[0x174] & 0xEF);
  printEx(buf);
  if ((iSDIO_buf[0x174] & 0x80) == 0x00) printEx(F(" (No Processing)"));
  if ((iSDIO_buf[0x174] & 0x80) == 0x80) printEx(F(" (Processing)"));
  printEx(F("\n [0575h] Power Save Management: "));
  if ((iSDIO_buf[0x175] & 0x01) == 0x00) printEx(F("Power Save Mode Off"));
  if ((iSDIO_buf[0x175] & 0x01) == 0x01) printEx(F("Power Save Mode On"));
  printEx(F("\n [0576h] File System Management: "));
  if ((iSDIO_buf[0x176] & 0x01) == 0x00) printEx(F("FS Information may be modified"));
  if ((iSDIO_buf[0x176] & 0x01) == 0x01) printEx(F("FS Information shall not be modified"));
  printEx(F("\n"));
  if (flashAirIP[0] == '\0') {
    return false;
  }
  return true;
}

boolean iSDIO_waitHttpResponse(uint32_t sequenceId) {
  char buf[BUFFER_LEN_HALF];

  printEx(F("\nWaiting response"));
  uint8_t prev = 0xFF;
  for (int i = 0; i < 20; ++i) {
    memset(iSDIO_buf, 0, 0x14);
    // Read command response status.
    if (!card.readExtMemory(1, 1, 0x440, 0x14, iSDIO_buf)) {
      return false;
    }
    uint8_t resp = get_u8(iSDIO_buf + 8);
    if (sequenceId == get_u32(iSDIO_buf + 4)) {
      if (prev != resp) {
        switch (resp) {
          case 0x00:
            printEx(F("\n  Initial"));
            break;
          case 0x01:
            printEx(F("\n  Command Processing"));
            break;
          case 0x02:
            printEx(F("\n  Command Rejected"));
            return false;
          case 0x03:
            printEx(F("\n  Process Succeeded"));
            return true;
          case 0x04:
            printEx(F("\n  Process Terminated"));
            return false;
          default:
            // 80h to FFh: Process Failed where
            // 80h: general error
            // 81h: argument error
            // 82h: network error
            // 83h: file system error
            // 84h: buffer overflow error
            // 85h to BFh: reserved
            // C0h to FFh: reserved for each application specification
            // FFh=20秒以上過ぎた時かステータスの読み出し異常の時です。
            printEx(F("\n  Process Failed "));
            sprintf(buf, "%X", resp);
            printEx(buf);
            return false;
        }
        prev = resp;
      }
    }
    printEx(F("."));
    delay(1000);
  }
  return false;
}

boolean iSDIO_sendHttpRequest(uint32_t sequenceId, int protocol = PROTOCOL_HTTP) {
  int i;

  clearScreen();
  printEx(F("\nsending HTTP Request...\n"));
  memset(iSDIO_buf, 0, 512);
  uint8_t* p = iSDIO_buf;
  p = put_command_header(p, 1, 0);
  p = put_command_info_header(p, protocol, sequenceId, 2);
  // put_command_info_header()関数の第2引数(protocol)について
  // protocol=PROTOCOL_HTTPS
  //  NICTは問題なし、WorldTimeApiは card.readExtMemory()は成功してもレスポンスがエラー(エラーコード:C0)のため不可
  // protocol=PROTOCOL_HTTP
  //  どちらも問題なし
  // PROTOCOL_HTTPS(コマンドID=23h) -> コマンド名称:SendHTTPSSLMessageByRegister(プロトコル:HTTP over SSL)
  // PROTOCOL_HTTP (コマンドID=21h) -> コマンド名称:SendHTTPMessageByRegister(プロトコル:HTTP)
  p = put_str_arg(p, httpCommandArg1); // Argument #1.
  p = put_str_arg(p, httpCommandArg2); // Argument #2.
  put_command_header(iSDIO_buf, 1, (p - iSDIO_buf));
  //printHex(iSDIO_buf, (p - iSDIO_buf));
  printEx(httpCommandArg1);
  printEx(F("\n"));
  printEx(httpCommandArg2);
  printEx(F("\n"));
  printEx(F("Waiting until writeExtDataPort command"));
  // ここで7秒以上待たないとレスポンスが返ってこない
  //for (i = 0; i < 7; i++) {
  for (i = 0; i < 10; i++) {
    printEx(F("."));
    delay(1000);
  }
  printEx(F("\n"));
  if (card.writeExtDataPort(1, 1, 0x000, iSDIO_buf) == false) {
    printEx(F("writeExtDataPort Failed"));
    return false;
  }
  printEx(F("writeExtDataPort Succeeded"));
  return true;
}

boolean iSDIO_readHttpResponse(char* httpResponse, int httpReponseSize) {
  char responseCode[100];
  char buf[100];
  int statusNum;
  char statusMsg[100];

  printEx(F("\nreading HTTP response...\n"));
  httpResponse[0] = '\0';
  // Read header and data.
  if (!card.readExtDataPort(1, 1, 0x200, iSDIO_buf)) {
    printEx(F("readExtDataPort command(Read header and data) failed"));
    return false;
  }
  uint32_t totalSize = get_u32(iSDIO_buf + 20);
  if (totalSize == 0) {
    printEx(F("readExtDataPort totalData == 0"));
    return false;
  }
  prevMillis = millis();
  uint32_t availableSize = totalSize > 488 ? 488 : totalSize;
  uint32_t pos = 24;
  uint32_t j = 0;
  for (;;) {
    for (uint32_t i = 0; i < availableSize; ++i) {
      if (j == (httpReponseSize - 1)) {
        break;
      }
      httpResponse[j] = (char)iSDIO_buf[pos + i];
      j++;
    }
    totalSize -= availableSize;

    // Have we read all data?
    if (totalSize == 0) break;

    // Read next data.
    if (!card.readExtDataPort(1, 1, 0x200, iSDIO_buf)) {
      printEx(F("readExtDataPort command(Read next data) failed"));
      return false;
    }
    availableSize = totalSize > 512 ? 512 : totalSize;
    pos = 0;
  }
  httpResponse[j] = '\0';
  clearScreen();
  printEx(httpResponse);
  if (j == (httpReponseSize - 1)) {
    saveEventToSdCard(F("httpResponseSize not enough"), currScreenMode, PRINTMODE_LINE);
    return false;
  }
  if (strtokEx(httpResponse, "HTTP/1.1 ", responseCode, sizeof(responseCode)) == false) {
    saveEventToSdCard(F("HTTP response code not found"), currScreenMode, PRINTMODE_LINE);
    return false;
  }
  trimCrLf(responseCode);
  sscanf(responseCode, "HTTP/1.1 %d %s", &statusNum, statusMsg);
  if (statusNum != 200) {
    sprintf(buf, "HTTP response error(status %d %s)", statusNum, statusMsg);
    saveEventToSdCard(buf, currScreenMode, PRINTMODE_LINE);
  }
  return true;
}

boolean syncTime(int provider) {
  switch (provider) {
    case PROVIDER_WORLD_TIME_API:
      return syncTimeWithWorldTimeApi();
    case PROVIDER_NICT:
      return syncTimeWithNict();
  }
}

boolean syncTimeWithWorldTimeApi(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, month, day, hour, minute, second;
  long microsecond;
  char datetime[60];

  isSynchronized = false;
  // 日本標準時を取得
  // http://worldtimeapi.org/api/timezone/Asia/Tokyo.txt
  for (numberOfTimes = 1; numberOfTimes <= MAX_NUMBER_OF_TIMES; numberOfTimes++) {
    if (numberOfTimes == 1) {
      strcpy(httpCommandArg1, "worldtimeapi.org");
      strcpy(httpCommandArg2,
             "GET /api/timezone/Asia/Tokyo.txt HTTP/1.1\r\n"
             "Host: worldtimeapi.org\r\n"
             "User-Agent: arduino\r\n"
             "Connection: close\r\n"
             "\r\n");
      if (iSDIO_sendHttpRequest(nextSequenceId)
          && iSDIO_waitHttpResponse(nextSequenceId)) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    } else {
      resetFlashAir();
      if (iSDIO_sendHttpRequest(nextSequenceId)
          && iSDIO_waitHttpResponse(nextSequenceId)) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    }
  }
  if (isSynchronized == false) {
    printEx(F("\nFailed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printEx(buf);
    requestFlashAirReset();
  } else {
    // httpResponseから時刻情報のある行を取り出す
    // datetime: 2021-01-23T20:38:02.410914+09:00
    isSynchronized = strtokEx(httpResponse, "datetime: ", datetime, sizeof(datetime));
    if (isSynchronized == false) {
      printEx(F("\nTime information not found"));
    } else {
      sscanf(datetime, "datetime: %04d-%02d-%02dT%02d:%02d:%02d.%06ld+09:00",
             &year, &month, &day, &hour, &minute, &second, &microsecond);
      // microsecondは使わない
      if (year > 2020) {
        adjustTimeByJst(year, month, day, hour, minute, second);
        printEx(F("\nSynchronization succeeded"));
      }
    }
  }
  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lX ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isSynchronized;
}

// NICTによる時刻配信サービス停止期間は使用不可
boolean syncTimeWithNict(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, day, hour, minute, second;
  char month[4], dayOfWeek[4];
  int j;
  char datetime[30];

  isSynchronized = false;
  // 日本標準時を取得
  // https://www.nict.go.jp/JST/http.html
  for (numberOfTimes = 1; numberOfTimes <= MAX_NUMBER_OF_TIMES; numberOfTimes++) {
    if (numberOfTimes == 1) {
      strcpy(httpCommandArg1, "ntp-b1.nict.go.jp");
      strcpy(httpCommandArg2, "GET /cgi-bin/time HTTP/1.1\r\nHost: ntp-b1.nict.go.jp\r\n\r\n");
      if (iSDIO_sendHttpRequest(nextSequenceId)
          && iSDIO_waitHttpResponse(nextSequenceId)) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    } else {
      resetFlashAir();
      if (iSDIO_sendHttpRequest(nextSequenceId)
          && iSDIO_waitHttpResponse(nextSequenceId)) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    }
  }
  if (isSynchronized == false) {
    printEx(F("\nFailed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printEx(buf);
    requestFlashAirReset();
  } else {
    // httpResponseから時刻情報のある行を取り出す
    // Mon Sep 04 21:28:18 2017 JST
    isSynchronized = strtokEx(httpResponse, " JST", datetime, sizeof(datetime));
    if (isSynchronized == false) {
      printEx(F("\nTime information not found"));
    } else {
      sscanf(buf, "%3s %3s %02d %02d:%02d:%02d %04d JST",
             dayOfWeek, month, &day, &hour, &minute, &second, &year);
      // dayOfWeekは使わない
      if (year > 2020) {
        for (j = 1; j < 13; j++) {
          if (strcmp(monthShortStr(j), month)  == 0) {
            break;
          }
        }
        adjustTimeByJst(year, j, day, hour, minute, second);
        printEx(F("\nSynchronization succeeded"));
      }
    }
  }
  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lX ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isSynchronized;
}

void syncTimeOnceADay(void) {
  char event[BUFFER_LEN_EVENT];
  int prevScreenMode;

  if (hasBlackout == true) {
    return;
  }

  if ((existsFlashAir == true) && (eepromAutoTimeSync == ON)) {
    Time t = getClockTime();
    if ((t.date != syncDay) && (t.hr == eepromTimeSyncHour) && (t.min == eepromTimeSyncMin)) {
      syncDay = t.date;
      prevScreenMode = currScreenMode;
      turnOnScreen();
      if (needsResetFlashAir == true) {
        resetFlashAir();
      }
      if (syncTime() == true) {
        sprintf(event, "[%04lX][%s]Time synchronized (automatically)", nextSequenceId - 1, httpCommandArg1);
        saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
      } else {
        sprintf(event, "[%04lX][%s]Time synchronization failed (automatically)", nextSequenceId - 1, httpCommandArg1);
        saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
      }
      initPrevClockValues();
      if (prevScreenMode == SCREEN_ON) {
        currScreenMode = printMainWindow(SCREEN_REDRAW);
      } else {
        currScreenMode = printMainWindow(prevScreenMode);
      }
    }
  }
  return;
}

void onOffFlashAir(int sec){
    myGLCD.setFont(SmallFont);
    myGLCD.setColor(VGA_WHITE);
    digitalWrite(PIN_SDCARD_VDD, LOW);
    printEx(F("FlashAir turned off\n"));
    delay(sec * 1000);
    digitalWrite(PIN_SDCARD_VDD, HIGH);
    printEx(F("FlashAir turned on\n"));
    delay(5000);
}
    
void(*resetFunc)(void) = 0;

void resetSystem(){
    clearScreen();
    myGLCD.setColor(VGA_YELLOW);
    myGLCD.setFont(BigFont);
    myGLCD.print(F("System Resetting..."), 0, 0);
    curX = 0;
    curY = 18;
    onOffFlashAir();
    myGLCD.clrScr();
    resetFunc();
    //ここへは到達しない
}

boolean getCardStatus(void) {
  // Initialize SD card.
  printEx(F("Initializing FlashAir..."));
  if (card.init(SPI_HALF_SPEED, PIN_SDCARD_CS)) {
    printEx(F("OK"));
  } else {
    printEx(F("NG"));
    printEx(F("\nIf the screen touched, reset the system"));
    while (getButtonNum() == BTN_NOT_PRESSED);
    resetSystem();
    //ここへは到達しない
  }

  // Read the previous sequence ID.
  if (card.readExtMemory(1, 1, 0x420, 0x34, iSDIO_buf)) {
    if (iSDIO_buf[0x20] == 0x01) {
      nextSequenceId = get_u32(iSDIO_buf + 0x24);
      iSDIO_waitHttpResponse(nextSequenceId);
      nextSequenceId++;
    } else {
      nextSequenceId = 0;
    }
  } else {
    printEx(F("\nFailed to read FlashAir memory."));
    nextSequenceId = 0;
    saveEventToSdCard(F("Failed to read FlashAir memory"), currScreenMode, PRINTMODE_LINE);
    delay(3000);
    return false;
  }

  if (!iSDIO_status()) {
    printEx(F("\nFailed to read FlashAir status."));
    saveEventToSdCard(F("Failed to read FlashAir status"), currScreenMode, PRINTMODE_LINE);
    delay(3000);
    return false;
  }
  return true;
}

void requestFlashAirReset(void) {
  needsResetFlashAir = true;
  syncTimeInfo[16] = 'R';
  syncTimeInfo[17] = '\0';
  if (currScreenMode == SCREEN_ON) {
    printSyncTimeInfo();
  }
}

void resetFlashAir(int sec) {
  clearScreen();
  onOffFlashAir(sec);
  memset(iSDIO_buf, 0, 512);
  nextSequenceId = 0;
  syncTimeInfo[16] = ' ';
  needsResetFlashAir = false;
  if (getCardStatus() == true) {
    saveEventToSdCard(F("FlashAir reset"), currScreenMode, PRINTMODE_LINE);
  }
}
#endif

// ************************************************************************************
// 文字列操作関数
// s内を検索しs1から始まりs2で終わる文字列をsubStringにコピーする
boolean strtokEx(char *s, const char *s1, const char *s2, char *subString, size_t subStringSize) {
  char *startIndex, *endIndex;

  startIndex = strstr(s, s1);
  if (startIndex == NULL) {
    return false;
  }
  endIndex = strstr(startIndex, s2);
  if (endIndex == NULL) {
    return false;
  }
  if ((endIndex - startIndex) > subStringSize) {
    return false;
  }
  strncpy(subString, startIndex, endIndex - startIndex);
  subString[endIndex - startIndex] = '\0';
  return true;
}

// s内を検索しs1が含まれる行をsubStringにコピーする
boolean strtokEx(String s, const char *s1, char *subString, size_t subStringSize) {
  char *index, *startIndex, *endIndex;

  index = s.indexOf(s1);
  if (index == -1) {
    return false;
  }
  startIndex = s.lastIndexOf("\n", index);
  if (startIndex == -1) {
    startIndex = 0;
  } else {
    startIndex++;
  }
  endIndex = s.indexOf("\n", startIndex);
  if (endIndex == -1) {
    endIndex = s.length() + 1;
  }
  if ((endIndex - startIndex) > subStringSize) {
    return false;
  }
  s.substring(startIndex, endIndex).toCharArray(subString, subStringSize);
  return true;
}

// 文字列内の改行を取り除く関数
//http://goldilocks-engineering.blogspot.com/2015/09/c.html
char* trimCrLf(char *str) {
  char *p;

  p = strchr(str, '\r');
  if (p != NULL) {
    *p = '\0';
  }
  p = strchr(str, '\n');
  if (p != NULL) {
    *p = '\0';
  }
  return str;
}

// ************************************************************************************
// 日付・時刻関連関数
//  millis()オーバーフロー対策関数
unsigned long getIntervalTime(unsigned long t1) {
  unsigned long t2 = millis();

  if (t2 < t1) {
    return (4294967295UL - t1 + t2);
  } else {
    return (t2 - t1);
  }
}

int getDayOfWeek(int y, int m, int d ) {
  //Zellerの公式
  //1・2月は前年の13・14月として計算する
  if ( m < 3 ) {
    y--;
    m += 12;
  }
  // 戻り値:曜日 -> 0:日曜日 1:月曜日 ... 6:土曜日
  return ( y + y / 4 - y / 100 + y / 400 + ( 13 * m + 8 ) / 5 + d ) % 7;
}

Time getClockTime(void) {
  if (clockMode == CLOCK_DS1302) {
    noInterrupts();
    Time t = ds1302.time();
    interrupts();
    return t;
  } else {
    tmElements_t tm;

    breakTime(now(), tm);
    //Time t(2013, 9, 22, 1, 38, 50, Time::kSunday);
    Time t(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second,
           Time::Day(getDayOfWeek(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day) + 1));
    return t;
  }
}

void setClockTime(Time t) {
  tmElements_t tm;

  if (clockMode == CLOCK_DS1302) {
    noInterrupts();
    ds1302.time(t);
    interrupts();
  } else {
    tm.Year = CalendarYrToTm(t.yr);
    tm.Month = t.mon;
    tm.Day = t.date;
    tm.Wday = getDayOfWeek(t.yr, t.mon, t.date) + 1;
    tm.Hour = t.hr;
    tm.Minute = t.min;
    tm.Second = t.sec;
    // tm構造体変数 -> システムの時刻を設定
    setTime(makeTime(tm));
  }
}

Time adjustIntlClock(int year, int month, int day, int hour, int minute, int second) {
  // 取得した時刻 -> tm構造体変数
  tmElements_t tm;

  tm.Year = CalendarYrToTm(year);
  tm.Month = month;
  tm.Day = day;
  tm.Wday = getDayOfWeek(year, month, day) + 1;
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = second;
  // tm構造体変数 -> システムの時刻を設定
  setTime(makeTime(tm));
  // 時刻取得からRTCへの時刻セットまでの処理に要する時間(秒未満切り上げ)を補正
  unsigned long delayMillis = millis() - prevMillis;
  if (delayMillis < 30000) { // 30秒以上掛かっている場合は何か異常が発生しているものとして補正しない
    adjustTime(int(delayMillis / 1000) + 1);
  }
  breakTime(now(), tm);
  //Time t(2013, 9, 22, 1, 38, 50, Time::kSunday);
  Time t(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second,
         Time::Day(getDayOfWeek(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day) + 1));
  return t;
}

void adjustTimeByJst(int year, int month, int day, int hour, int minute, int second) {
  // World Time API -> datetime: 2021-01-23T20:38:02.410914+09:00 ('\0'含めて43文字)
  // NICT -> Mon Sep 04 21:28:18 2017 JST ('\0'含めて30文字)
  char buf[50]; // 時刻情報の行が格納できるサイズ

  clearScreen();
  // RTC
  Time t = getClockTime();
  sprintf(buf, "RTC: %04d/%02d/%02d %3s %02d:%02d:%02d",
          t.yr, t.mon, t.date, dayShortStr(t.day), t.hr, t.min, t.sec);
  printEx(buf);
  // JST
  t = adjustIntlClock(year, month, day, hour, minute, second);
  noInterrupts();
  ds1302.time(t);
  interrupts();
  // TimeLib.h > DateStrings.cpp > dayShortStr() 引数:戻り値 -> 0:Err 1:Sun 2:Mon ... 7:Sat
  sprintf(buf, "%04d/%02d/%02d %3s %02d:%02d:%02d",
          t.yr, t.mon, t.date, dayShortStr(t.day), t.hr, t.min, t.sec);
  printEx(F("\nJST: "));
  printEx(buf);
  sprintf(jst, "%02d/%02d/%02d %02d:%02d:%02d",
          t.yr % 100, t.mon, t.date, t.hr, t.min, t.sec);
  syncDay = t.date;
  syncHour = t.hr;
  syncMin = t.min;
}

void adjustIntlClockByRtc(void) {
  if (clockMode == CLOCK_INTERNAL) {
    Time t = getClockTime();
    if ((t.sec == 30) && ((t.min * 60 + t.sec) != prevSec)) {
      prevSec = t.min * 60 + t.sec;
      noInterrupts();
      t = ds1302.time();
      interrupts();
      setClockTime(t);
    }
  }
  return;
}

// ************************************************************************************
// ビープ音関数
void beep(int mode) {
  switch (mode) {
    case BEEP_OFF:
      if (eepromSound == ON) {
        noTone(PIN_SPEAKER);
      }
      delay(50);
      return;
    case BEEP_ON:
      if (eepromSound == ON) {
        tone(PIN_SPEAKER, 800);
      }
      delay(50);
      return;
    case BEEP_LOW:
      if (eepromSound == ON) {
        tone(PIN_SPEAKER, 400, 50);
      }
      delay(50);
      break;
    case BEEP_HIGH:
      if (eepromSound == ON) {
        tone(PIN_SPEAKER, 800, 50);
      }
      delay(50);
      break;
    case BEEP_PIPO: // PC9800シリーズ起動音
      tone(PIN_SPEAKER, 2000, 150);
      delay(150);
      tone(PIN_SPEAKER, 1000, 150);
      delay(150);
      break;
    case BEEP_800MS: // タイマー不使用の TimerFreeTone() を使用
      if (eepromSound == ON) {
        TimerFreeTone(PIN_SPEAKER, 800, 800);
      }
      delay(50);
      break;
  }
}

// ************************************************************************************
// 電力量計算関数
void sampleFromSensor(int sensorNum) {
  unsigned long t1, t2;
  int i, r, v1, a1, a2, v2;
  float vv, aa;

  t1 = micros();
  for (i = 0; i < NUMBER_OF_SAMPLES; i++) {
    r = analogRead(PIN_AREF);
    v1 = analogRead(PIN_VOLTAGE);
    a1 = analogRead(PIN_SENSOR[sensorNum]);
    a2 = analogRead(PIN_SENSOR[sensorNum]);
    v2 = analogRead(PIN_VOLTAGE);
    VASamples[(i * 4) + 0] = v1 - r;
    VASamples[(i * 4) + 1] = a1 - r;
    VASamples[(i * 4) + 2] = a2 - r;
    VASamples[(i * 4) + 3] = v2 - r;
    do {
      t2 = micros();
    } while ((t2 - t1) < SAMPLING_PERIOD);
    t1 += SAMPLING_PERIOD;
  }
  // １サイクル分の電圧と電流、電力を計算
  vrms[sensorNum] = 0.0;
  irms[sensorNum] = 0.0;
  watt[sensorNum] = 0.0;
  for (i = 0; i < NUMBER_OF_SAMPLES; i++) {
    v1 = VASamples[(i * 4) + 0];
    a1 = VASamples[(i * 4) + 1];
    a2 = VASamples[(i * 4) + 2];
    v2 = VASamples[(i * 4) + 3];
    vv = ((((v1 + v2) / 2) * VCC) / 1024) * KVT;
    vv = vv * (float)eepromVtPercentage / 100.0; // 誤差補正
    aa = ((((a1 + a2) / 2) * VCC) / 1024) / KCT[sensorNum];
    aa = aa * (float)eepromCtPercentage[sensorNum] / 100.0 ; // 誤差補正
    vrms[sensorNum] += vv * vv;
    irms[sensorNum] += aa * aa;
    watt[sensorNum] += vv * aa;
  }
  // 2乗平均平方根(rms)を求める
  vrms[sensorNum] = sqrt(vrms[sensorNum] / NUMBER_OF_SAMPLES);
  irms[sensorNum] = sqrt(irms[sensorNum] / NUMBER_OF_SAMPLES);
  // 平均電力を求める
  watt[sensorNum] = watt[sensorNum] / NUMBER_OF_SAMPLES;
  // 測定していないセンサーは結果を0にする
  // 200V回路のセンサーは結果を2倍する
  switch (eepromCtVoltage[sensorNum]) {
    case 0:
      irms[sensorNum] = 0.0;
      watt[sensorNum] = 0.0;
      break;
    case 200:
      vrms[sensorNum] = vrms[sensorNum] * 2.0;
      watt[sensorNum] = watt[sensorNum] * 2.0;
      break;
  }
  // 0.2A未満は測定誤差として切り捨て
  if (irms[sensorNum] < 0.2) {
    irms[sensorNum] = 0.0;
  }
  irms_max[sensorNum] = max(irms_max[sensorNum] , irms[sensorNum]);
  // 電圧と電流が逆相の場合の対策
  watt[sensorNum] = abs(watt[sensorNum]);
  // 20W(200V回路のセンサーは40W)未満は測定誤差として切り捨て
  if (watt[sensorNum] < (0.2 * eepromCtVoltage[sensorNum])) {
    watt[sensorNum] = 0.0;
  }
  watt_max[sensorNum] = max(watt_max[sensorNum] , watt[sensorNum]);
}

void timer1(void) {
  switch (timer1_state) {
    case STATE_STOP:
      break;
    case STATE_SENSOR1:
      timer1_state = STATE_SENSOR2;
      sampleFromSensor(1);
      break;
    case STATE_SENSOR2:
      timer1_state = STATE_SENSOR3;
      sampleFromSensor(2);
      break;
    case STATE_SENSOR3:
      timer1_state = STATE_CALCULATE;
      sampleFromSensor(3);
      break;
    case STATE_CALCULATE:
      timer1_state = STATE_SENSOR1;
      watt[0] = watt[1] + watt[2] + watt[3];
      watt_max[0] = max(watt_max[0] , watt[0]);
      watt_sum1 = watt[1];
      watt_sum2 = watt[2];
      watt_sum3 = watt[3];
      watt_sum = watt_sum1 + watt_sum2 + watt_sum3;
      watt_hour1 += watt_sum1 / 3600.0;
      watt_hour2 += watt_sum2 / 3600.0;
      watt_hour3 += watt_sum3 / 3600.0;
      watt_hour += watt_sum / 3600.0;
      // 時間帯別に積算する
      Time t = getClockTime();
      if (t.hr < 7) {
        wattHourTimeperiod4 += watt_sum / 3600.0;
      } else if (t.hr < 10) {
        wattHourTimeperiod1 += watt_sum / 3600.0;
      } else if (t.hr < 17) {
        wattHourTimeperiod2 += watt_sum / 3600.0;
      } else if (t.hr < 23) {
        wattHourTimeperiod3 += watt_sum / 3600.0;
      } else {
        wattHourTimeperiod4 += watt_sum / 3600.0;
      }
      wattHourTimeperiod += watt_sum / 3600.0;
      // 停電、電源電圧低下検出
      if (vrms[1] < 1.0) {
        if (hasBlackout == false) {
          hasBlackout = true;
          saveEventToSdCard(F("Power down"), currScreenMode);
          break;
        }
      } else if (vrms[1] < 80.0) {
        if (hasBrownout == false) {
          hasBrownout = true;
          saveEventToSdCard(F("Power voltage dropped"), currScreenMode);
          break;
        }
      }
      // 復電、電源電圧復帰検出
      if (vrms[1] >= 90.0) {
        if (hasBlackout == true) {
          hasBlackout = false;
          hasBrownout = false;
          requestFlashAirReset();
          saveEventToSdCard(F("Power restored"), currScreenMode);
        }
        if (hasBrownout == true) {
          hasBrownout = false;
          requestFlashAirReset();
          saveEventToSdCard(F("Power voltage recovered"), currScreenMode);
        }
      }
      break;
    default:
      //何もしない
      break;
  }
}

// ************************************************************************************
// ボタン押下判定関数
int getButtonNum(void) {
  int x, y;

  if (myTouch.dataAvailable()) {
    beep(BEEP_LOW);
    if ((currScreenMode == SCREEN_OFF) || (currScreenMode == SCREEN_CLOCK)) {
      return BTN_SCREEN_ON;
    }
    myTouch.read();
    x = myTouch.getX();
    y = myTouch.getY();
    if ((y >= 209) && (y <= 239)) {
      // ボタン1
      if ((x >= 0) && (x <= 60)) {
        return BTN_RESET;
      }
      // ボタン2
      if ((x >= 64) && (x <= 124)) {
        return  BTN_ADJUST_TIME;
      }
      // ボタン3
      if ((x >= 128) && (x <= 188)) {
        return BTN_CONFIG;
      }
      // ボタン4
      if ((x >= 192) && (x <= 252)) {
        return BTN_SDCARD;
      }
      // ボタン5
      if ((x >= 256) && (x <= 316)) {
        return BTN_SCREEN_OFF;
      }
    }
    return BTN_PRESSED;
  }
  return BTN_NOT_PRESSED;
}

// ************************************************************************************
// 画面表示関連関数
void turnOnScreen(void) {
  digitalWrite(PIN_LCD_LED, LOW);
}
void turnOffScreen(void) {
  digitalWrite(PIN_LCD_LED, HIGH);
}

void clearScreen(int ms) {
  delay(ms);
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  curX = 0;
  curY = 0;
}

void printCrLf(void) {
  // SmallFont専用
  if (curY >= (240 - 12)) {
    clearScreen();
  } else {
    curX = 0;
    curY = curY + 12;
  }
}

// 現在のカーソル位置を管理し、座標指定なしに文字列表示
// SmallFont専用
void printEx(String s) {
  int i, len;

  len = s.length();
  i = 0;
  while (i < len) {
    if (curX >= 320) {
      printCrLf();
    }
    if (s.charAt(i) == '\r') {
      i++;
      continue;
    }
    if (s.charAt(i) == '\n') {
      printCrLf();
      i++;
      continue;
    }
    // 制御文字は除外する
    if ((s.charAt(i) <= 31) || (s.charAt(i) >= 127)) {
      i++;
      continue;
    }
    myGLCD.printChar(s.charAt(i), curX, curY);
    curX = curX + 8;
    i++;
  }
}

// SmallFont専用
void printHexEx(String s) {
  int i, len;
  char buf[3];

  len = s.length();
  i = 0;
  while (i < len) {
    if (curX >= 320) {
      printCrLf();
    }
    sprintf(buf, "%02x", s.charAt(i));
    myGLCD.print(buf, curX, curY);
    curX = curX + 8 * 2;
    i++;
  }
}

void printByte(uint8_t value) {
  char buf[5];

  sprintf(buf, "%01X%01X", value >> 4, value & 0xF);
  printEx(buf);
}

void printBytes(uint8_t* p, uint32_t len) {
  for (int i = 0; i < len; ++i) {
    printByte(p[i]);
  }
}

void convIPAddressToString(uint8_t* p, char* s) {
  // 変数sのサイズは16byte以上必要
  sprintf(s, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

void printIPAddress(uint8_t* p) {
  char buf[16];

  convIPAddressToString(p, buf);
  printEx(buf);
}

void printHex(uint8_t* p, uint32_t len) {
  int i = 0;
  while (i < len) {
    if ((i & 0xf) == 0) {
      printEx(F("\n"));
      printByte(i >> 4);
      printEx(F(": "));
    }
    printByte(*p++);
    i++;
  }
  printEx(F("\n"));
}

void printString(char* s1, int x1, int y1, int status) {
  if (status == STS_NORMAL) {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);
  } else {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_WHITE);
  }
  myGLCD.print(s1, x1, y1);
}

void printStringEx(char* s, int x, int y, unsigned int c, unsigned int bc) {
  // 座標、色を指定する
  myGLCD.setColor(c);
  myGLCD.setBackColor(bc);
  myGLCD.print(s, x, y);
}

// 変化のあった桁のみ更新して表示を高速化する
// 桁、座標、文字サイズ、色を指定する
void printStringExEx(char* s1, char* s2, unsigned int digit, int x, int y, int font, unsigned long c) {
  unsigned int i;
  char s[2];
  uint8_t* pFont;
  int fontSizeX;

  switch (font) {
    case FONT_SMALL_FONT: myGLCD.setFont(SmallFont); break;
    case FONT_BIG_FONT: myGLCD.setFont(BigFont); break;
    case FONT_SEVEN_SEG_NUM_FONT: myGLCD.setFont(SevenSegNumFont); break;
    //case FONT_SIXTEEN_SEGMENT_32X48: myGLCD.setFont(SixteenSegment32x48); break;
    case FONT_GROTESK_BOLD_32X64: myGLCD.setFont(GroteskBold32x64); break;
    case FONT_SEVEN_SEG_XXXL_NUM: myGLCD.setFont(SevenSeg_XXXL_Num); break;
    default:
      break;
  }
  pFont = myGLCD.getFont();
  if (pFont ==  SmallFont) fontSizeX = 8;
  if (pFont ==  BigFont) fontSizeX = 16;
  if (pFont ==  SevenSegNumFont) fontSizeX = 32;
  //if (pFont ==  SixteenSegment32x48) fontSizeX = 32;
  if (pFont ==  GroteskBold32x64) fontSizeX = 32;
  if (pFont ==  SevenSeg_XXXL_Num) fontSizeX = 64;
  if (c != VGA_NO_CHANGE) {
    myGLCD.setColor(c);
  }
  for (i = 0; i < digit; i++) {
    if ((s1[0] == '\0') || (s2[i] != s1[i])) {
      s[0] = s2[i];
      s[1] = '\0';
      myGLCD.print(s, x + i * fontSizeX, y);
    }
  }
  if ((font != FONT_NO_CHANGE) && (pFont != SmallFont)) {
    myGLCD.setFont(SmallFont);
  }
}

void countupTimer(void) {
  if (scrOffTimerInterval.check() == 1) {
    if (hasBlackout == true) { //停電中
      //scrOffTimer2を1分毎に増加させる
      if (scrOffTimer2 >= 0) {
        scrOffTimer2++;
      }
    } else {
      if ((eepromScreenOffTime > 0) && (currScreenMode != SCREEN_OFF)) {
        scrOffTimer1++;
      }
    }
  }
}

void setScreenMode(void) {
  if ((currScreenMode == SCREEN_OFF) || (currScreenMode == SCREEN_CLOCK)) {
    // SCREEN_OFF(またはSCREEN_CLOCK)状態で
    if ((hasBlackout == true) && (scrOffTimer2 == TIMER_RESET)) {
      // 停電が発生した場合
      currScreenMode = printMainWindow(SCREEN_ON);
    }
    if ((hasBlackout == false) && (scrOffTimer2 != TIMER_RESET)) {
      // 復電した場合
      currScreenMode = printMainWindow(SCREEN_ON);
    }
  } else {
    if (hasBlackout == true) {
      // 停電中は自動消灯を実行しない
    } else {
      // 通電中
      if ((eepromScreenOffTime != 0) && (scrOffTimer1 >= eepromScreenOffTime)) {
        // 最後の操作から一定時間が経過したら表示を消す
        if (eepromClock == ON) {
          currScreenMode = SCREEN_CLOCK;
        } else {
          currScreenMode = SCREEN_OFF;
        }
        currScreenMode = printMainWindow(currScreenMode);
      }
    }
  }
}

int printMainWindow(int screenMode) {
  char buf[BUFFER_LEN_FULL];

  switch (screenMode) {
    case SCREEN_OFF:
      myGLCD.clrScr();
      turnOffScreen();
      return screenMode;
    case SCREEN_CLOCK:
      if (hasBlackout == false) {
        printClock();
      } else {
        myGLCD.clrScr();
        turnOffScreen();
      }
      return screenMode;
    case SCREEN_ON:
      turnOnScreen();
      break;
  }
  screenMode = SCREEN_ON;
  myGLCD.clrScr();
  printDateTime();
  initPrevValues();
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("Voltage", 0, 18);
  myGLCD.print("No.1         A", 0, 31);
  myGLCD.print("No.2         A", 0, 44);
  myGLCD.print("No.3         A", 0, 57);
  myGLCD.setColor(VGA_AQUA);
  sprintf(buf, "No.1         Wh(%2dm)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 70);
  sprintf(buf, "No.2         Wh(%2dm)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 83);
  sprintf(buf, "No.3         Wh(%2dm)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 96);
  sprintf(buf, "Total        Wh(%2dm)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 109);
  myGLCD.setColor(VGA_FUCHSIA);
  sprintf(buf, "No.1       W(%2dm Pk)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 122);
  sprintf(buf, "No.2       W(%2dm Pk)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 135);
  sprintf(buf, "No.3       W(%2dm Pk)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 148);
  sprintf(buf, "Total      W(%2dm Pk)", SAVING_INTERVAL);
  myGLCD.print(buf, 0, 161);
  // 積算電力量(時間帯別)
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_YELLOW);
  myGLCD.print("INTEGRATED VALUES", 160, 18);
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("Since", 160, 31);
  myGLCD.print(" 7-10            kWh", 160, 44);
  myGLCD.print("10-17            kWh", 160, 57);
  myGLCD.print("17-23            kWh", 160, 70);
  myGLCD.print("23- 7            kWh", 160, 83);
  myGLCD.print("Total kWh", 160, 96);
  myGLCD.print("kWh/Day(Average)", 184, 161);
  // バーグラフ
  printBarGraphFrame();
  // 時刻（日本標準時）取得情報を表示
  printSyncTimeInfo();
  // ファンクションボタン
  printButton("RESET", "", 0, 209, 60, 239, STS_NORMAL);
  printButton("DATE", "TIME", 64, 209, 124, 239, STS_NORMAL);
  printButton("CONFIG", "", 128, 209, 188, 239, STS_NORMAL);
  printButton("SD", "CARD", 192, 209, 252, 239, STS_NORMAL);
  printButton("SCREEN", "OFF", 256, 209, 316, 239, STS_NORMAL);
  printSdStatus();
  printElapsedDays();
  printResetTime();
  scrOffTimer2 = TIMER_RESET;
  return screenMode;
}

// SmallFont専用
void printButton(char* s1, char* s2, int x1, int y1, int x2, int y2, int sts) {
  int x3; // s1の開始X座標
  int x4; // s2の開始X座標

  myGLCD.setBackColor(VGA_BLACK);
  if (sts == STS_NORMAL) {
    myGLCD.setColor(VGA_LIME);
  } else {
    myGLCD.setColor(VGA_FUCHSIA);
  }
  myGLCD.fillRoundRect(x1, y1, x2, y2);
  if (sts == STS_NORMAL) {
    myGLCD.setBackColor(VGA_LIME);
  } else {
    myGLCD.setBackColor(VGA_FUCHSIA);
  }
  myGLCD.setColor(VGA_BLACK);
  // SmallFontの横幅8ドット=文字5ドット+右にスペース3ドット
  // 右に2ドットずらすことでテキストの表示位置をボタンの中心に近づける
  // テキストの開始X座標 = (ボタンの中心位置) - (テキストの中心位置) + 2ドット
  x3 = (x1 + x2) / 2 - (strlen(s1) * 8) / 2 + 2;
  x4 = (x1 + x2) / 2 - (strlen(s2) * 8) / 2 + 2;
  if (s2[0] == '\0') {
    myGLCD.print(s1, x3, y1 + 10);
  } else {
    myGLCD.print(s1, x3, y1 + 5);
    myGLCD.print(s2, x4, y1 + 15);
  }
}

void printWindowFrame(int x1, int y1, int x2, int y2) {
  myGLCD.setColor(VGA_BLACK);
  myGLCD.fillRoundRect(x1, y1, x2, y2);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.drawRoundRect(x1, y1, x2, y2);
}

void popupWindow(void) {
  int btnId;

  btnId = getButtonNum();
  switch (btnId) {
    case BTN_RESET:
      reset();
      break;
    case BTN_ADJUST_TIME:
      adjustDateTime();
      break;
    case BTN_CONFIG:
      config();
      break;
    case BTN_SDCARD:
      sdCard();
      break;
    case BTN_SCREEN_OFF:
      beep(BEEP_HIGH);
      if (eepromClock == ON) {
        currScreenMode = SCREEN_CLOCK;
      } else {
        currScreenMode = SCREEN_OFF;
      }
      currScreenMode = printMainWindow(currScreenMode);
      break;
    case BTN_SCREEN_ON:
      beep(BEEP_HIGH);
      currScreenMode = printMainWindow(SCREEN_ON);
      break;
    default:
      break;
  }
  // メニューボタンを押して戻ってきたらSCREEN_OFF(またはSCREEN_CLOCK)にするタイマーをセットする
  if (btnId != BTN_NOT_PRESSED) {
    scrOffTimer1 = 0;
    scrOffTimerInterval.reset();
  }
}

void printDateTime(void) {
  char buf[BUFFER_LEN_HALF];

  if ((currScreenMode == SCREEN_OFF) || (currScreenMode == SCREEN_CLOCK)) {
    return;
  }
  Time t = getClockTime();
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%04d/%02d/%02d %s %02d:%02d", t.yr, t.mon, t.date, dayShortStr(t.day), t.hr, t.min);
  printStringExEx(prevDateTime, buf, 20, 0, 0, FONT_BIG_FONT, VGA_WHITE);
}

void printSyncTimeInfo(int screenMode) {
  char buf[BUFFER_LEN_HALF];
  int x, y;

  if (screenMode == SCREEN_ON) {
    x = 0;
    y = 174;
  }
  if (screenMode == SCREEN_CLOCK) {
    x = 0;
    y = 210;
  }
  myGLCD.setBackColor(VGA_BLACK);
  if (isSynchronized == true) {
    myGLCD.setColor(VGA_LIME);
  } else {
    myGLCD.setColor(VGA_RED);
  }
  myGLCD.setFont(SmallFont);
  sprintf(buf, "[T]%s", syncTimeInfo);
  myGLCD.print(buf, x, y);
}

void printBarGraphFrame(int screenMode) {
  int x, y;

  if (screenMode == SCREEN_ON) {
    x = 142;
    y = 187;
  }
  if (screenMode == SCREEN_CLOCK) {
    x = 142;
    y = 223;
  }
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.drawRect(x + 6, y, x + 127, y + 11);
  myGLCD.print("kW", x + 162, y);
}

void printBarGraph(int screenMode) {
  int segment;
  int level;
  char buf[BUFFER_LEN_HALF];
  int x, y;

  if (screenMode == SCREEN_ON) {
    x = 142;
    y = 187;
  }
  if (screenMode == SCREEN_CLOCK) {
    x = 142;
    y = 223;
  }
  // CONFIG画面で設定したkWがフルスケールになるようにlevelを計算する
  level = (int)(watt[0] * 15.0 / (float)eepromBarGraphFullscale / 1000.0);
  for (segment = 1; segment < 16; segment++) {
    if (level <= 15) { // オーバーレンジしていない
      // 描画不要な目盛りに対するスキップ処理
      if (prevLevel <= 15) { // 直前もオーバーレンジしていない
        // 増加
        if (prevLevel < level) {
          // 増加した部分以外の目盛り
          if ((segment <= prevLevel) || (segment > level)) {
            continue; // 何もしない
          }
        }
        // 減少
        if (prevLevel > level) {
          // 減少した部分以外の目盛りは
          if ((segment <= level) || (segment > prevLevel)) {
            continue; // 何もしない
          }
        }
        // 変化なし
        if (prevLevel == level) {
          continue; // 何もしない
        }
      }
      // 目盛りの描画処理
      if (segment <= level) { // 指示値以内
        // 1～3目盛りは緑色に
        if (segment <= 3) {
          myGLCD.setColor(VGA_LIME);
        }
        // 4～12目盛りは緑色に
        if ((segment >= 4) && (segment <= 12)) {
          myGLCD.setColor(VGA_YELLOW);
        }
        // 13～15目盛りはピンク色に
        if (segment >= 13) {
          myGLCD.setColor(VGA_FUCHSIA);
        }
      } else { // 指示値を超える部分は黒色に
        myGLCD.setColor(VGA_BLACK);
      }
    } else { // オーバーレンジ時
      if (prevLevel <= 15) { // 直前はオーバーレンジしていない
        myGLCD.setColor(VGA_RED); // 全ての目盛りを赤色に
      } else { // 直前もオーバーレンジしている
        continue;  // 何もしない
      }
    }
    myGLCD.fillRect(x + (segment * 8), y + 2, x + 5 + (segment * 8), y + 9);
  }
  prevLevel = level;
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  dtostrf(watt[0] / 1000.0, 4, 1, buf);
  myGLCD.print(buf, x + 130, y);
}

void printClock(int screenMode) {
  myGLCD.clrScr();
  printStringEx("Since", 0, 197, VGA_YELLOW, VGA_BLACK);
  printResetTime(screenMode);
  printSyncTimeInfo(screenMode);
  printSdStatus(screenMode);
  myGLCD.setFont(BigFont);
  printStringEx("kWh", 256, 197, VGA_YELLOW, VGA_BLACK);
  myGLCD.setFont(SmallFont);
  printBarGraphFrame(screenMode);
  initPrevClockValues();
  initPrevValues();
  printClockDateTime();
  myGLCD.setColor(VGA_WHITE);
  myGLCD.fillCircle(160, 105, 7);
  myGLCD.fillCircle(160, 155, 7);
}

void printClockDateTime(void) {
  char buf[11];

  Time t = getClockTime();
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%04d/%02d/%02d", t.yr, t.mon, t.date);
  printStringExEx(prevClockDate, buf, 10, 0, 0, FONT_GROTESK_BOLD_32X64, VGA_WHITE);
  strcpy(prevClockDate, buf);
  sprintf(buf, "%02d", t.hr);
  printStringExEx(prevClockHour, buf, 2, 0, 80, FONT_SEVEN_SEG_XXXL_NUM, VGA_WHITE);
  strcpy(prevClockHour, buf);
  sprintf(buf, "%02d", t.min);
  printStringExEx(prevClockMin, buf, 2, 192, 80, FONT_SEVEN_SEG_XXXL_NUM, VGA_WHITE);
  strcpy(prevClockMin, buf);
}

void printVoltAmpere(void) {
  char buf[BUFFER_LEN_HALF];

  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  if (hasBlackout == false) {
    dtostrf(vrms[1], 5, 1, buf);
    strcat(buf, "V  ");
    myGLCD.print(buf, 64, 18);
  }
  dtostrf(irms[1], 5, 1, buf);
  myGLCD.print(buf, 64, 31);
  dtostrf(irms[2], 5, 1, buf);
  myGLCD.print(buf, 64, 44);
  dtostrf(irms[3], 5, 1, buf);
  myGLCD.print(buf, 64, 57);
}

void printBlackout(void) {
  static boolean isBeep = false;

  //停電発生時1回だけ実行
  if ((hasBlackout == true) && (scrOffTimer2 == TIMER_RESET)) {
    scrOffTimer2 = 0;
    scrOffTimer1 = 0;
  }
  //復電時1回だけ実行
  if ((hasBlackout == false) && (scrOffTimer2 != TIMER_RESET)) {
    beep(BEEP_OFF);
    scrOffTimer2 = TIMER_RESET;
    if (currScreenMode == SCREEN_OFF) {
      currScreenMode = printMainWindow(SCREEN_ON);
    }
    //scrOffTimer1 = 0;
    scrOffTimerInterval.reset();
  }
  //停電中
  if (hasBlackout == true) {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_RED);
    myGLCD.print(BLACKOUT, 64, 18);
    if (scrOffTimer2 < 3) { //3分未満はビープ音を鳴らす
      beep(BEEP_800MS);
    } else { //3分になったらビープ音を止めてSCREENOFF
      scrOffTimer2 = TIMER_STOP;
      if (eepromScreenOffTime > 0) {
        currScreenMode = printMainWindow(SCREEN_OFF);
      }
    }
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.print(BLACKOUT, 64, 18);
  }
}

void printWattHour(void) {
  char buf[BUFFER_LEN_HALF];

  if ((currScreenMode == SCREEN_OFF) || (currScreenMode == SCREEN_CLOCK)) {
    return;
  }
  // Wh
  myGLCD.setColor(VGA_AQUA);
  myGLCD.setBackColor(VGA_BLACK);
  dtostrf(watt_hour1, WATTHOUR_LEN, 1, buf);
  printStringExEx(prevWattHour1, buf, WATTHOUR_LEN, 56, 70, FONT_SMALL_FONT);
  strcpy(prevWattHour1, buf);
  dtostrf(watt_hour2, WATTHOUR_LEN, 1, buf);
  printStringExEx(prevWattHour2, buf, WATTHOUR_LEN, 56, 83, FONT_SMALL_FONT);
  strcpy(prevWattHour2, buf);
  dtostrf(watt_hour3, WATTHOUR_LEN, 1, buf);
  printStringExEx(prevWattHour3, buf, WATTHOUR_LEN, 56, 96, FONT_SMALL_FONT);
  strcpy(prevWattHour3, buf);
  dtostrf(watt_hour, WATTHOUR_LEN, 1, buf);
  printStringExEx(prevWattHour, buf, WATTHOUR_LEN, 56, 109, FONT_SMALL_FONT);
  strcpy(prevWattHour, buf);
  //W(Peak)
  myGLCD.setColor(VGA_FUCHSIA);
  dtostrf(watt_max[1], 5, 0, buf);
  myGLCD.print(buf, 48, 122);
  dtostrf(watt_max[2], 5, 0, buf);
  myGLCD.print(buf, 48, 135);
  dtostrf(watt_max[3], 5, 0, buf);
  myGLCD.print(buf, 48, 148);
  dtostrf(watt_max[0], 5, 0, buf);
  myGLCD.print(buf, 48, 161);
}

void incrElapsedDays(void) {
  Time t = getClockTime();

  if (prevDate != t.date) {
    prevDate = t.date;
    elapsedDays++;
    // 累積使用量の自動リセット
    if (t.date == eepromAutoResetDate) {
      needsSaveIntegratedValues = true;
      //saveIntegratedValuesToSdCard(currScreenMode);
      //saveEventToSdCard("Integrated values reset (automatically)", currScreenMode);
    }
  }
}

void printElapsedDays(int screenMode) {
  char buf[BUFFER_LEN_HALF];

  if ((screenMode == SCREEN_OFF) || (screenMode == SCREEN_CLOCK)) {
    return;
  }
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%5dDay", elapsedDays);
  if (elapsedDays == 1) {
    strcat(buf, " ");
  } else {
    strcat(buf, "s");
  }
  printStringExEx(prevElapsedDays, buf, ELAPSEDDAYS_LEN, 248, 96, FONT_SMALL_FONT, VGA_YELLOW);
  strcpy(prevElapsedDays, buf);
}

void printWattHourTimeperiod(int screenMode) {
  char buf[BUFFER_LEN_HALF];

  if (screenMode == SCREEN_OFF) {
    return;
  }
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%05ld", ((long)(wattHourTimeperiod / 1000.0)) % 100000);
  if (screenMode == SCREEN_CLOCK) {
    printStringExEx(prevWattHourTimeperiod, buf, WATTHOURTIMEPERIOD_LEN, 176, 197, FONT_BIG_FONT);
    strcpy(prevWattHourTimeperiod, buf);
    return;
  } else {
    printStringExEx(prevWattHourTimeperiod, buf, WATTHOURTIMEPERIOD_LEN, 160, 109, FONT_SEVEN_SEG_NUM_FONT);
    strcpy(prevWattHourTimeperiod, buf);
  }
  printElapsedDays(screenMode);
  dtostrf(wattHourTimeperiod1 / 1000.0, WATTHOURTIMEPERIOD1_LEN, 1, buf);
  printStringExEx(prevWattHourTimeperiod1, buf, WATTHOURTIMEPERIOD1_LEN, 208, 44, FONT_SMALL_FONT);
  strcpy(prevWattHourTimeperiod1, buf);
  dtostrf(wattHourTimeperiod2 / 1000.0, WATTHOURTIMEPERIOD1_LEN, 1, buf);
  printStringExEx(prevWattHourTimeperiod2, buf, WATTHOURTIMEPERIOD1_LEN, 208, 57, FONT_SMALL_FONT);
  strcpy(prevWattHourTimeperiod2, buf);
  dtostrf(wattHourTimeperiod3 / 1000.0, WATTHOURTIMEPERIOD1_LEN, 1, buf);
  printStringExEx(prevWattHourTimeperiod3, buf, WATTHOURTIMEPERIOD1_LEN, 208, 70, FONT_SMALL_FONT);
  strcpy(prevWattHourTimeperiod3, buf);
  dtostrf(wattHourTimeperiod4 / 1000.0, WATTHOURTIMEPERIOD1_LEN, 1, buf);
  printStringExEx(prevWattHourTimeperiod4, buf, WATTHOURTIMEPERIOD1_LEN, 208, 83, FONT_SMALL_FONT);
  strcpy(prevWattHourTimeperiod4, buf);
  if (elapsedDays == 1) {
    sprintf(buf, "%3d", (int)(wattHourTimeperiod / 1000.0));
  } else {
    sprintf(buf, "%3d", (int)(((wattHourTimeperiod / 1000.0) / elapsedDays) + 0.5));
  }
  if (strcmp(buf, prevWattHourPerDay) != 0) {
    myGLCD.print(buf, 160, 161);
    strcpy(prevWattHourPerDay, buf);
  }
}

// SDカードの30分値の最終書込日時の表示(正常でない場合、エラー内容を表示)
void printSdStatus(int screenMode, int printMode) {
  int x, y;

  if (screenMode == SCREEN_OFF) {
    return;
  }
  if (printMode == PRINTMODE_LINE) {
    if (sdSts != SD_SUCCESS) {
      printEx(F("\n"));
      printEx(sdStsMsg[sdSts]);
    }
    return;
  }
  switch (screenMode) {
    case SCREEN_ON:
      x = 0;
      y = 187;
      break;
    case SCREEN_CLOCK:
      x = 0;
      y = 223;
      break;
  }
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_LIME);
  switch (sdSts) {
    case SD_SUCCESS:
    case SD_WRITING:
      break;
    default:
      myGLCD.setBackColor(VGA_RED);
      break;
  }
  myGLCD.setFont(SmallFont);
  myGLCD.print("SD", x, y);
  myGLCD.setColor(VGA_LIME);
  myGLCD.setBackColor(VGA_BLACK);
  // ステータス表示は16桁必要
  // xから3文字分("SD "の3文字*8dot=24dot)右の位置から表示するとバーグラフに重なるため
  // そこから4dot左に表示をずらすことで重ならないようにする
  x = x + 24 - 4;
  // 一旦ステータスを消去する
  myGLCD.print(sdStsMsg[SD_STS_MSG_BLANK], x, y);
  switch (sdSts) {
    case SD_SUCCESS:
      myGLCD.print(sdLastUpdatedTime, x, y);
      break;
    case SD_WRITING:
      myGLCD.print(sdStsMsg[sdSts], x, y);
      break;
    default:
      myGLCD.setColor(VGA_RED);
      myGLCD.print(sdStsMsg[sdSts], x, y);
      break;
  }
}

void printResetTime(int screenMode) {
  char buf[BUFFER_LEN_HALF];
  int x, y;

  if (screenMode == SCREEN_OFF) {
    return;
  }
  if (screenMode == SCREEN_ON) {
    x = 208;
    y = 31;
  }
  if (screenMode == SCREEN_CLOCK) {
    x = 48;
    y = 197;
  }
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(resetTime, x, y);
}

void initPrevValues(void) {
  prevDateTime[0] = '\0';
  prevWattHour1[0] = '\0';
  prevWattHour2[0] = '\0';
  prevWattHour3[0] = '\0';
  prevWattHour[0] = '\0';
  prevElapsedDays[0] = '\0';
  prevWattHourTimeperiod[0] = '\0';
  prevWattHourTimeperiod1[0] = '\0';
  prevWattHourTimeperiod2[0] = '\0';
  prevWattHourTimeperiod3[0] = '\0';
  prevWattHourTimeperiod4[0] = '\0';
  prevWattHourPerDay[0] = '\0';
  prevLevel = 0;
}

void initPrevClockValues(void) {
  prevClockDate[0] = '\0';
  prevClockHour[0] = '\0';
  prevClockMin[0] = '\0';
}

// ************************************************************************************
// RESETボタン押下
void reset(void) {
  int x, y;

  beep(BEEP_HIGH);
  printButton("RESET", "", 0, 209, 60, 239, STS_INVERSE);
  printWindowFrame(24, 62, 296, 160);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("RESET THE INTEGRATED VALUES?", 32, 90);
  myGLCD.setColor(VGA_LIME);
  printButton("SYSTEM", "RESET", 40, 124, 104, 152, STS_NORMAL);
  printButton("OK", "", 144, 124, 208, 152, STS_NORMAL);
  printButton("CANCEL", "", 216, 124, 280, 152, STS_NORMAL);
  while (1) {
    if (myTouch.dataAvailable()) {
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if ((y >= 124) && (y <= 152)) {
        // SYSTEM RESET
        if ((x >= 40) && (x <= 104)) {
          beep(BEEP_HIGH);
          printButton("SYSTEM", "RESET", 40, 124, 104, 152, STS_INVERSE);
          systemReset();
          break;
        }
        // OK
        if ((x >= 144) && (x <= 208)) {
          beep(BEEP_HIGH);
          printButton("OK", "", 144, 124, 208, 152, STS_INVERSE);
          saveIntegratedValuesToSdCard(SCREEN_OFF);
          saveEventToSdCard(F("Integrated values reset (manually)"), SCREEN_OFF);
          break;
        }
        // CANCEL
        if ((x >= 216) && (x <= 280)) {
          beep(BEEP_HIGH);
          break;
        }
      }
    }
  }
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

// SYSTEM RESETボタン押下
void systemReset(void) {
  int x, y;

  beep(BEEP_HIGH);
  printWindowFrame(24, 62, 296, 160);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("RESET THE SYSTEM?", 32, 76);
  myGLCD.print("IF YOU DO, THE INTEGRATED VALUES", 32, 90);
  myGLCD.print("WILL AlSO BE RESET!", 32, 104);
  myGLCD.setColor(VGA_LIME);
  printButton("OK", "", 144, 124, 208, 152, STS_NORMAL);
  printButton("CANCEL", "", 216, 124, 280, 152, STS_NORMAL);
  while (1) {
    if (myTouch.dataAvailable()) {
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if ((y >= 124) && (y <= 152)) {
        // OK
        if ((x >= 144) && (x <= 208)) {
          beep(BEEP_HIGH);
          printButton("OK", "", 144, 124, 208, 152, STS_INVERSE);
          saveIntegratedValuesToSdCard(SCREEN_OFF);
          saveEventToSdCard(F("Integrated values reset (manually)"), SCREEN_OFF);
          saveEventToSdCard(F("System reset"), SCREEN_OFF);
          resetSystem();
          //ここへは到達できない
          break;
        }
        // CANCEL
        if ((x >= 216) && (x <= 280)) {
          beep(BEEP_HIGH);
          break;
        }
      }
    }
  }
}

void resetIntegratedValues(void) {
  watt_hour1 = 0.0;
  watt_hour2 = 0.0;
  watt_hour3 = 0.0;
  watt_hour = 0.0;
  watt_sum1 = 0.0;
  watt_sum2 = 0.0;
  watt_sum3 = 0.0;
  watt_sum = 0.0;
  watt_samples = 0;
  wattHourTimeperiod1 = 0.0;
  wattHourTimeperiod2 = 0.0;
  wattHourTimeperiod3 = 0.0;
  wattHourTimeperiod4 = 0.0;
  wattHourTimeperiod = 0.0;
  irms_max[1] = 0.0;
  irms_max[2] = 0.0;
  irms_max[3] = 0.0;
  watt_max[1] = 0.0;
  watt_max[2] = 0.0;
  watt_max[3] = 0.0;
  watt_max[0] = 0.0;
}

void saveIntegratedValuesToSdCard(int screenMode) {
  File dataFile;
  char dataString[MAX_DATASTRING_LEN];
  char fileName[MAX_FILE_NAME_LEN];

  if (sdSts == SD_CARD_NOT_PRESENT) {
    return;
  }
  //リセット直前のデータを書き込み
  saveDataToSdCard(screenMode);
  strcpy(fileName, "RESET.CSV");
  // ファイルがない場合は、タイトル行を書き込み
  if (!SD.exists(fileName)) {
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println("yyyy/mm/dd,hh:mm:ss,7-10(kWh),10-17(kWh),17-23(kWh),23-7(kWh)");
    dataFile.close();
  }
  // ここからデータ行を書き込み
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    sdSts = SD_WRITING;
    if (screenMode == SCREEN_ON) {
      printStringEx("Writing    ", 0, 187, VGA_LIME, VGA_BLACK);
    }
    Time t = getClockTime();
    // データの文字数を増やす場合は、dataString変数のサイズを超えないようにする
    sprintf(dataString, "%04d/%02d/%02d,%02d:%02d:%02d,%d,%d,%d,%d",
            t.yr, t.mon, t.date, t.hr, t.min, t.sec,
            (int)(wattHourTimeperiod1 / 1000.0 + 0.5),
            (int)(wattHourTimeperiod2 / 1000.0 + 0.5),
            (int)(wattHourTimeperiod3 / 1000.0 + 0.5),
            (int)(wattHourTimeperiod4 / 1000.0 + 0.5));
    dataFile.println(dataString);
    dataFile.close();
  } else {
    sdSts = SD_OPEN_ERROR;
    printSdStatus(screenMode);
  }
  if (sdSts != SD_OPEN_ERROR) {
    sdSts = SD_SUCCESS;
  }
  Time t = getClockTime();
  sprintf(resetTime, "%02d/%02d/%02d %02d:%02d", t.yr % 100, t.mon, t.date, t.hr, t.min);
  printResetTime(screenMode);
  elapsedDays = 1;
  timer1_state  = STATE_STOP;
  resetIntegratedValues();
  //リセット直後のデータを書き込み
  saveDataToSdCard(screenMode);
  timer1_state  = STATE_SENSOR1;
  prevDate = t.date;
}

// ************************************************************************************
// DATE TIMEボタン押下
void adjustDateTime(void) {
  int x, y;
  int mode;
  // 各月の日数
  int dayset[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  char event[BUFFER_LEN_EVENT];

  beep(BEEP_HIGH);
  printButton("DATE", "TIME", 64, 209, 124, 239, STS_INVERSE);
  printWindowFrame(24, 20, 296, 188);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("Year", 32, 34);
  myGLCD.print("Month", 32, 48);
  myGLCD.print("Day", 32, 62);
  myGLCD.print("Day of the week", 32, 76);
  myGLCD.print("Hour", 32, 90);
  myGLCD.print("Minute", 32, 104);
  myGLCD.print("Second", 32, 118);
  Time t = getClockTime();
  t.sec = 0;
  printTempTime(t, TIME_YEAR, STS_INVERSE);
  printTempTime(t, TIME_MONTH, STS_NORMAL);
  printTempTime(t, TIME_DAY, STS_NORMAL);
  printTempTime(t, TIME_DAYNAME, STS_NORMAL);
  printTempTime(t, TIME_HOUR, STS_NORMAL);
  printTempTime(t, TIME_MINUTE, STS_NORMAL);
  printTempTime(t, TIME_SECOND, STS_NORMAL);
  myGLCD.setColor(VGA_LIME);
  printButton("UP", "", 224, 40, 264, 68, STS_NORMAL);
  printButton("-", "", 200, 75, 240, 103, STS_NORMAL);
  printButton("+", "", 248, 75, 288, 103, STS_NORMAL);
  printButton("DOWN", "", 224, 110, 264, 138, STS_NORMAL);
  printButton("SYNC", "", 48, 152, 112, 180, STS_NORMAL);
  printButton("OK", "", 152, 152, 216, 180, STS_NORMAL);
  printButton("CANCEL", "", 224, 152, 288, 180, STS_NORMAL);
  mode = TIME_YEAR;
  while (1) {
    if (myTouch.dataAvailable()) {
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      // 閏年なら2月は29日まで
      if (t.yr % 4 == 0 && t.yr % 100 != 0 || t.yr % 400 == 0) {
        dayset[2] = 29;
      } else {
        dayset[2] = 28;
      }
      if ((x >= 224) && (x <= 264)) {
        // DOWN
        if ((y >= 110) && (y <= 138)) {
          beep(BEEP_HIGH);
          printButton("DOWN", "", 224, 110, 264, 138, STS_INVERSE);
          printTempTime(t, mode, STS_NORMAL);
          mode = changeValue(mode, 1, 0, 6);
          printTempTime(t, mode, STS_INVERSE);
          printButton("DOWN", "", 224, 110, 264, 138, STS_NORMAL);
          continue;
        }
        // UP
        if ((y >= 40) && (y <= 68)) {
          beep(BEEP_HIGH);
          printButton("UP", "", 224, 40, 264, 68, STS_INVERSE);
          printTempTime(t, mode, STS_NORMAL);
          mode = changeValue(mode, -1, 0, 6);
          printTempTime(t, mode, STS_INVERSE);
          printButton("UP", "", 224, 40, 264, 68, STS_NORMAL);
          continue;

        }
      }
      if ((y >= 75) && (y <= 103)) {
        // +
        if ((x >= 248) && (x <= 288)) {
          beep(BEEP_HIGH);
          printButton("+", "", 248, 75, 288, 103, STS_INVERSE);
          switch (mode) {
            case TIME_YEAR:
              t.yr = changeValue(t.yr, 1, 2000, 2099);
              break;
            case TIME_MONTH:
              t.mon = changeValue(t.mon, 1, 1, 12);
              break;
            case TIME_DAY:
              t.date = changeValue(t.date, 1, 1, dayset[t.mon]);
              break;
            case TIME_DAYNAME:
              t.day = (Time::Day)changeValue(t.day, 1, 1, 7);
              break;
            case TIME_HOUR:
              t.hr = changeValue(t.hr, 1, 0, 23);
              break;
            case TIME_MINUTE:
              t.min = changeValue(t.min, 1, 0, 59);
              break;
            case TIME_SECOND:
              t.sec = changeValue(t.sec, 1, 0, 59);
              break;
          }
          printTempTime(t, mode, STS_INVERSE);
          printButton("+", "", 248, 75, 288, 103, STS_NORMAL);
          continue;
        }
      }
      // -
      if ((x >= 200) && (x <= 240)) {
        beep(BEEP_HIGH);
        printButton("-", "", 200, 75, 240, 103, STS_INVERSE);
        switch (mode) {
          case TIME_YEAR:
            t.yr = changeValue(t.yr, -1, 2000, 2099);
            break;
          case TIME_MONTH:
            t.mon = changeValue(t.mon, -1, 1, 12);
            break;
          case TIME_DAY:
            t.date = changeValue(t.date, -1, 1, dayset[t.mon]);
            break;
          case TIME_DAYNAME:
            t.day = (Time::Day)changeValue(t.day, -1, 1, 7);
            break;
          case TIME_HOUR:
            t.hr = changeValue(t.hr, -1, 0, 23);
            break;
          case TIME_MINUTE:
            t.min = changeValue(t.min, -1, 0, 59);
            break;
          case TIME_SECOND:
            t.sec = changeValue(t.sec, -1, 0, 59);
            break;
        }
        printTempTime(t, mode, STS_INVERSE);
        printButton("-", "", 200, 75, 240, 103, STS_NORMAL);
        continue;
      }
      if ((y >= 152) && (y <= 180)) {
        // SYNC
        if ((x >= 48) && (x <= 112)) {
          if (existsFlashAir == true) {
            beep(BEEP_HIGH);
            printButton("SYNC", "", 48, 152, 112, 180, STS_INVERSE);
            currScreenMode = SCREEN_SETUP;
            if (needsResetFlashAir == true) {
              resetFlashAir();
            }
            isSynchronized = syncTime();
            if (isSynchronized == true) {
              sprintf(event, "[%04lX][%s]Time synchronized (manually)", nextSequenceId - 1, httpCommandArg1);
              saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
            } else {
              sprintf(event, "[%04lX][%s]Time synchronization failed (manually)", nextSequenceId - 1, httpCommandArg1);
              saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
            }
            delay(3000);
          }
          break;
        }
        // OK
        if ((x >= 152) && (x <= 216)) {
          beep(BEEP_HIGH);
          printButton("OK", "", 152, 152, 216, 180, STS_INVERSE);
          setClockTime(t);
          break;
        }
        // CANCEL
        if ((x >= 224) && (x <= 288)) {
          beep(BEEP_HIGH);
          break;
        }
      }
    }
  }
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

void printTempTime(Time tt, int m, int status) {
  char buf[BUFFER_LEN_HALF];

  if (status == STS_NORMAL) {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);
  } else {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_WHITE);
  }
  switch (m) {
    case TIME_YEAR:
      sprintf(buf, "%04d", tt.yr);
      myGLCD.print(buf, 88, 34);
      break;
    case TIME_MONTH:
      sprintf(buf, "%02d", tt.mon);
      myGLCD.print(buf, 88, 48);
      break;
    case TIME_DAY:
      sprintf(buf, "%02d", tt.date);
      myGLCD.print(buf, 88, 62);
      break;
    case TIME_DAYNAME:
      sprintf(buf, "%3s", dayShortStr(tt.day));
      myGLCD.print(buf, 160, 76);
      break;
    case TIME_HOUR:
      sprintf(buf, "%02d", tt.hr);
      myGLCD.print(buf, 88, 90);
      break;
    case TIME_MINUTE:
      sprintf(buf, "%02d", tt.min);
      myGLCD.print(buf, 88, 104);
      break;
    case TIME_SECOND:
      sprintf(buf, "%02d", tt.sec);
      myGLCD.print(buf, 88, 118);
      break;
  }
}

int changeValue(int value , int step, int lowerLimit, int upperLimit) {
  if (step > 0) {
    if (value < upperLimit) {
      value = value + step;
      if (value > upperLimit) {
        value = upperLimit;
      }
    } else {
      value = lowerLimit;
    }
  }
  if (step < 0) {
    if (value > lowerLimit) {
      value = value + step;
      if (value < lowerLimit) {
        value = lowerLimit;
      }
    } else {
      value = upperLimit;
    }
  }
  return value;
}

// ************************************************************************************
// CONFIGボタン押下
void config(void) {
  int x, y;
  int mode;
  char buf[BUFFER_LEN_HALF];
  boolean isContinue;
  byte tempValue[EEPROM_MODE_MAX + 1];

  beep(BEEP_HIGH);
  printButton("CONFIG", "", 128, 209, 188, 239, STS_INVERSE);
  printWindowFrame(24, 20, 296, 230);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(VER, 32, 26);
  if (existsFlashAir == true) {
    sprintf(buf, "IP %15s", flashAirIP);
    myGLCD.print(buf, 32, 40);
  }
  myGLCD.print("Screen off time", 32, 54);
  myGLCD.print("Auto reset date", 32, 68);
  myGLCD.print("Voltage  100V", 32, 82);
  myGLCD.print("Sensor1", 32, 96);
  myGLCD.print("Sensor2", 32, 110);
  myGLCD.print("Sensor3", 32, 124);
  myGLCD.print("Sound", 32, 138);
  myGLCD.print("Clock disp", 32, 152);
  //      ここまで表示可能→| OFF 12:34 
  myGLCD.print("Clock sync       :", 32, 166);
  myGLCD.print("Bar graph F.S.", 32, 180);
  tempValue[EEPROM_SCREEN_OFF_TIME] = eepromScreenOffTime;
  tempValue[EEPROM_AUTO_RESET_DATE] = eepromAutoResetDate;
  tempValue[EEPROM_CT1_VOLTAGE] = eepromCtVoltage[1];
  tempValue[EEPROM_CT2_VOLTAGE] = eepromCtVoltage[2];
  tempValue[EEPROM_CT3_VOLTAGE] = eepromCtVoltage[3];
  tempValue[EEPROM_VT_PERCENTAGE] = eepromVtPercentage;
  tempValue[EEPROM_CT1_PERCENTAGE] = eepromCtPercentage[1];
  tempValue[EEPROM_CT2_PERCENTAGE] = eepromCtPercentage[2];
  tempValue[EEPROM_CT3_PERCENTAGE] = eepromCtPercentage[3];
  tempValue[EEPROM_SOUND] = eepromSound;
  tempValue[EEPROM_CLOCK] = eepromClock;
  tempValue[EEPROM_AUTO_SYNC_TIME] = eepromAutoTimeSync;
  tempValue[EEPROM_TIME_SYNC_HOUR] = eepromTimeSyncHour;
  tempValue[EEPROM_TIME_SYNC_MIN] = eepromTimeSyncMin;
  tempValue[EEPROM_BAR_GRAPH_FULLSCALE] = eepromBarGraphFullscale;
  printTempValues(tempValue);
  printButton("UP", "", 224, 40, 264, 68, STS_NORMAL);
  printButton("-", "", 200, 75, 240, 103, STS_NORMAL);
  printButton("+", "", 248, 75, 288, 103, STS_NORMAL);
  printButton("DOWN", "", 224, 110, 264, 138, STS_NORMAL);
  printButton("LOAD", "DEFAULT", 92, 194, 152, 222, STS_NORMAL);
  printButton("OK", "", 160, 194, 220, 222, STS_NORMAL);
  printButton("CANCEL", "", 228, 194, 288, 222, STS_NORMAL);
  while (1) {
    isContinue = false;
    mode = EEPROM_SCREEN_OFF_TIME;
    while (1) {
      if (myTouch.dataAvailable()) {
        beep(BEEP_LOW);
        myTouch.read();
        x = myTouch.getX();
        y = myTouch.getY();
        if ((x >= 224) && (x <= 264)) {
          // DOWN
          if ((y >= 110) && (y <= 138)) {
            beep(BEEP_HIGH);
            printButton("DOWN", "", 224, 110, 264, 138, STS_INVERSE);
            printTempValue(tempValue, mode, STS_NORMAL);
            mode = changeValue(mode, 1, 0, EEPROM_MODE_MAX);
            printTempValue(tempValue, mode, STS_INVERSE);
            printButton("DOWN", "", 224, 110, 264, 138, STS_NORMAL);
            continue;
          }
          // UP
          if ((y >= 40) && (y <= 68)) {
            beep(BEEP_HIGH);
            printButton("UP", "", 224, 40, 264, 68, STS_INVERSE);
            printTempValue(tempValue, mode, STS_NORMAL);
            mode = changeValue(mode, -1, 0, EEPROM_MODE_MAX);
            printTempValue(tempValue, mode, STS_INVERSE);
            printButton("UP", "", 224, 40, 264, 68, STS_NORMAL);
            continue;
          }
        }
        if ((y >= 75) && (y <= 103)) {
          // +
          if ((x >= 248) && (x <= 288)) {
            beep(BEEP_HIGH);
            printButton("+", "", 248, 75, 288, 103, STS_INVERSE);
            switch (mode) {
              case EEPROM_SCREEN_OFF_TIME:
                tempValue[mode] = changeValue(tempValue[mode], 1, 0, 99);
                break;
              case EEPROM_AUTO_RESET_DATE:
                tempValue[mode] = changeValue(tempValue[mode], 1, 0, 28);
                break;
              case EEPROM_CT1_VOLTAGE:
              case EEPROM_CT2_VOLTAGE:
              case EEPROM_CT3_VOLTAGE:
                tempValue[mode] = changeValue(tempValue[mode], 100, 0, 200);
                break;
              case EEPROM_VT_PERCENTAGE:
              case EEPROM_CT1_PERCENTAGE:
              case EEPROM_CT2_PERCENTAGE:
              case EEPROM_CT3_PERCENTAGE:
                tempValue[mode] = changeValue(tempValue[mode], 1, 1, 255);
                break;
              case EEPROM_SOUND:
              case EEPROM_CLOCK:
              case EEPROM_AUTO_SYNC_TIME:
                tempValue[mode] = changeValue(tempValue[mode], 1, OFF, ON);
                break;
              case EEPROM_TIME_SYNC_HOUR:
                tempValue[mode] = changeValue(tempValue[mode], 1, 0, 23);
                break;
              case EEPROM_TIME_SYNC_MIN:
                tempValue[mode] = changeValue(tempValue[mode], 1, 0, 59);
                break;
              case EEPROM_BAR_GRAPH_FULLSCALE:
                tempValue[mode] = changeValue(tempValue[mode], 1, 1, 49);
                break;
            }
            printTempValue(tempValue, mode, STS_INVERSE);
            printButton("+", "", 248, 75, 288, 103, STS_NORMAL);
            continue;
          }
          // -
          if ((x >= 200) && (x <= 240)) {
            beep(BEEP_HIGH);
            printButton("-", "", 200, 75, 240, 103, STS_INVERSE);
            switch (mode) {
              case EEPROM_SCREEN_OFF_TIME:
                tempValue[mode] = changeValue(tempValue[mode], -1, 0, 99);
                break;
              case EEPROM_AUTO_RESET_DATE:
                tempValue[mode] = changeValue(tempValue[mode], -1, 0, 28);
                break;
              case EEPROM_CT1_VOLTAGE:
              case EEPROM_CT2_VOLTAGE:
              case EEPROM_CT3_VOLTAGE:
                tempValue[mode] = changeValue(tempValue[mode], -100, 0, 200);
                break;
              case EEPROM_VT_PERCENTAGE:
              case EEPROM_CT1_PERCENTAGE:
              case EEPROM_CT2_PERCENTAGE:
              case EEPROM_CT3_PERCENTAGE:
                tempValue[mode] = changeValue(tempValue[mode], -1, 1, 255);
                break;
              case EEPROM_SOUND:
              case EEPROM_CLOCK:
              case EEPROM_AUTO_SYNC_TIME:
                tempValue[mode] = changeValue(tempValue[mode], -1, OFF, ON);
                break;
              case EEPROM_TIME_SYNC_HOUR:
                tempValue[mode] = changeValue(tempValue[mode], -1, 0, 23);
                break;
              case EEPROM_TIME_SYNC_MIN:
                tempValue[mode] = changeValue(tempValue[mode], -1, 0, 59);
                break;
              case EEPROM_BAR_GRAPH_FULLSCALE:
                tempValue[mode] = changeValue(tempValue[mode], -1, 1, 49);
                break;
            }
            printTempValue(tempValue, mode, STS_INVERSE);
            printButton("-", "", 200, 75, 240, 103, STS_NORMAL);
            continue;
          }
        }
        if ((y >= 194) && (y <= 222)) {
          // LOAD DEFAULT
          if ((x >= 92) && (x <= 152)) {
            beep(BEEP_HIGH);
            printButton("LOAD", "DEFAULT",  92, 194, 152, 222, STS_INVERSE);
            tempValue[EEPROM_SCREEN_OFF_TIME] = DEFAULT_SCREEN_OFF_TIME;
            tempValue[EEPROM_AUTO_RESET_DATE] = DEFAULT_AUTO_RESET_DATE;
            tempValue[EEPROM_CT1_VOLTAGE] = DEFAULT_CT1_VOLTAGE;
            tempValue[EEPROM_CT2_VOLTAGE] = DEFAULT_CT2_VOLTAGE;
            tempValue[EEPROM_CT3_VOLTAGE] = DEFAULT_CT3_VOLTAGE;
            tempValue[EEPROM_VT_PERCENTAGE] = DEFAULT_VT_PERCENTAGE;
            tempValue[EEPROM_CT1_PERCENTAGE] = DEFAULT_CT1_PERCENTAGE;
            tempValue[EEPROM_CT2_PERCENTAGE] = DEFAULT_CT2_PERCENTAGE;
            tempValue[EEPROM_CT3_PERCENTAGE] = DEFAULT_CT3_PERCENTAGE;
            tempValue[EEPROM_SOUND] = DEFAULT_SOUND;
            tempValue[EEPROM_CLOCK] = DEFAULT_CLOCK;
            tempValue[EEPROM_AUTO_SYNC_TIME] = DEFAULT_AUTO_TIME_SYNC;
            tempValue[EEPROM_TIME_SYNC_HOUR] = DEFAULT_TIME_SYNC_HOUR;
            tempValue[EEPROM_TIME_SYNC_MIN] = DEFAULT_TIME_SYNC_MIN;
            tempValue[EEPROM_BAR_GRAPH_FULLSCALE] = DEFAULT_BAR_GRAPH_FULLSCALE;
            printTempValues(tempValue);
            printButton("LOAD", "DEFAULT",  92, 194, 152, 222,  STS_NORMAL);
            isContinue = true;
            break;
          }
          // OK
          if ((x >= 160) && (x <= 220)) {
            beep(BEEP_HIGH);
            printButton("OK", "", 160, 194, 220, 222, STS_INVERSE);
            eepromScreenOffTime = tempValue[EEPROM_SCREEN_OFF_TIME];
            eepromAutoResetDate = tempValue[EEPROM_AUTO_RESET_DATE];
            eepromCtVoltage[1] = tempValue[EEPROM_CT1_VOLTAGE];
            eepromCtVoltage[2] = tempValue[EEPROM_CT2_VOLTAGE];
            eepromCtVoltage[3] = tempValue[EEPROM_CT3_VOLTAGE];
            eepromVtPercentage = tempValue[EEPROM_VT_PERCENTAGE];
            eepromCtPercentage[1] = tempValue[EEPROM_CT1_PERCENTAGE];
            eepromCtPercentage[2] = tempValue[EEPROM_CT2_PERCENTAGE];
            eepromCtPercentage[3] = tempValue[EEPROM_CT3_PERCENTAGE];
            eepromSound = tempValue[EEPROM_SOUND];
            eepromClock = tempValue[EEPROM_CLOCK];
            eepromAutoTimeSync = tempValue[EEPROM_AUTO_SYNC_TIME];
            eepromTimeSyncHour = tempValue[EEPROM_TIME_SYNC_HOUR];
            eepromTimeSyncMin = tempValue[EEPROM_TIME_SYNC_MIN];
            eepromBarGraphFullscale = tempValue[EEPROM_BAR_GRAPH_FULLSCALE];
            EEPROM.write(EEPROM_SCREEN_OFF_TIME, eepromScreenOffTime);
            EEPROM.write(EEPROM_AUTO_RESET_DATE, eepromAutoResetDate);
            EEPROM.write(EEPROM_CT1_VOLTAGE, eepromCtVoltage[1]);
            EEPROM.write(EEPROM_CT2_VOLTAGE, eepromCtVoltage[2]);
            EEPROM.write(EEPROM_CT3_VOLTAGE, eepromCtVoltage[3]);
            EEPROM.write(EEPROM_VT_PERCENTAGE, eepromVtPercentage - 1);
            EEPROM.write(EEPROM_CT1_PERCENTAGE, eepromCtPercentage[1] - 1);
            EEPROM.write(EEPROM_CT2_PERCENTAGE, eepromCtPercentage[2] - 1);
            EEPROM.write(EEPROM_CT3_PERCENTAGE, eepromCtPercentage[3] - 1);
            EEPROM.write(EEPROM_SOUND, eepromSound);
            EEPROM.write(EEPROM_CLOCK, eepromClock);
            EEPROM.write(EEPROM_AUTO_SYNC_TIME, eepromAutoTimeSync);
            EEPROM.write(EEPROM_TIME_SYNC_HOUR, eepromTimeSyncHour);
            EEPROM.write(EEPROM_TIME_SYNC_MIN, eepromTimeSyncMin);
            EEPROM.write(EEPROM_BAR_GRAPH_FULLSCALE, eepromBarGraphFullscale);
            break;
          }
          // CANCEL
          if ((x >= 228) && (x <= 288)) {
            beep(BEEP_HIGH);
            break;
          }
        }
      }
    }
    if (isContinue == true) {
      continue;
    } else {
      break;
    }
  }
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

void printTempValue(byte * v, int m, int sts) {
  char buf[BUFFER_LEN_HALF];

  if (sts == STS_NORMAL) {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);
  } else {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_WHITE);
  }
  switch (m) {
    case EEPROM_SCREEN_OFF_TIME:
      sprintf(buf, "%02d", v[m]);
      myGLCD.print(buf, 160, 54);
      break;
    case EEPROM_AUTO_RESET_DATE:
      sprintf(buf, "%02d", v[m]);
      myGLCD.print(buf, 160, 68);
      break;
    case EEPROM_CT1_VOLTAGE:
      sprintf(buf, "%3dV", v[m]);
      myGLCD.print(buf, 104, 96);
      break;
    case EEPROM_CT2_VOLTAGE:
      sprintf(buf, "%3dV", v[m]);
      myGLCD.print(buf, 104, 110);
      break;
    case EEPROM_CT3_VOLTAGE:
      sprintf(buf, "%3dV", v[m]);
      myGLCD.print(buf, 104, 124);
      break;
    case EEPROM_VT_PERCENTAGE:
      sprintf(buf, "%3d%%", v[m]);
      myGLCD.print(buf, 144, 82);
      break;
    case EEPROM_CT1_PERCENTAGE:
      sprintf(buf, "%3d%%", v[m]);
      myGLCD.print(buf, 144, 96);
      break;
    case EEPROM_CT2_PERCENTAGE:
      sprintf(buf, "%3d%%", v[m]);
      myGLCD.print(buf, 144, 110);
      break;
    case EEPROM_CT3_PERCENTAGE:
      sprintf(buf, "%3d%%", v[m]);
      myGLCD.print(buf, 144, 124);
      break;
    case EEPROM_SOUND:
      sprintf(buf, "%3s", (v[m] == ON) ? "ON" : "OFF");
      myGLCD.print(buf, 152, 138);
      break;
    case EEPROM_CLOCK:
      sprintf(buf, "%3s", (v[m] == ON) ? "ON" : "OFF");
      myGLCD.print(buf, 152, 152);
      break;
    case EEPROM_AUTO_SYNC_TIME:
      sprintf(buf, "%3s", (v[m] == ON) ? "ON" : "OFF");
      myGLCD.print(buf, 120, 166);
      break;
    case EEPROM_TIME_SYNC_HOUR:
      sprintf(buf, "%02d", v[m]);
      myGLCD.print(buf, 152, 166);
      break;
    case EEPROM_TIME_SYNC_MIN:
      sprintf(buf, "%02d", v[m]);
      myGLCD.print(buf, 176, 166);
      break;
    case EEPROM_BAR_GRAPH_FULLSCALE:
      sprintf(buf, "%2dkW", v[m]);
      myGLCD.print(buf, 160, 180);
      break;
  }
}

void printTempValues(byte * v) {
  printTempValue(v, EEPROM_SCREEN_OFF_TIME, STS_INVERSE);
  printTempValue(v, EEPROM_AUTO_RESET_DATE, STS_NORMAL);
  printTempValue(v, EEPROM_CT1_VOLTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_CT2_VOLTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_CT3_VOLTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_VT_PERCENTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_CT1_PERCENTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_CT2_PERCENTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_CT3_PERCENTAGE, STS_NORMAL);
  printTempValue(v, EEPROM_SOUND, STS_NORMAL);
  printTempValue(v, EEPROM_CLOCK, STS_NORMAL);
  printTempValue(v, EEPROM_AUTO_SYNC_TIME, STS_NORMAL);
  printTempValue(v, EEPROM_TIME_SYNC_HOUR, STS_NORMAL);
  printTempValue(v, EEPROM_TIME_SYNC_MIN, STS_NORMAL);
  printTempValue(v, EEPROM_BAR_GRAPH_FULLSCALE, STS_NORMAL);
}

// ************************************************************************************
// SD CARDボタン押下
void sdCard(void) {
  int x, y;
  int fileNum;
  int currPage, nextPage;

  beep(BEEP_HIGH);
  printButton("SD", "CARD", 192, 209, 252, 239, STS_INVERSE);
  fileNum = 0;
  printFileListWindow(fileNum);
  while (1) {
    if (myTouch.dataAvailable()) {
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if ((x >= 232) && (x <= 288)) {
        // PAGE UP
        if ((y >= 27) && (y <= 53)) {
          if (numberOfFiles <= 10) {
            beep(BEEP_LOW);
            continue;
          }
          beep(BEEP_HIGH);
          printButton("PAGE", "UP", 232, 27, 288, 53, STS_INVERSE);
          currPage = (int)(fileNum / 10) + 1;
          nextPage = (int)(numberOfFiles / 10) + 1;
          if (currPage > 1) {
            fileNum = (currPage - 2) * 10;
          } else {
            fileNum = (nextPage - 1) * 10;
          }
          printFileList(fileNum);
          printButton("PAGE", "UP", 232, 27, 288, 53, STS_NORMAL);
          continue;
        }
        // CURSOR UP
        if ((y >= 57) && (y <= 83)) {
          if (numberOfFiles < 1) {
            beep(BEEP_LOW);
            continue;
          }
          beep(BEEP_HIGH);
          printButton("CURSOR", "UP", 232, 57, 288, 83, STS_INVERSE);
          if (fileNum > 0) {
            printFile(fileNum, fileNum - 1);
            fileNum--;
          } else {
            printFile(fileNum, numberOfFiles - 1);
            fileNum = numberOfFiles - 1;
          }
          printButton("CURSOR", "UP", 232, 57, 288, 83, STS_NORMAL);
          continue;
        }
        // CURSOR DOWN
        if ((y >= 87) && (y <= 113)) {
          if (numberOfFiles < 1) {
            beep(BEEP_LOW);
            continue;
          }
          beep(BEEP_HIGH);
          printButton("CURSOR", "DOWN", 232, 87, 288, 113, STS_INVERSE);
          if ((fileNum + 1) < numberOfFiles) {
            printFile(fileNum, fileNum + 1);
            fileNum++;
          } else {
            printFile(fileNum, 0);
            fileNum = 0;
          }
          printButton("CURSOR", "DOWN", 232, 87, 288, 113, STS_NORMAL);
          continue;
        }
        // PAGE DOWN
        if ((y >= 117) && (y <= 143)) {
          if (numberOfFiles <= 10) {
            beep(BEEP_LOW);
            continue;
          }
          beep(BEEP_HIGH);
          printButton("PAGE", "DOWN", 232, 117, 288, 143, STS_INVERSE);
          currPage = (int)(fileNum / 10) + 1;
          nextPage = (int)(numberOfFiles / 10) + 1;
          if (currPage < nextPage) {
            fileNum = currPage * 10;
          } else {
            fileNum = 0;
          }
          printFileList(fileNum);
          printButton("PAGE", "DOWN", 232, 117, 288, 143, STS_NORMAL);
          continue;
        }
      }
      if ((y >= 152) && (y <= 208)) {
        // DELETE ALL
        if ((x >= 80) && (x <= 144)) {
          beep(BEEP_HIGH);
          printButton("DELETE", "ALL", 80, 180, 144, 208, STS_INVERSE);
          deleteFile(FILENUM_DELETE_ALL);
          fileNum = 0;
          printFileListWindow(fileNum);
          continue;
        }
        // DELETE
        if ((x >= 152) && (x <= 216)) {
          if (fileList[fileNum]->attribute == '+') {
            beep(BEEP_LOW);
            continue;
          }
          beep(BEEP_HIGH);
          myGLCD.setColor(VGA_FUCHSIA);
          printButton("DELETE", "", 152, 180, 216, 208, STS_INVERSE);
          if (deleteFile(fileNum) == true) {
            if (fileNum > 0) {
              fileNum--;
            }
          }
          printFileListWindow(fileNum);
          continue;
        }
        // CANCEL
        if ((x >= 224) && (x <= 288)) {
          beep(BEEP_HIGH);
          break;
        }
      }
    }
  }
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

void getNumberOfFiles(File dir) {
  File entry;

  while (true) {
    entry =  dir.openNextFile();
    if (! entry) {
      return;
    }
    numberOfFiles = numberOfFiles + 1;
    getNumberOfFiles(entry);
    entry.close();
  }
}

void getFileNameAndSize(File dir, int level) {
  File entry;
  char buf[BUFFER_LEN_FULL];

  while (true) {
    entry =  dir.openNextFile();
    if (! entry) {
      return;
    }
    fileList[numberOfFiles] = new FILELIST;
    fileList[numberOfFiles]->num = numberOfFiles + 1;
    strcpy(fileList[numberOfFiles]->name, entry.name());
    if (level == 0) {
      fileList[numberOfFiles]->attribute = ' ';
    } else {
      fileList[numberOfFiles]->attribute = '+';
    }
    if (entry.isDirectory()) {
      strcpy(fileList[numberOfFiles]->size, "  <dir>");
      fileList[numberOfFiles]->attribute = '+';
    } else {
      sprintf(fileList[numberOfFiles]->size, "%8u", entry.size()); // バイト単位
      fileUsage = fileUsage + entry.size();
    }
    numberOfFiles = numberOfFiles + 1;
    getFileNameAndSize(entry, level + 1);
    entry.close();
  }
}

int getFileList(void) {
  File root;
  int i;

  if (!SD.begin(PIN_SDCARD_CS)) {
    return SD_CARD_NOT_PRESENT;
  }
  root = SD.open("/");
  if (!root) {
    return  SD_OPEN_ERROR;
  }
  for (i = 0; i < numberOfFiles; i++) {
    delete fileList[i];
  }
  numberOfFiles = 0;
  root.rewindDirectory(); // openNextFile() が参照するすファイル位置をディレクトリの最初のファイルに戻す
  getNumberOfFiles(root);
  fileList = new FILELIST*[numberOfFiles];
  fileUsage = 0;
  numberOfFiles = 0;
  root.rewindDirectory(); // openNextFile() が参照するすファイル位置をディレクトリの最初のファイルに戻す
  getFileNameAndSize(root, 0);
  root.close();
  return SD_SUCCESS;
}

void printFileList(int fileNum) {
  int i, j;
  int page;
  int row;
  uint32_t volumesize;
  char buf[BUFFER_LEN_FULL];
  SdVolume volume; // 総容量取得に使用

  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  if (sdSts == SD_CARD_NOT_PRESENT) {
    myGLCD.print(sdStsMsg[sdSts], 24, 34);
    return;
  }
  if (sdSts == SD_OPEN_ERROR) {
    myGLCD.print(sdStsMsg[sdSts], 24, 34);
    return;
  }
  if (numberOfFiles == 0) {
    myGLCD.print(sdStsMsg[SD_NO_FILES], 24, 34);
    return;
  }
  page = (int)(fileNum / 10) + 1;
  row = fileNum % 10;
  i = 0;
  while (i < 10) {
    j = (page - 1) * 10 + i;
    if (j < numberOfFiles) {
      sprintf(buf, "%2d%c%-13s%8s", fileList[j]->num, fileList[j]->attribute, fileList[j]->name, fileList[j]->size);
    } else {
      strcpy(buf, "                        ");
    }
    myGLCD.print(buf, 24, 34 + i * 14);
    i++;
  }
  // 現在ページ / 総ページ
  myGLCD.print("Page", 24, 174);
  sprintf(buf, "%02d/%02d", page, (int)(numberOfFiles / 10) + 1);
  myGLCD.print(buf, 24, 188);
  // SD.card.cardSize() return The number of 512 byte data blocks in the card or zero if an error occurs.
  // 16GBは正しく動作するも、32GBは正しく動作しない
  //volumesize = SD.card.cardSize() / 2LU; // 2(*512/1024)で割ることでキロバイト単位にする
  volumesize = 0;
  if (volume.init(card)) {
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  }
  if (volumesize != 0) {
    sprintf(buf, "%10lu", volumesize - fileUsage / 1024LU); // 総容量-使用容量=空き容量(キロバイト単位)
  }
  myGLCD.print(buf, 216, 146);
  myGLCD.print(F("   kB Free"), 216, 160);

  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  sprintf(buf, "%2d%c%-13s%8s",
          fileList[fileNum]->num, fileList[fileNum]->attribute, fileList[fileNum]->name, fileList[fileNum]->size);
  myGLCD.print(buf, 24, 34 + row * 14);
}

void printFile(int currFileNum, int nextFileNum) {
  int currPage, nextPage;
  int currRow, nextRow;
  char buf[BUFFER_LEN_FULL];

  currPage = (int)(currFileNum / 10) + 1;
  nextPage = (int)(nextFileNum / 10) + 1;
  if (currPage != nextPage) {
    printFileList(nextFileNum);
    return;
  }
  currRow = currFileNum % 10;
  nextRow = nextFileNum % 10;
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%2d%c%-13s%8s",
          fileList[currFileNum]->num, fileList[currFileNum]->attribute, fileList[currFileNum]->name, fileList[currFileNum]->size);
  myGLCD.print(buf, 24, 34 + currRow * 14);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  sprintf(buf, "%2d%c%-13s%8s",
          fileList[nextFileNum]->num, fileList[nextFileNum]->attribute, fileList[nextFileNum]->name, fileList[nextFileNum]->size);
  myGLCD.print(buf, 24, 34 + nextRow * 14);
}

boolean deleteFile(int fileNum) {
  int i;
  int x, y;
  boolean deleted;
  char buf[BUFFER_LEN_FULL];

  printWindowFrame(16, 62, 296, 160);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  if (fileNum == FILENUM_DELETE_ALL) {
    strcpy(buf, "DELETE ALL FILES ? ");
  } else {
    sprintf(buf, "DELETE \"%s\" ?", fileList[fileNum]->name);
  }
  myGLCD.print(buf, 32, 90);
  printButton("OK", "", 144, 124, 208, 152, STS_NORMAL);
  printButton("CANCEL", "", 216, 124, 280, 152, STS_NORMAL);
  while (1) {
    if (myTouch.dataAvailable()) {
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if ((y >= 124) && (y <= 152)) {
        // OK
        if ((x >= 144) && (x <= 208)) {
          beep(BEEP_HIGH);
          //DELETE ALL
          if (fileNum == FILENUM_DELETE_ALL) {
            for (i = 1; i < numberOfFiles; i++) {
              if (fileList[fileNum]->attribute == ' ') {
                SD.remove(fileList[i]->name);
              }
            }
            i = getFileList();
            if (i == 0) {
              return true;
            } else {
              return false;
            }
          } else {
            return SD.remove(fileList[fileNum]->name);
          }
        }
        // CANCEL
        if ((x >= 216) && (x <= 280)) {
          beep(BEEP_HIGH);
          return false;
        }
      }
    }
  }
}

void printFileListWindow(int fileNum) {
  printWindowFrame(16, 20, 296, 216);
  sdSts = getFileList();
  printFileList(fileNum);
  printButton("PAGE", "UP", 232, 27, 288, 53, STS_NORMAL);
  printButton("CURSOR", "UP", 232, 57, 288, 83, STS_NORMAL);
  printButton("CURSOR", "DOWN", 232, 87, 288, 113, STS_NORMAL);
  printButton("PAGE", "DOWN", 232, 117, 288, 143, STS_NORMAL);
  printButton("DELETE", "ALL", 80, 180, 144, 208, STS_NORMAL);
  printButton("DELETE", "", 152, 180, 216, 208, STS_NORMAL);
  printButton("CANCEL", "", 224, 180, 288, 208, STS_NORMAL);
}

void setSdLastUpdatedTime(void) {
  if (sdLastUpdatedTime[0] == '\0') {
    strcpy(sdLastUpdatedTime, "--/--/-- --:--");
  } else {
    Time t = getClockTime();
    sprintf(sdLastUpdatedTime, "%02d/%02d/%02d %02d:%02d", t.yr % 100, t.mon, t.date, t.hr, t.min);
  }
}

// (ファイル書込時タイムスタンプ更新のためのコールバック関数)
void dateTime(uint16_t* date, uint16_t* time) {
  uint16_t year;
  uint8_t month, day, hour, minute, second;

  // User gets date and time from GPS or real-time clock here
  Time t = getClockTime();
  year = t.yr;
  month = t.mon;
  day = t.date;
  hour = t.hr;
  minute = t.min;
  second = t.sec;

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year, month, day);

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour, minute, second);
}

void saveDataToSdCard(int screenMode) {
  File dataFile;
  char dataString[MAX_DATASTRING_LEN];
  char fileName[MAX_FILE_NAME_LEN];
  char buf[BUFFER_LEN_HALF];

  if (sdSts == SD_CARD_NOT_PRESENT) {
    return;
  }
  Time t = getClockTime();
  sprintf(fileName, "%04d%02d.CSV", t.yr, t.mon);
  // ファイルがない場合は、タイトル行を書き込み
  if (!SD.exists(fileName)) {
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println(
      "yyyy/mm/dd,hh:mm,"
      "Integrated(Wh),"
      "Sensor1(Wh),Sensor2(Wh),Sensor3(Wh),Total(Wh),"
      "Sensor1(Peak W),Sensor2(Peak W),Sensor3(Peak W),Total(Peak W)");
    dataFile.close();
  }
  // ここからデータ行を書き込み
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    sdSts = SD_WRITING;
    printSdStatus(screenMode);
    dtostrf(wattHourTimeperiod, -1, 1, buf);
    strchr(buf, '.')[0] = '\0';
    // データの文字数を増やす場合は、dataString変数のサイズを超えないようにする
    sprintf(dataString, "%04d/%02d/%02d,%02d:%02d,%s,%d,%d,%d,%d,%d,%d,%d,%d",
            t.yr, t.mon, t.date, t.hr, t.min,
            buf,
            (int)watt_hour1, (int)watt_hour2, (int)watt_hour3, (int)watt_hour,
            (int)watt_max[1], (int)watt_max[2], (int)watt_max[3], (int)watt_max[0]);
    if (dataFile.println(dataString) > 0) {
      setSdLastUpdatedTime();
      sdSts = SD_SUCCESS;
      printSdStatus(screenMode);
      watt_hour1 = watt_hour1 - (int)watt_hour1;
      watt_hour2 = watt_hour2 - (int)watt_hour2;
      watt_hour3 = watt_hour3 - (int)watt_hour3;
      watt_hour = watt_hour - (int)watt_hour;
      irms_max[1] = 0.0;
      irms_max[2] = 0.0;
      irms_max[3] = 0.0;
      watt_max[1] = 0.0;
      watt_max[2] = 0.0;
      watt_max[3] = 0.0;
      watt_max[0] = 0.0;
    } else {
      sdSts = SD_WRITE_ERROR;
      printSdStatus(screenMode);
    }
    dataFile.close();
  } else {
    sdSts = SD_OPEN_ERROR;
    printSdStatus(screenMode);
  }
}

void saveEventToSdCard(String event, int screenMode, int printMode) {
  File dataFile;
  char dataString[MAX_DATASTRING_LEN];
  char fileName[MAX_FILE_NAME_LEN];

  if (sdSts == SD_CARD_NOT_PRESENT) {
    return;
  }
  strcpy(fileName, "EVENT.CSV");
  // ファイルがない場合は、タイトル行を書き込み
  if (!SD.exists(fileName)) {
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println("yyyy/mm/dd,hh:mm:ss,event");
    dataFile.close();
  }
  // ここからデータ行を書き込み
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    sdSts = SD_WRITING;
    printSdStatus(screenMode, printMode);
    Time t = getClockTime();
    // データの文字数を増やす場合は、dataString変数のサイズを超えないようにする
    sprintf(dataString, "%04d/%02d/%02d,%02d:%02d:%02d,%s",
            t.yr, t.mon, t.date, t.hr, t.min, t.sec, event.c_str());
    if (dataFile.println(dataString) > 0) {
      sdSts = SD_SUCCESS;
      printSdStatus(screenMode, printMode);
    } else {
      sdSts = SD_WRITE_ERROR;
      printSdStatus(screenMode, printMode);
    }
    dataFile.close();
  } else {
    sdSts = SD_OPEN_ERROR;
    printSdStatus(screenMode, printMode);
  }
}

void saveWattHourTimeperiod(void) {
  Time t = getClockTime();
  sprintf(currTime, "%02d:%02d", t.hr, t.min);
  if ((t.date == eepromAutoResetDate) && (strcmp(currTime, "00:00") == 0)) {
    if (needsSaveIntegratedValues == true) {
      saveIntegratedValuesToSdCard(currScreenMode);
      saveEventToSdCard("Integrated values reset (automatically)", currScreenMode);
      needsSaveIntegratedValues = false;
    }
  } else {
    if ((t.min % SAVING_INTERVAL == 0) && (strcmp(currTime, prevTime) != 0)) {
      strcpy(prevTime, currTime);
      saveDataToSdCard(currScreenMode);
    }
  }
}

// ************************************************************************************
// setup(), loop()関数
void setup() {
  byte value;
  char event[BUFFER_LEN_EVENT];

  pinMode(PIN_SPEAKER, OUTPUT);
  beep(BEEP_PIPO);
  pinMode(PIN_LCD_LED, OUTPUT);
  digitalWrite(PIN_LCD_LED, LOW);
  myGLCD.InitLCD();
  clearScreen();
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("Watt Monitor"), 0, 0);
  curX = 0;
  curY = 18;
  myGLCD.setFont(SmallFont);
  printEx(VER);
  printEx(F("\n"));
  myGLCD.setColor(VGA_WHITE);
  pinMode(PIN_SDCARD_VDD, OUTPUT);
  digitalWrite(PIN_SDCARD_VDD, HIGH);
  delay(5000);
  // 各パラメーターの初期値
  eepromScreenOffTime = DEFAULT_SCREEN_OFF_TIME;
  eepromAutoResetDate = DEFAULT_AUTO_RESET_DATE;
  eepromCtVoltage[1] = DEFAULT_CT1_VOLTAGE;
  eepromCtVoltage[2] = DEFAULT_CT2_VOLTAGE;
  eepromCtVoltage[3] = DEFAULT_CT3_VOLTAGE;
  eepromVtPercentage = DEFAULT_VT_PERCENTAGE;
  eepromCtPercentage[1] = DEFAULT_CT1_PERCENTAGE;
  eepromCtPercentage[2] = DEFAULT_CT2_PERCENTAGE;
  eepromCtPercentage[3] = DEFAULT_CT3_PERCENTAGE;
  eepromSound = DEFAULT_SOUND;
  eepromClock = DEFAULT_CLOCK;
  eepromAutoTimeSync = DEFAULT_AUTO_TIME_SYNC;
  eepromTimeSyncHour = DEFAULT_TIME_SYNC_HOUR;
  eepromTimeSyncMin = DEFAULT_TIME_SYNC_MIN;
  eepromBarGraphFullscale = DEFAULT_BAR_GRAPH_FULLSCALE;
  // EEPROM内のデータが有効であれば各パラメーターをEEPROM内のデータで上書きする
  value = EEPROM.read(EEPROM_SCREEN_OFF_TIME); if ((value >= 0) && (value <= 99)) eepromScreenOffTime = value;
  value = EEPROM.read(EEPROM_AUTO_RESET_DATE); if ((value >= 0) && (value <= 28)) eepromAutoResetDate = value;
  value = EEPROM.read(EEPROM_CT1_VOLTAGE); if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[1] = value;
  value = EEPROM.read(EEPROM_CT2_VOLTAGE); if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[2] = value;
  value = EEPROM.read(EEPROM_CT3_VOLTAGE); if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[3] = value;
  value = EEPROM.read(EEPROM_SOUND); if ((value == OFF) || (value == ON)) eepromSound = value;
  value = EEPROM.read(EEPROM_CLOCK); if ((value == OFF) || (value == ON)) eepromClock = value;
  value = EEPROM.read(EEPROM_AUTO_SYNC_TIME); if ((value == OFF) || (value == ON)) eepromAutoTimeSync = value;
  value = EEPROM.read(EEPROM_TIME_SYNC_HOUR); if ((value >= 0) && (value <= 23)) eepromTimeSyncHour = value;
  value = EEPROM.read(EEPROM_TIME_SYNC_MIN); if ((value >= 0) && (value <= 59)) eepromTimeSyncMin = value;
  value = EEPROM.read(EEPROM_BAR_GRAPH_FULLSCALE); if ((value >= 1) && (value <= 49)) eepromBarGraphFullscale = value;
  // EEPROM書き込み時はデータを-1したうえで保存し、 EEPROM読み出し時は+1した値を使用する
  // CONFIGデータ| EEPROMデータ
  // ------------+----------------------------
  // 0～254      | 1～255のデータを表す
  // 255         | データ消去状態を表す
  value = EEPROM.read(EEPROM_VT_PERCENTAGE); if ((value >= 0) && (value <= 254)) eepromVtPercentage = value + 1;
  value = EEPROM.read(EEPROM_CT1_PERCENTAGE); if ((value >= 0) && (value <= 254)) eepromCtPercentage[1] = value + 1;
  value = EEPROM.read(EEPROM_CT2_PERCENTAGE); if ((value >= 0) && (value <= 254)) eepromCtPercentage[2] = value + 1;
  value = EEPROM.read(EEPROM_CT3_PERCENTAGE); if ((value >= 0) && (value <= 254)) eepromCtPercentage[3] = value + 1;
  elapsedDays = 1;
  resetIntegratedValues();
  ds1302.writeProtect(false);
  ds1302.halt(false);
  Time t = ds1302.time();
  setClockTime(t);
  prevDate = t.date;
  setSdLastUpdatedTime();
  sprintf(resetTime, "%02d/%02d/%02d %02d:%02d", t.yr % 100, t.mon, t.date, t.hr, t.min);
  pinMode(PIN_SS, OUTPUT);
  SdFile::dateTimeCallback(&dateTime); // ファイル書込時タイムスタンプ更新のため関数を登録
  //SDカードの状態確認
  sdSts = getFileList();
  if (sdSts != SD_SUCCESS) {
    printEx(F("\n"));
    printEx(sdStsMsg[sdSts]);
    abort();
  }
  currScreenMode = SCREEN_SETUP;
  saveEventToSdCard(F("System started"), currScreenMode, PRINTMODE_LINE);
  existsFlashAir = false;
#ifdef FLASH_AIR
  //FlashAirの無線LAN機能の起動
  clearScreen();
  if (getCardStatus() == true) {
    existsFlashAir = true;
    //日本標準時の取得
    isSynchronized = syncTime();
    if (isSynchronized == true) {
      sprintf(event, "[%04lX][%s]Time synchronized (setup)", nextSequenceId - 1, httpCommandArg1);
      saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
    } else {
      sprintf(event, "[%04lX][%s]Time synchronization failed (setup)", nextSequenceId - 1, httpCommandArg1);
      saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
    }
  }
  delay(3000);
  myGLCD.clrScr();
#endif
  currScreenMode = printMainWindow(SCREEN_REDRAW);
  hasBlackout = false;
  hasBrownout = false;
  needsResetFlashAir = false;
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  scrOffTimer1 = 0;
  scrOffTimer2 = TIMER_RESET;
  scrOffTimerInterval.reset();
  updateScreenInterval.reset();
  //タイマー割り込み
  timer1_state  = STATE_SENSOR1;
  Timer1.initialize(250000); // 250ms間隔で割り込み
  Timer1.attachInterrupt(timer1); // 割り込み時に呼び出す関数を指定
}

void loop() {
  // メインの処理
  if (updateScreenInterval.check() == 1) {
    switch (currScreenMode) {
      case SCREEN_ON:
        myGLCD.setFont(SmallFont);
        printStringEx("Printing", 168, 174, VGA_WHITE, VGA_BLACK);
        printDateTime();                      // 現在日時を表示
        printVoltAmpere();                    // 電圧・電流を表示
        printBlackout();                      // 停電時の表示
        printWattHour();                      // 設定した時間単位の電力量、ピーク電力を表示
        printWattHourTimeperiod();            // 時間帯別積算電力量を表示
        printBarGraph();                      // 消費電力のバーグラフを表示
        myGLCD.setFont(SmallFont);
        printStringEx("        ", 168, 174, VGA_WHITE, VGA_BLACK);
        break;
      case SCREEN_CLOCK:
        printClockDateTime();
        printWattHourTimeperiod(currScreenMode);
        printBarGraph(currScreenMode);
        break;
      default:
        //何もしない
        break;
    }
  }
  popupWindow();            // ボタンにタッチした時の処理
  setScreenMode();          // 画面表示ON/OFF制御
  adjustIntlClockByRtc();   // 1分に1回(30秒の時)システム時刻をRTCに合わせる
  countupTimer();           // 1分毎にタイマーを動かす
  incrElapsedDays();        // 1日1回累積日数の加算
  saveWattHourTimeperiod(); // 設定した時間間隔(30分)毎にSDカードにデータを保存する
  syncTimeOnceADay();       // 1日1回時刻をRTCとシステム時刻を日本標準時に合わせる
}
