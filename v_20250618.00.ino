#define VER "Program version 20250618.00"
//**************************************************************************************************
/*
  電力モニター Watt_Monitor
  ■主要部品
  ・ATmega1284P-PU 3.3V 12MHz水晶で動作
  ・DS1302(動作電圧：2.0V～5.5V)
  ・秋月 高精度電流センサー [SR-3702-150N/14Z]
  　適用電流：～80A AC
  　変流比:3000対1(測定電流30Aに対して出力電流0.01A、負荷抵抗100Ωの場合0.01A*100Ω=1Vを出力)
  ・秋月 ＳＤカードスロットＤＩＰ化モジュール(Cixi Borui Electronic Technology製) -> 秋月　AE-SDに取替
  ・Aitendo 3.2inch LCD(M032C1289TP)(動作電圧：3.3V)(320 * 240) 8ビットモードで使用
    https://www.aitendo.com/product/3861
    (16ビット⇨8ビットへの変更方法)(Aitendoから引用)
   デフォルトは16ビットです。R4を（半田こて）取り外して→R5に取り付けて8ビットモードになります。8ビットの接続：DB0〜DB7
  　↓
   version 20250515以降
  ・3.2インチのタッチスクリーンLCDディスプレイモジュール,
    3.2インチ,カラー,il9341 240x320 3.3v〜5v,8ビット/16ビット,パラレルインターフェース
    https://ja.aliexpress.com/item/1005006935859647.html
    (16ビット⇨8ビットへの変更方法)
    デフォルトは16ビット。R16の0Ω抵抗をR8に移動し8ビットモードにする。8ビットの接続：DB8〜DB15

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

  ■IDE (1.8.5)(1.8.13)(1.8.19)(2.2.1)
  　・スケッチと同じフォルダに入れておくファイル
  　　FlashAir関係 iSdio.cpp, iSdio.h
  　　SDカード関係 Sd2Card.h, Sd2CardExt.cpp,Sd2CardExt.h, Sd2PinMap.h,SdInfo.h
  　　フォント関係 GroteskBold32x64.c,SevenSeg_XXXL_Num.c, SixteenSegment32x48.c　
  　・ツール　　ボード：OSWIN ATMega1284P 12MHz using Optiboot (Bobuino pinlayout)
  　・スケッチ　書込装置(Arduini as ISP)を使って書き込む

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
  2022/5/11 incrElapsedDays()関数の位置をsaveWattHourTimeperiodToSdCard()の前に移動
  2022/5/11 setTime()関数 内蔵時計機能使用時にsetTime()関数を誤って再帰的に呼び出していたため修正
  2022/5/11 setTime()他RTC関係の関数の誤りを修正
  2022/7/10 resetIntegratedValues()関数 watt_sum = 0.0 を追加
  2022/7/11 いくつかの変数、定数の名前変更
  2022/7/16 DEFAULT_CT1_PERCENTAGE,DEFAULT_CT2_PERCENTAGE 134 -> 140
  2022/8/21 resetIntegratedValues()関数 saveIntegratedValuesToSdCardに名前変更
  2022/8/21 累積値のリセットをresetIntegratedValues()関数にまとめる
  2022/8/28 saveIntegratedValuesToSdCard()関数 計測を一時停止してリセットした累積値をすぐにSDカードに記録する
  2022/8/28 saveWattHourTimeperiodToSdCard()とincrElapsedDays()の実行順を逆にする
  2022/8/28 beep()関数 サウンドOFFの時にもdelayを実行する
  2022/9/14 saveWattHourTimeperiodToSdCard()関数 自動リセット日の00:00には記録しないように修正
  2022/9/15 saveWattHourTimeperiodToSdCard()関数 自動リセット日の00:00に記録する処理をここに移動
  2022/10/11 saveWattHourTimeperiodToSdCard()関数 currTimeのコロン漏れを修正
  2022/12/2 getButtonNum()関数 BTN_PRESSEDを追加してボタン以外のタッチでも消灯タイマーをリセットする
  2022/12/13 resetSystem()関数 システムリセット(ソフトリセット)の機能を追加
  2022/12/13 resetSystem()関数 システムリセットの画面を追加
  2022/12/16 resetSystem()関数 表示内容を変更
  2022/12/17 config()関数 "Clock"と"Sync JST"を変更
  2023/1/1 getCardStatus()関数 表示内容を追加
  2023/5/11 config()関数 "Clock disp"と"Clock Sync"を変更
  2023/5/27 config()関数 設定項目の表示位置を簡単に変更てきるように変更
  2023/5/30 syncTimeOnceADay(),adjustDateTime()関数 サーバ名をログに記録しないよう変更
  2023/6/6 memorysaver.h 修正(SSD1289_8以外の行を有効化)するとフラッシュメモリの使用量は減る(88%)が
  　動作が不安定(30分値途中から記録されず。LCDの表示が異常になる。)になるため修正しない -> おそらく不具合と無関係
    30分値途中から記録されない -> おそらくローカル変数のメモリー不足またはスタックのオバーフロー -> 2023/10/2修正
    LCDの表示が異常になる -> ゼロプレッシャーソケットのレバーが圧電スピーカーに接触時によく起こる。おそらくハードの接触不良　
  2023/6/8 printFileList()関数 ファイル数が10で割り切れる場合に、存在しないファイルが1行余計に表示される不具合を修正
  2023/6/11 定数をenumで定義
  2023/6/16 sdStsMsg[]変数の初期化部分を修正
  2023/6/18 beep()関数 BEEP_800MSの場合、sound off時に800msのdelayを入れる
  2023/6/25 popupWidow表示中に一定時間が経過すると閉じるようにタイマー(popupWindowOffTimer)を追加
  2023/6/25 popupWidow表示中に停電,復電した時にSDカード書込の表示が出ないように修正
  2023/8/10 変数名の綴りを修正 ROW_HIGHT -> ROW_HEIGHT
  2023/9/15 CONFIG画面でOKを押したらイベント(Settings changed)を書き込むよう追加
  2023/9/17 CONFIG画面でOKを押したらイベント(値の変更内容)を書き込むよう変更
  2023/9/20 printBlackout()関数内の画面ON/OFF操作部分をsetCsreenMode()関数に移動して修正
  2023/9/22 screenoffTimerOnBattery等、停電時画面自動消灯用加算タイマー関係の変数名変更
  2023/9/25 停電中に、画面OFFからタッチして画面表示させるとすぐまた画面OFFになる不具合他を修正
  2023/9/25 popupWindow自動キャンセル用減算タイマーを加算タイマーに変更
  2023/9/26 config()関数内の2重のwhileループの内側のループを削除
  2023/9/27 printBarGraphFrame()他各関数のデフォルト引数screenModeのデフォルト値をSCREEN_CURRENTに変更
  2023/9/27 setScreenMode()関数 画面OFFタイマー作動でSCREEN_CLOCKに切り替わると再描画を繰り返す不具合を修正
  2023/9/27 測定値の誤差修正率(%)のデータ範囲を 1～255から1～254に変更
  2023/9/27 saveWattHourTimeperiodToSdCard()関数他をupdateScreenInterval.check()==1の時に実行するように場所移動
  2023/9/28 修正したmemorysaver.hをいつのまにか使っていたため、修正前のmemorysaver.hに戻す
  2023/9/29 countupTimer()関数だけ元の場所に戻す
  2023/9/30 細かい修正
  2023/10/1 config()関数内のitem[]変数の定義(struct ITEM item[EEPROM_MODE_MAX];)をグローバルに移動。修正したmemorysaver.hを使用
  2023/10/1 syncTime()関数 #define,#ifdef,#endif文でどちらかの関数を呼び出すように修正、他 細かい修正
    スケッチが114906バイト（88%）、グローバル変数は6040バイト使用
  2023/10/2 config()関数内のitem[]変数の定義(struct ITEM item[EEPROM_MODE_MAX];)をローカルに戻して、static修飾子付加
    スケッチが114906バイト（88%）、グローバル変数は6040バイト使用
  2023/10/2 ITEM構造体のname,formatメンバーをchar型の配列からchar型のポインタへ変更し、item[]変数のstatic修飾子を外す
    スケッチが114912バイト（88%）、グローバル変数は5622バイト使用
  2023/10/2 getCardStatus()関数の名前をinitFlashAirに変更
  2023/10/2 isSynchronized変数の名前をisTimeSynchronizedに変更
    スケッチが115168バイト（88%）、グローバル変数は5487バイト使用
  2023/10/3 saveWattHourTimeperiodToSdCard()関数 wattHourTimeperiod変数が99999kWhを超えたら積算値を強制的にリセットする機能を追加
    スケッチが115244バイト（88%）、グローバル変数は5453バイト使用
  2023/10/4 config()関数 Screen off timer の設定値に単位(min)も表示
  2023/10/5 setup()関数 電源ONからFlashAirイニシャライズ開始までの待ち時間を追加
  2023/10/5 timer1()関数他 popupWindow表示中に復電した場合に画面表示が正しく切り替わらない不具合を修正
  2023/10/5 timer1()関数 popupWindow表示中に復電または停電した場合にメイン画面に戻るよう修正
  2023/10/6 config()関数 eepromScreenOffTimer変数の値の書式を"%02dmin"から"%2dmin"に修正
  2023/10/7 saveEventToSdCard()、saveDataToSdCard()関数 if文を修正
    スケッチが115390バイト（88%）、グローバル変数は5461バイト使用
  2023/10/9 timer1()関数 細かい修正
  2023/11/10 syncTimeOnceADay()関数 sprintf(event, "SYSTEM: [%04lX][Time synchronization failed"...の文字列修正
  2023/11/16 printSyncTimeInfo)関数 "[T]"を"T"に変更し、関連する部分を修正
  2023/11/16 needsResetFlashAir変数の名前をisResetFlashAirRequestedに変更
  2023/11/16 syncTimeInfo[]内の'R'を書き込む位置を直接数値ではなく定数RESET_FLASHAIR_REQUEST_MARKに変更
  2024/4/26 iSDIO_sendHttpRequest()関数 レスポンス待ちのメッセージ変更
  2024/4/28 initFlashAir()関数、resetSystem()関数 メッセージ変更
  2024/8/15 strtokEx()関数 findSubstring()関数とfindLine()関数に関数名変更
  2024/8/15 WorldTimeAPIが返すデータの並び順が変わり(datetime:の前にutc_datetime:が来る)時刻を正しく処理できなくなったため
    findLine()関数に引数(indexVal)を追加して検索する文字列の位置を指定する機能を追加
  2024/9/21 iSDIO_sendHttpRequest()関数 10秒待つ処理の画面表示を変更
  2024/9/28 iSDIO_status()関数 IPアドレスが不正の場合、flashAirIP変数を空にしない
  2025/2/6 World Time APIがサーバ不安定のためTime APIを追加して切り替え
  2025/2/6 Time API(HTTPS)から時刻が取得できないためAPI Ninjasを追加
  2025/2/7 API Ninjas(HTTPS)から時刻が取得できないためWorld Clock APIを追加
    Time API,API Ninjasのどちらも送信は正常終了するが受信ができない。(iSDIO_waitHttpResponseの結果がProcess Failed C0)
    同じHTTPSでもVisual Crossing Weatherは受信まで正常終了。
    ↓HTTPS/TLS証明書問題の可能性あり
    https://seesaawiki.jp/flashair-dev/d/HTTPS/TLS%be%da%cc%c0%bd%f1%cc%e4%c2%ea
  2025/2/9 World Clock APIを時刻取得先サーバに設定
  2025/2/10 NICTのsyncTime()関数を削除
  2025/2/11 printlnEx()関数を追加
  2025/2/12 World Clock APIでの503. The service is unavailable時にも日時を取得する
  2025/2/13 表示の改行漏れの手直し
  2025/2/15 statusMsg変数内の改行コードを削除
  2025/2/17 t += 32400 -> t += 32400UL
  2025/2/23 World Clock APIでの503. The service is unavailable時の処理を修正
  2025/2/23 SD CardのDELETE ALL時にCSVファイル以外は削除しないように修正
  2025/2/23 SD Cardのファイル削除時にイベント書込するよう修正
  2025/2/24 不要な画面消去、入力漏れのまま自動整形したため発生した文字列内の不要なスペースを削除
  2025/3/8 iSDIO_waitHttpResponse()関数 "Timed Out" を "time out" に修正
  2025/4/5 syncTime()関数 tm.Monthがずれる時があるためサーバーからのレスポンスをHTTP_RES.TXTに記録する処理を追加、他修正
  2025/4/6 syncTime()関数 修正
  2025/4/6 syncTime()関数 isTimeSynchronized = false の時 httpResponse[0] = '\0' 追加
  2025/4/6 syncTime()関数 tm変数の各メンバーの0初期化を追加
  2025/4/13 サーバーからのレスポンスをHTTP_RES.TXTに記録する処理をsaveHttpResponseToSdCard()関数化
   memorysaver.h 修正(SSD1289_8以外の行を有効化)
    最大130048バイトのフラッシュメモリのうち、スケッチが126744バイト（97%）を使っています。
    グローバル変数は5949バイトのRAMを使用しています。
   memorysaver.h 修正(ILI*,S*以外の行を有効化)
    最大130048バイトのフラッシュメモリのうち、スケッチが124224バイト（95%）を使っています。
    グローバル変数は5949バイトのRAMを使用しています。
  2025/4/13 ENABLE_MODE_CLOCKが未定義だと、MODE_CLOCKが無効化されその分のメモリが節約されるように修正
  2025/5/17 LCDをAliExpress購入品に取り替え、memorysaver.h 修正(ILI9341_8以外の行を有効化) -> Setup()関数完了までの画面表示が安定
  2025/5/17 Timer1の割り込み設定変更 -> Loop()関数に入ってからの動作が安定
  2025/5/19 30分積算データがSDカードに自動保存されない不具合発生、再起動すると直ったり直らなかったり
  -> loop()関数内の各関数の順序を修正、 saveWattHourTimeperiodToSdCard()関数内 saveDataToSdCard()の後にstrcpy(prevTime, currTime)を実行
    最大130048バイトのフラッシュメモリのうち、スケッチが118840バイト（91%）を使っています。
    グローバル変数は5963バイトのRAMを使用しています。
  2025/5/20 不具合発生-> IDE Ver.2.2.1でコンパイル
  2025/5/22 不具合発生-> IDE Ver.2.3.2でコンパイル(Ver.2.3.3以降はエラー発生し使用不可)
  -> saveWattHourTimeperiodToSdCard()関数 sprintf文とprintStringEx文があると不具合発生せず
  2025/5/24 sprintf文とprintStringEx文を取り、SAVING_INTERVAL定数の使用をやめて30に置き換え、saveWattHourTimeperiodToSdCard()関数 if文他を修正
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 ダミーのprintStringEx文を追加
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 ダミーでないprintStringEx文に修正
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 ダミーでないprintStringEx文を取り、sprintf文を追加
  -> 不具合発生せず
  2025/5/24 saveWattHourTimeperiodToSdCard()関数 sprintf文を取り、noInterrupts,interrupts文を追加
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 snoInterrupts,interrupts文を取り、if文を早期returnに修正
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 if文の前にdelay(100)を追加
  2025/5/24 不具合発生 -> saveWattHourTimeperiodToSdCard()関数 delay(100)を取り、sprintf文を追加
  -> 不具合発生せず
  2025/5/26 sprintf文を削除、saveWattHourTimeperiodToSdCard()関数内に記述している処理をloop()関数内に直に記述するように修正
  -> 不具合発生せず
  2025/6/8 RESETするとすぐ2Daysになるのを修正
  起動してしばらくすると小さいbeep音が断続的になる不具合発生
  　2025/6/10 画面オフタイマーが動作せず -> countupTimer()関数内に記述している処理をloop()関数内に直に記述するように修正
  　2025/6/11 タッチスクリーンが反応しない -> popupWindow()関数内に記述している処理をloop()関数内に直に記述するように修正
  　beep音は、タッチスクリーンの四隅を圧迫しているのが原因の可能性あり、固定しているビスを少し緩めて様子をみる
  時々画面表示が異常になる
    2025/6/13 できるだけ割り込み中に画面描画をしないように変更
    2025/6/14 できるだけ割り込み中に画面描画をしないように変更
  saveWattHourTimeperiodToSdCard()関数内に記述している処理をloop()関数内に直に記述するように修正したのを元に戻す
  30分積算データがSDカードに自動保存されない不具合発生
    2025/6/15 currTime,prevTime変数にvolatile修飾子を追加
    -> 不具合発生
    2025/6/15 currTime,prevTime変数のvolatile修飾子を取る　if文を修正
    2025/6/15 saveWattHourTimeperiodToSdCard()関数 printStringEx文を追加
    2025/6/16 saveWattHourTimeperiodToSdCard()関数 printStringEx文を修正
    -> 不具合発生せず
  2025/6/16 loop()関数 修正
  画面消灯からタッチで復帰した時に画面真っ白
    2025/6/17  loop()関数 timer1_status = TIMER1_STS_PRINTED;の直前のpopupWindow();をコメント化
    2025/6/17 loop()関数 割り込み中は極力処理をしないよう修正
  2025/6/18 loop()関数 修正
  2025/6/18 setup()関数 prevSec,prevDate,elapsedDaysの初期化処理を追加
*/

// **************************************************************************************************
// ATmega1284P-PU　ピン配置(Bobuino pin layout)
// 　https://maniacbug.wordpress.com/2011/11/27/arduino-on-atmega1284p-4/
//   maniacbug-mighty-1284p-68ed99c.zip
// 　(C:\Program Files\Arduino\hardware\arduino\avr\variants\bobuino\pins_arduino.h)
enum { OFF,
       ON
};
#define ENABLE_MODE_CLOCK
enum { MODE_BLANK,
       MODE_CLOCK
};  //SCREEN OFFボタンを押した時のモードの選択
//時刻を取得するサーバの選択
#define PROVIDER_WORLD_CLOCK_API  // サーバ不安定
//#define PROVIDER_API_NINJAS // Process Failed C0
//#define PROVIDER_TIME_API // Process Failed C0
//#define PROVIDER_WORLD_TIME_API // サーバ不安定
//#define PROVIDER_NICT // サーバ停止のため使用不可
// 汎用の文字列格納用バッファのサイズ
const int BUFFER_LEN_FULL = 41;  // LCD画面1行の最大の文字数　320 ÷ 8(SmallFontの文字幅) + '\0' = 41
const int BUFFER_LEN_HALF = 21;  // LCD画面1行の半分の文字数　160 ÷ 8(SmallFontの文字幅) + '\0' = 21
// SDカードに保存するイベント情報格納用バッファのサイズ
const int BUFFER_LEN_EVENT = 101;  //100文字 + '\0' = 101

// 環境に合わせて修正が必要な設定値
#define FLASH_AIR  // FlashAir使用の場合に定義
// 商用電源周波数(Hz)
const unsigned long POWER_FREQ = 50L;
// calcWatt()関数に使用する定数(実測に基づく係数)
const float VT_CONVERSION_RATIO = 71.29;  // VTの出力電圧を実際の電圧に換算する比率(%)
const float CT1_LOAD_RESISTANCE = 99.8;   // CT(センサーNo.1)の負荷抵抗(Ω)
const float CT2_LOAD_RESISTANCE = 100.2;  // CT(センサーNo.2)の負荷抵抗(Ω)
const float CT3_LOAD_RESISTANCE = 100.3;  // CT(センサーNo.3)の負荷抵抗(Ω)

// EEPROMに保存する初期値
// 1.EEPROMから読み出した値が有効な範囲にない場合に使用する
// 2.CONFIGのLOAD DEFAULTを実行した場合に使用する
// 毎月自動で積算値をリセットする日付(日) 0～28
// 0にすると自動リセットしない
const byte DEFAULT_AUTO_RESET_DATE = 11;
// 測定対象の電圧(V) 0 or 100 or 200
// 0にすると測定しない
const byte DEFAULT_CT1_VOLTAGE = 100;
const byte DEFAULT_CT2_VOLTAGE = 100;
const byte DEFAULT_CT3_VOLTAGE = 200;
// 測定値の誤差修正率(%) 1～254
const byte DEFAULT_VT_PERCENTAGE = 89;
const byte DEFAULT_CT1_PERCENTAGE = 139;
const byte DEFAULT_CT2_PERCENTAGE = 139;
const byte DEFAULT_CT3_PERCENTAGE = 176;
// 自動消灯設定時間(分) 0～99
// 最後の操作から一定時間が経過したら表示を消す NEVER_SCREEN_OFF(0)にすると消灯しない
const byte NEVER_SCREEN_OFF = 0;
const byte DEFAULT_SCREEN_OFF_TIMER = 10;
// SCREEN OFF時、時計を表示 MODE_CLOCK or 表示しない MODE_BLANK
const byte DEFAULT_SCREEN_OFF_MODE = MODE_BLANK;
// 音を鳴らす ON or 鳴らさない OFF
const byte DEFAULT_SOUND = ON;
// 自動で時刻合わせするかどうか
const byte DEFAULT_AUTO_TIME_SYNC = ON;
// 自動で時刻合わせする時刻
const byte DEFAULT_TIME_SYNC_HOUR = 15;
const byte DEFAULT_TIME_SYNC_MIN = 5;
// バーグラフのフルスケール(kW)
const byte DEFAULT_BAR_GRAPH_FULLSCALE = 20;

// EEPROM関係
#include <EEPROM.h>
byte eepromScreenOffTimer;
byte eepromAutoResetDate;
byte eepromCtVoltage[4];
byte eepromVtPercentage, eepromCtPercentage[4];
byte eepromSound, eepromScreenOffMode;
byte eepromAutoTimeSync;
byte eepromTimeSyncHour, eepromTimeSyncMin;
byte eepromBarGraphFullscale;
enum eeprom {
  EEPROM_AUTO_RESET_DATE,
  EEPROM_VT_PERCENTAGE,
  EEPROM_CT1_VOLTAGE,
  EEPROM_CT1_PERCENTAGE,
  EEPROM_CT2_VOLTAGE,
  EEPROM_CT2_PERCENTAGE,
  EEPROM_CT3_VOLTAGE,
  EEPROM_CT3_PERCENTAGE,
  EEPROM_SCREEN_OFF_TIMER,
  EEPROM_SCREEN_OFF_MODE,
  EEPROM_SOUND,
  EEPROM_AUTO_TIME_SYNC,
  EEPROM_TIME_SYNC_HOUR,
  EEPROM_TIME_SYNC_MIN,
  EEPROM_BAR_GRAPH_FULLSCALE,
  EEPROM_MODE_MAX
};

// 時刻関係
//https://github.com/PaulStoffregen/Time
//Time-master.zip
#include <TimeLib.h>
char currTime[6] = "";  //"hh:mm" + '\0'
char prevTime[6] = "";  //"hh:mm" + '\0'
enum {
  TIME_YEAR,
  TIME_MONTH,
  TIME_DAY,
  TIME_DAYNAME,
  TIME_HOUR,
  TIME_MINUTE,
  TIME_SECOND
};

// DS1302関係
// https://github.com/msparks/arduino-ds1302/
// arduino-ds1302-master.zip
#include <DS1302.h>
namespace {
const int PIN_DS1302_CE = 29;    // DS1302 Pin 5(CE)   -> Arduino Pin pin 29
const int PIN_DS1302_IO = 28;    // DS1302 Pin 6(I/O)  -> Arduino Pin pin 28
const int PIN_DS1302_SCLK = 27;  // DS1302 Pin 7(SCLK) -> Arduino Pin pin 27
DS1302 ds1302(PIN_DS1302_CE, PIN_DS1302_IO, PIN_DS1302_SCLK);
}
enum {
  CLOCK_DS1302,   // DS1302を基準の時計として使用
  CLOCK_INTERNAL  // 内蔵RTCを基準の時計として使用
};
int clockMode = CLOCK_DS1302;

// タイマー関係
// https://github.com/thomasfredericks/Metro-Arduino-Wiring
// Metro-Arduino-Wiring-master.zip
#include <Metro.h>
Metro offTimerInterval = Metro(60000);  // タイマー用変数増加間隔(ms) 60000=1分
//単位は3つとも「分」
int screenOffTimer;                // 画面自動消灯用加算タイマー
int screenOffTimerInBlackout;      // 停電時画面自動消灯用加算タイマー
volatile int popupWindowOffTimer;  // popupWindow自動キャンセル用加算タイマー
const int SCREENOFFTIMER_START = 0;
const int SCREENOFFTIMERINBLACKOUT_STOP = -1;
const int SCREENOFFTIMERINBLACKOUT_START = 0;
const int SCREENOFFTIMERINBLACKOUT_END = 3;
const int POPUPWINDOWOFFTIMER_START = 0;
const int POPUPWINDOWOFFTIMER_END = 2;

// タイマー割り込み関係
// https://github.com/PaulStoffregen/TimerOne
#include <TimerOne.h>
//タイマー
//Timer0 delay()等で使用
//Timer1 TimerOneライブラリで使用
//Temwr2 tone()で使用

// mighty-1284p-master\variants\bobuino\pins_arduino.h
//  TIMER1A D8  (PD5)pin.19 -> LCD CS
//  TIMER1B D30 (PD4)pin.18 -> LCD RESET
volatile int timer1_status;  //状態制御用変数
enum {
  TIMER1_STS_CALCULATE,  //計測中、計算中
  TIMER1_STS_IDLE,       //画面更新可能
  TIMER1_STS_PRINT,      //画面更新中
};

// LCD関係
// http://www.rinkydinkelectronics.com/library.php?id=51
// UTFT.zip (UTFT_VERSION  282)(UTFT_VERSION  283)
// http://www.rinkydinkelectronics.com/library.php?id=92
// URTouch.zip
// 空きメモリーを増やすために、memorysaver.h を修正
// C:\Users\user\Documents\Arduino\libraries\UTFT\memorysaver.h 修正
// Aitendo LCD(M032C1289TP) -> //#define DISABLE_SSD1289 以外の行をアンコメント(//を削除して有効化)
// AliExpress LCD -> //#define DISABLE_ILI9341_8 以外の行をアンコメント(//を削除して有効化)
#include <memorysaver.h>
#include <UTFT.h>
#include <URTouch.h>
#include <URTouchCD.h>
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
// http://www.rinkydinkelectronics.com/r_fonts.php
//extern uint8_t SixteenSegment32x48[];
extern uint8_t GroteskBold32x64[];
extern uint8_t SevenSeg_XXXL_Num[];

// Aitendo 3.2inch LCD(M032C1289TP)(動作電圧：3.3V)(320 * 240) 8ビットモードで使用
/*  ピン配置
  LCD        Arduino         LCD          Arduino
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
//DB0～DB7はD0～D7に固定

// AliExpress 3.2inch LCD(ILI9341)(動作電圧：3.3V)(320 * 240) 8ビットモードで使用
/*  ピン配置
  LCD        Arduino         LCD          Arduino
  1 CS    -> 19 (D8)          18 DB12    ->  1 (D4)
  2 RS    -> 21 (D31)         19 DB13    ->  2 (D5)
  3 WR    -> 20 (D9)          20 DB14    ->  3 (D6)
  4 RD    -> VDD              21 DB15    ->  4 (D7)
  5 RST   -> 18 (D30)         22 SDCS
  6 DB0                       23 BL      -> 2SC1815 Corrector
  7 DB1                       24 VDD     -> VDD
  8 DB2                       25 VDD
  9 DB3                       26 GND     -> GND
  10 DB4                      27 GND
  11 DB5                      28 nc
  12 DB6                      29 MISO    -> 23 (D24)
  13 DB7                      30 MOSI    -> 24 (D23)
  14 DB8  -> 14 (D0)          31 PEN     -> 22 (D22)
  15 DB9  -> 15 (D1)          32 F_CS
  16 DB10 -> 16 (D2)          33 T_CS    -> 25 (D25)
  17 DB11 -> 17 (D3)          34 CLK     -> 26 (D26)
*/
//DB8～DB15はD0～D7に固定
const int PIN_LCD_LED = 20;  //バックライトの電源制御用
const int PIN_LCD_RS = 31;
const int PIN_LCD_WR = 9;
const int PIN_LCD_CS = 8;
const int PIN_LCD_RSET = 30;
//UTFT myGLCD(SSD1289_8, PIN_LCD_RS, PIN_LCD_WR, PIN_LCD_CS, PIN_LCD_RSET);
UTFT myGLCD(ILI9341_8, PIN_LCD_RS, PIN_LCD_WR, PIN_LCD_CS, PIN_LCD_RSET);
const int PIN_TP_CLK = 26;
const int PIN_TP_CS = 25;
const int PIN_TP_DIN = 24;
const int PIN_TP_OUT = 23;
const int PIN_TP_PENIRQ = 22;
URTouch myTouch(PIN_TP_CLK, PIN_TP_CS, PIN_TP_DIN, PIN_TP_OUT, PIN_TP_PENIRQ);

// メインウィンドウの状態
int currScreenMode;
int curX, curY;
enum {
  SCREEN_SETUP,   // 初期化処理中モード
  SCREEN_ON,      // 画面表示状態/再表示指示(printMainWindow(SCREEN_ON)を実行することでバックライトのLEDをオンにして再表示する)
  SCREEN_BLANK,   // 画面非表示状態/非表示指示(printMainWindow(SCREEN_BLANK)を実行することでバックライトのLEDをオフにして非表示にする)
  SCREEN_REDRAW,  // 画面再描画指示モード
  SCREEN_CLOCK,   // 時計表示モード
  SCREEN_POPUP,   // popupWindow表示中のモード
  SCREEN_CURRENT  // 現在の画面モードを指定するためのモード
};
// printStringExEx()関係
const long VGA_NO_CHANGE = VGA_TRANSPARENT;
enum {
  FONT_NO_CHANGE,
  FONT_SMALL_FONT,
  FONT_BIG_FONT,
  FONT_SEVEN_SEG_NUM_FONT,
  FONT_SIXTEEN_SEGMENT_32X48,
  FONT_GROTESK_BOLD_32X64,
  FONT_SEVEN_SEG_XXXL_NUM
};
// ボタン・行の状態(選択/非選択)
enum {
  STS_NORMAL,  // 非選択
  STS_INVERSE  // 選択
};
// ボタン番号
enum {
  BTN_NOT_PRESSED,
  BTN_PRESSED,
  BTN_SCREEN_ON,
  BTN_RESET,
  BTN_DATE_TIME,
  BTN_CONFIG,
  BTN_SD_CARD,
  BTN_SCREEN_OFF,
  BTN_RESET_SYSTEM_RESET,
  BTN_RESET_OK,
  BTN_RESET_CANCEL,
  BTN_SYSTEM_RESET_OK,
  BTN_SYSTEM_RESET_CANCEL,
  BTN_DATE_TIME_UP,
  BTN_DATE_TIME_DOWN,
  BTN_DATE_TIME_PLUS,
  BTN_DATE_TIME_MINUS,
  BTN_DATE_TIME_SYNC,
  BTN_DATE_TIME_OK,
  BTN_DATE_TIME_CANCEL,
  BTN_CONFIG_UP,
  BTN_CONFIG_DOWN,
  BTN_CONFIG_PLUS,
  BTN_CONFIG_MINUS,
  BTN_CONFIG_LOAD_DEFAULT,
  BTN_CONFIG_OK,
  BTN_CONFIG_CANCEL,
  BTN_SD_CARD_PAGE_UP,
  BTN_SD_CARD_CURSOR_UP,
  BTN_SD_CARD_CURSOR_DOWN,
  BTN_SD_CARD_PAGE_DOWN,
  BTN_SD_CARD_DELETE_ALL,
  BTN_SD_CARD_DELETE,
  BTN_SD_CARD_CANCEL,
  BTN_DELETE_FILE_OK,
  BTN_DELETE_FILE_CANCEL
};
struct BTN {
  char name1[8];  //ボタンに表示する文字列の1行目の最大文字数+1('\0')
  char name2[8];  //ボタンに表示する文字列の2行目の最大文字数+1('\0')
  int x1;         //ボタンの左上のx座標
  int y1;         //ボタンの左上のy座標
  int x2;         //ボタンの右下のx座標
  int y2;         //ボタンの右下のy座標
};
const struct BTN btn[] = {
  { "", "", 0, 0, 0, 0 },
  { "", "", 0, 0, 0, 0 },
  { "", "", 0, 0, 0, 0 },
  { "RESET", "", 0, 209, 60, 239 },
  { "DATE", "TIME", 64, 209, 124, 239 },
  { "CONFIG", "", 128, 209, 188, 239 },
  { "SD", "CARD", 192, 209, 252, 239 },
  { "SCREEN", "OFF", 256, 209, 316, 239 },
  // RESET
  { "SYSTEM", "RESET", 40, 124, 104, 152 },
  { "OK", "", 144, 124, 208, 152 },
  { "CANCEL", "", 216, 124, 280, 152 },
  // SYSTEM RESET
  { "OK", "", 144, 124, 208, 152 },
  { "CANCEL", "", 216, 124, 280, 152 },
  // DATE TIME
  { "UP", "", 224, 40, 264, 68 },
  { "DOWN", "", 224, 110, 264, 138 },
  { " + ", "", 248, 75, 288, 103 },
  { " - ", "", 200, 75, 240, 103 },
  { "SYNC", "", 48, 152, 112, 180 },
  { "OK", "", 152, 152, 216, 180 },
  { "CANCEL", "", 224, 152, 288, 180 },
  // CONFIG
  { "UP", "", 240, 40, 280, 68 },
  { "DOWN", "", 240, 110, 280, 138 },
  { " + ", "", 264, 75, 304, 103 },
  { " - ", "", 216, 75, 256, 103 },
  { "LOAD", "DEFAULT", 108, 194, 168, 222 },
  { "OK", "", 176, 194, 236, 222 },
  { "CANCEL", "", 244, 194, 304, 222 },
  // SD CARD
  { "PAGE", "UP", 232, 27, 288, 53 },
  { "CURSOR", "UP", 232, 57, 288, 83 },
  { "CURSOR", "DOWN", 232, 87, 288, 113 },
  { "PAGE", "DOWN", 232, 117, 288, 143 },
  { "DELETE", "ALL", 80, 180, 144, 208 },
  { "DELETE", "", 152, 180, 216, 208 },
  { "CANCEL", "", 224, 180, 288, 208 },
  // DELETE FILE
  { "OK", "", 144, 124, 208, 152 },
  { "CANCEL", "", 216, 124, 280, 152 },
};
//CONFIGの設定項目表示用のパラメータを保持する
struct ITEM {
  char *name;    //項目名
  byte value;    //項目の値
  char *format;  //値の書式
  int x;         //表示の左上のx座標
  int y;         //表示の左上のy座標
};

//デフォルト引数のある関数の宣言
void printStringExEx(char *s1, char *s2, unsigned int digit, int x, int y, int font, unsigned long c = VGA_NO_CHANGE);
void printBarGraphFrame(int screenMode = SCREEN_CURRENT);
void printBarGraph(int screenMode = SCREEN_CURRENT);
void printSyncTimeInfo(int screenMode = SCREEN_CURRENT);
void printElapsedDays(int screenMode = SCREEN_CURRENT);
void printWattHourTimeperiod(int screenMode = SCREEN_CURRENT);
void printClock(int screenMode = SCREEN_CLOCK);
void clearScreen(int ms = 1000);  // ms: 画面クリアまでの待ち時間(ミリ秒)

// 電力量関係
// 端子定義
const int PIN_AREF = 0;     // 基準の電圧(電源電圧の中点=1.65V)
const int PIN_VOLTAGE = 1;  // 電圧計測用
const int PIN_SENSOR1 = 2;  // 電流センサー No.1
const int PIN_SENSOR2 = 3;  // 電流センサー No.2
const int PIN_SENSOR3 = 4;  // 電流センサー No.3
const int PIN_SENSOR[4] = { 0, PIN_SENSOR1, PIN_SENSOR2, PIN_SENSOR3 };
// マイコンの電源電圧(V)
const float VCC = 3.3;
// 実効電圧、実効電流、有効電力
const unsigned int NUMBER_OF_SAMPLES = 25;                                                   // １サイクルあたりのサンプル数
unsigned long SAMPLING_PERIOD = 1000000L / (POWER_FREQ * (unsigned long)NUMBER_OF_SAMPLES);  // サンプリング間隔(マイクロ秒)
int VASamples[NUMBER_OF_SAMPLES * 4];                                                        // サンプリング用バッファ
const float KVT = VT_CONVERSION_RATIO;                                                       // VTの出力電圧から実際の電圧に換算する比率
// CTの出力電圧から実際の電流に換算する比率
// CT負荷抵抗 * 係数 / 巻き数
const float KCT1 = CT1_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT2 = CT2_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT3 = CT3_LOAD_RESISTANCE * 1.16 / 3000.0;
const float KCT[4] = { 0, KCT1, KCT2, KCT3 };
volatile float vrms[4];
volatile float irms[4];
volatile float irms_max[4];
volatile float watt[4];
volatile float watt_max[4];
int prevLevel;  // バーグラフの前回指示値を格納
volatile int watt_samples;
volatile float watt_hour, watt_hour1, watt_hour2, watt_hour3;
volatile float watt_sum, watt_sum1, watt_sum2, watt_sum3;
volatile float wattHourTimeperiod1, wattHourTimeperiod2, wattHourTimeperiod3, wattHourTimeperiod4;
volatile float wattHourTimeperiod;
// 積算値リセット関係
char resetTime[BUFFER_LEN_HALF];
int elapsedDays;                            //リセット日からの経過日数+1
boolean needsSaveIntegratedValues = false;  // trueの時、積算値をリセットする
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
int prevDate;
int prevSec;
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
const int PIN_SDCARD_CS = 10;
const int PIN_SS = 10;               // 変更不可
const int PIN_SDCARD_VDD = A5;       // 電源制御
const int MAX_DATASTRING_LEN = 501;  // SDカードへ１回に書き込むデータの最大サイズ(500文字 + '\0'(1文字) = 501)
const int MAX_FILE_NAME_LEN = 13;    // ファイル名 8+3形式(12桁)+'\0'
const int MAX_FILE_SIZE_LEN = 11;    // long型変数の最大桁数(10桁)+'\0'
// (SDカードの状態)
int sdSts;
const int SD_CARD_PRESENT = 0;  // SDカードが挿入されている
//SDカードの状態を表す定数　※ここを変更する場合は、この下のsdStsMsg配列も変更すること
enum {
  SD_SUCCESS,
  SD_CARD_NOT_PRESENT,  // SDカードが挿入されていない
  SD_OPEN_ERROR,        // ファイルのオープン失敗
  SD_WRITING,           // ファイル書き込み中
  SD_NO_FILES,          // ファイルが見つからない
  SD_WRITE_ERROR,       // ファイルの書き込み失敗
  SD_STS_MSG_BLANK,     // メッセージ消去用
};
//SDカードの状態を表す定数に対応する文字列
const char sdStsMsg[][17] = {
  "Succeeded       ",
  "Card not present",
  "Card open error ",
  "Writing to card ",
  "File not found  ",
  "Card write error",
  "                ",
};

// (ファイル書き込み関係)
char sdLastUpdatedTime[BUFFER_LEN_HALF];
// ファイル一覧作成のための変数
struct FILELIST {
  int num;         // 通し番号(1～)
  char attribute;  // ファイル属性(' ':ルートファイル、'+':ルートファイル以外)
  char name[MAX_FILE_NAME_LEN];
  char size[MAX_FILE_SIZE_LEN];
};
struct FILELIST **fileList;         // SD内のファイル情報を格納する変数
int numberOfFiles;                  // SD内のファイル数
uint32_t fileUsage;                 // SD内のファイルサイズの合計
const int FILENUM_DELETE_ALL = -1;  // SD CARD画面で[DELETE ALL]を押した時にdeleteFile()関数に渡す引数(任意の負数)
// printMode(saveEventToSdCard()内で使用する画面上の文字列表示の指定)
enum {
  PRINTMODE_WINDOW,  // 指定位置に表示
  PRINTMODE_LINE,    // 現在のカーソル位置に表示
  PRINTMODE_NOPRINT  // 表示しない
};
//デフォルト引数のある関数の宣言
void saveEventToSdCard(String event, int screenMode = SCREEN_CURRENT, int printMode = PRINTMODE_WINDOW);
void saveDataToSdCard(int sccreenMode = SCREEN_CURRENT);
void printSdStatus(int screenMode = SCREEN_CURRENT, int printMode = PRINTMODE_WINDOW);
void printResetTime(int screenMode = SCREEN_CURRENT);

// FlashAir関係
// https://flashair-developers.com/ja/ -> 閉鎖
// https://flashair-developers.github.io/website/
#include "iSdio.h"
//#include "utility/Sd2CardExt.h"
#include "Sd2CardExt.h"
Sd2CardExt card;
uint8_t iSDIO_buf[512];
uint32_t nextSequenceId = 0;
char jst[] = "----/--/-- --:--:--";
int syncDay, syncHour, syncMin;
//boolean syncTime(int provider = DEFAULT_PROVIDER);
char httpCommandArg1[30];
char httpCommandArg2[250];
boolean isTimeSynchronized;
boolean existsFlashAir = false;
char syncTimeInfo[] = "--/-- --:-- ----h ";
// requestFlashAirReset()実行(FlashAIrリセット要求)時、syncTimeInfo[RESET_FLASHAIR_REQUEST_MARK]を'R'にセット
// resetFlashAir()実行(FlashAirリセット処理)時、syncTimeInfo[RESET_FLASHAIR_REQUEST_MARK]を' 'にセット
const int RESET_FLASHAIR_REQUEST_MARK = 17;
boolean isResetFlashAirRequested;  // trueの時、次の時刻取得の際にFlashAirをリセットする
const int PROTOCOL_HTTP = 0x21;
const int PROTOCOL_HTTPS = 0x23;
const int MAX_HTTP_RESPONSE_LEN = 2048 - 24;
unsigned long prevMillis;
char flashAirIP[16];
//デフォルト引数のある関数の宣言
void resetFlashAir(int sec = 5);
void onOffFlashAir(int sec = 5);
boolean iSDIO_sendHttpRequest(uint32_t sequenceId, int protocol = PROTOCOL_HTTP);

// 圧電スピーカー関係
const int PIN_SPEAKER = 21;
enum {
  BEEP_OFF,
  BEEP_ON,
  BEEP_LOW,
  BEEP_HIGH,
  BEEP_PIPO,
  BEEP_800MS
};
// タイマーを使用しないtone代替関数ライブラリ
// https://bitbucket.org/teckel12/arduino-timer-free-tone/wiki/Home
#include <TimerFreeTone.h>

// 文字列操作関数
//デフォルト引数のある関数の宣言
//indexVal -1:valの位置を指定しない(デフォルト)、0以上:valの位置を指定
boolean findLine(String str, const char *val, char *line, size_t lineSize, int indexVal = -1);
void printlnEx(String s = "");

// ************************************************************************************
// FLASH AIR制御関数
boolean iSDIO_status() {
  char buf[BUFFER_LEN_HALF];

  flashAirIP[0] = '\0';
  printlnEx(F("Read iSDIO Status Register"));
  // Read iSDIO Status Register (E7 1.10 2.2.2.1)
  memset(iSDIO_buf, 0, 0x200);
  if (!card.readExtMemory(1, 1, 0x400, 0x200, iSDIO_buf)) {
    return false;
  }
  // Show values in the common status area.
  printlnEx(F(" == iSDIO Status Registers == "));
  printEx(F(" [0400h] Command Write Status: "));
  if (iSDIO_buf[0x000] & 0x01) printEx(F("CWU "));
  if (iSDIO_buf[0x000] & 0x02) printEx(F("CWA "));
  printlnEx();
  printEx(F(" [0420h] iSDIO Status: "));
  if (iSDIO_buf[0x020] & 0x01) printEx(F("CRU "));
  if (iSDIO_buf[0x020] & 0x02) printEx(F("ESU "));
  if (iSDIO_buf[0x020] & 0x04) printEx(F("MCU "));
  if (iSDIO_buf[0x020] & 0x08) printEx(F("ASU "));
  printlnEx();
  printEx(F(" [0422h] iSDIO Int Enable: "));
  if (iSDIO_buf[0x022] & 0x01) printEx(F("CRU_ENA "));
  if (iSDIO_buf[0x022] & 0x02) printEx(F("ESU_ENA "));
  if (iSDIO_buf[0x022] & 0x04) printEx(F("MCU_ENA "));
  if (iSDIO_buf[0x022] & 0x08) printEx(F("ASU_ENA "));
  printlnEx();
  printEx(F(" [0424h] Error Status: "));
  if (iSDIO_buf[0x024] & 0x01) printEx(F("CRE "));
  if (iSDIO_buf[0x024] & 0x02) printEx(F("CWE "));
  if (iSDIO_buf[0x024] & 0x04) printEx(F("RRE "));
  if (iSDIO_buf[0x024] & 0x08) printEx(F("APE "));
  printlnEx();
  printEx(F(" [0426h] Memory Status: "));
  if (iSDIO_buf[0x026] & 0x01) printEx(F("MEX "));
  if (iSDIO_buf[0x026] & 0x02) printEx(F("FAT "));
  printlnEx();
  for (int i = 0; i < 8; ++i) {
    uint8_t addr = 0x40 + i * 0x14;
    printEx(F(" [04"));
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
        case 0x00:
          printlnEx();
          printEx(F("Initial"));
          break;
        case 0x01:
          printlnEx();
          printEx(F("Command Processing"));
          break;
        case 0x02:
          printlnEx();
          printEx(F("Command Rejected"));
          break;
        case 0x03:
          printlnEx();
          printEx(F("Process Succeeded"));
          break;
        case 0x04:
          printlnEx();
          printEx(F("Process Terminated"));
          break;
        default:
          printlnEx();
          printEx(F("Process Failed "));
          sprintf(buf, " % X", iSDIO_buf[addr + 8]);
          printlnEx(buf);
          break;
      }
    } else {
      printlnEx();
      printEx(F("Not registered"));
    }
  }
  printlnEx();
  // Show values in the application status area.
  printlnEx(F(" == Wireless LAN Status Registers == "));
  printEx(F(" [0500h] DLNA Status: "));
  if (iSDIO_buf[0x100] & 0x01) printEx(F("ULR "));
  if (iSDIO_buf[0x100] & 0x02) printEx(F("DLU "));
  if (iSDIO_buf[0x100] & 0x04) printEx(F("CBR "));
  if (iSDIO_buf[0x100] & 0x08) printEx(F("CDR "));
  printlnEx();
  printEx(F(" [0501h] P2P Status: "));
  if (iSDIO_buf[0x101] & 0x01) printEx(F("ILU "));
  if (iSDIO_buf[0x101] & 0x02) printEx(F("FLU "));
  printlnEx();
  printEx(F(" [0502h] PTP Status: "));
  if (iSDIO_buf[0x102] & 0x01) printEx(F("RPO "));
  if (iSDIO_buf[0x102] & 0x02) printEx(F("RPD "));
  if (iSDIO_buf[0x102] & 0x04) printEx(F("RPC "));
  if (iSDIO_buf[0x102] & 0x08) printEx(F("CPI "));
  if (iSDIO_buf[0x102] & 0x10) printEx(F("DPI "));
  if (iSDIO_buf[0x102] & 0x20) printEx(F("CIL "));
  printlnEx();
  printEx(F(" [0504h] Application: "));
  printlnEx(String((char)iSDIO_buf[0x104]));
  printEx(F(" [0506h] WLAN: "));
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
  if ((iSDIO_buf[0x106] & 0x60) == 0x40) printEx(F("Wi - Fi Direct, "));
  if ((iSDIO_buf[0x106] & 0x80) == 0x00) printEx(F("No Connection, "));
  if ((iSDIO_buf[0x106] & 0x80) == 0x80) printEx(F("Connected, "));
  printlnEx();
  printEx(F(" [0508h] SSID: "));
  for (int i = 0; i < 32 && iSDIO_buf[0x108 + i] != 0; ++i) {
    printEx(String((char)iSDIO_buf[0x108 + i]));
  }
  printlnEx();
  printEx(F(" [0528h] Encryption Mode: "));
  switch (iSDIO_buf[0x128]) {
    case 0: printEx(F("Open System and no encryption")); break;
    case 1: printEx(F("Open System and WEP")); break;
    case 2: printEx(F("Shared Key and WEP")); break;
    case 3: printEx(F("WPA - PSK and TKIP")); break;
    case 4: printEx(F("WPA - PSK and AES")); break;
    case 5: printEx(F("WPA2 - PSK and TKIP")); break;
    case 6: printEx(F("WPA2 - PSK and AES")); break;
    default: printEx(F("Unknown"));
  }
  printlnEx();
  printEx(F(" [0529h] Signal Strength: "));
  sprintf(buf, "%d", iSDIO_buf[0x129]);
  printlnEx(buf);
  printEx(F(" [052Ah] Channel: "));
  if (iSDIO_buf[0x12A] == 0) {
    printlnEx(F("No connection"));
  } else {
    sprintf(buf, "%d", iSDIO_buf[0x12A]);
    printlnEx(buf);
  }
  printEx(F(" [0530h] MAC Address: "));
  printBytes(iSDIO_buf + 0x130, 6);
  printlnEx();
  printEx(F(" [0540h] ID: "));
  for (int i = 0; i < 16 && iSDIO_buf[0x140 + i] != 0; ++i) {
    printEx(String((char)iSDIO_buf[0x140 + i]));
  }
  printlnEx();
  printEx(F(" [0550h] IP Address: "));
  //printIPAddress(iSDIO_buf + 0x150);
  convIPAddressToString(iSDIO_buf + 0x150, flashAirIP);
  printlnEx(flashAirIP);
  printEx(F(" [0554h] Subnet Mask: "));
  printIPAddress(iSDIO_buf + 0x154);
  printlnEx();
  printEx(F(" [0558h] Default Gateway: "));
  printIPAddress(iSDIO_buf + 0x158);
  printlnEx();
  printEx(F(" [055Ch] Preferred DNS Server: "));
  printIPAddress(iSDIO_buf + 0x15C);
  printlnEx();
  printEx(F(" [0560h] Alternate DNS Server: "));
  printIPAddress(iSDIO_buf + 0x160);
  printlnEx();
  printEx(F(" [0564h] Proxy Server: "));
  if ((iSDIO_buf[0x164] & 0x01) == 0x00) printEx(F("Disabled"));
  if ((iSDIO_buf[0x164] & 0x01) == 0x01) printEx(F("Enabled"));
  printlnEx();
  printEx(F(" [0570h] Date: "));
  sprintf(buf, "%04d-%02d-%02d", iSDIO_buf[0x171] + 1980, iSDIO_buf[0x170] >> 4, iSDIO_buf[0x170] & 0xF);
  printlnEx(buf);
  printEx(F(" [0572h] Time: "));
  sprintf(buf, "%02d:%02d:%02d", iSDIO_buf[0x173] >> 3, iSDIO_buf[0x172] << 3 | iSDIO_buf[0x170] >> 3, (iSDIO_buf[0x172] & 0x1F) * 2);
  printlnEx(buf);
  printEx(F(" [0574h] HTTP Status: "));
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
  if (strcmp(flashAirIP, "192.168.0.1") == 0) {
    return false;
  }
  if (strcmp(flashAirIP, "0.0.0.0") == 0) {
    return false;
  }
  return true;
}

boolean iSDIO_sendHttpRequest(uint32_t sequenceId, int protocol) {
  char buf[BUFFER_LEN_HALF];

  clearScreen();
  printlnEx(F("sending HTTP Request..."));
  memset(iSDIO_buf, 0, 512);
  uint8_t *p = iSDIO_buf;
  p = put_command_header(p, 1, 0);
  p = put_command_info_header(p, protocol, sequenceId, 2);
  // put_command_info_header()関数の第2引数(protocol)について
  // protocol=PROTOCOL_HTTPS
  //  NICTは問題なし、World Time APIは card.readExtMemory()は成功してもレスポンスがエラー(エラーコード:C0)のため不可
  // protocol=PROTOCOL_HTTP
  //  どちらも問題なし
  // PROTOCOL_HTTPSとPROTOCOL_HTTPの違い
  //  PROTOCOL_HTTPS  コマンドID:23h コマンド名称:SendHTTPSSLMessageByRegister(プロトコル:HTTP over SSL)
  //  PROTOCOL_HTTP   コマンドID:21h コマンド名称:SendHTTPMessageByRegister(プロトコル:HTTP)
  p = put_str_arg(p, httpCommandArg1);  // Argument #1.
  p = put_str_arg(p, httpCommandArg2);  // Argument #2.
  put_command_header(iSDIO_buf, 1, (p - iSDIO_buf));
  //printHex(iSDIO_buf, (p - iSDIO_buf));
  printlnEx(httpCommandArg1);
  printEx(httpCommandArg2);
  // ここで10秒以上待たないとレスポンスが返ってこない
  int waitingTime = 10;
  sprintf(buf, "Waiting %d seconds", waitingTime);
  printEx(buf);
  for (int i = 0; i < waitingTime; i++) {
    printEx(F("."));
    delay(1000);
  }
  printlnEx();
  if (card.writeExtDataPort(1, 1, 0x000, iSDIO_buf) == false) {
    printlnEx(F("writeExtDataPort command Failed"));
    return false;
  }
  printlnEx(F("writeExtDataPort command Succeeded"));
  return true;
}

boolean iSDIO_waitHttpResponse(uint32_t sequenceId) {
  char buf[BUFFER_LEN_HALF];

  printEx(F("Waiting response"));
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
            printlnEx();
            printEx(F("  Initial"));
            break;
          case 0x01:
            printlnEx();
            printEx(F("  Command Processing"));
            break;
          case 0x02:
            printlnEx();
            printlnEx(F("  Command Rejected"));
            return false;
          case 0x03:
            printlnEx();
            printlnEx(F("  Process Succeeded"));
            return true;
          case 0x04:
            printlnEx();
            printlnEx(F("  Process Terminated"));
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
            printlnEx();
            printEx(F("  Process Failed "));
            sprintf(buf, "%X", resp);
            printlnEx(buf);
            return false;
        }
        prev = resp;
      }
    }
    printEx(F("."));
    delay(1000);
  }
  printlnEx();
  printlnEx(F("  Process time out"));
  return false;
}

boolean iSDIO_readHttpResponse(char *httpResponse, int httpReponseSize) {
  char responseCode[100];
  int statusNum;
  //char statusMsg[100];
  char *statusMsg;
  char event[BUFFER_LEN_EVENT];
  char buf[BUFFER_LEN_HALF];

  printlnEx(F("reading HTTP response..."));
  httpResponse[0] = '\0';
  // Read header and data.
  if (!card.readExtDataPort(1, 1, 0x200, iSDIO_buf)) {
    printlnEx(F("readExtDataPort command(Read header and data) failed"));
    return false;
  }
  uint32_t totalSize = get_u32(iSDIO_buf + 20);
  if (totalSize == 0) {
    printlnEx(F("readExtDataPort totalData==0"));
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
      printlnEx(F("readExtDataPort command(Read next data) failed"));
      delay(3000);
      return false;
    }
    availableSize = totalSize > 512 ? 512 : totalSize;
    pos = 0;
  }
  httpResponse[j] = '\0';
  printlnEx(httpResponse);
  if (j == (httpReponseSize - 1)) {
    saveEventToSdCard(F("SYSTEM: FlashAir httpResponseSize not enough"), currScreenMode, PRINTMODE_LINE);
    return false;
  }
  if (findLine(httpResponse, "HTTP/1.1", responseCode, sizeof(responseCode)) == false) {
    saveEventToSdCard(F("SYSTEM: FlashAir HTTP response code not found"), currScreenMode, PRINTMODE_LINE);
    return false;
  }
  strtok(responseCode, " ");
  statusNum = atoi(strtok(NULL, " "));
  statusMsg = strtok(NULL, "\n");
  trimCrLf(statusMsg);
  if (statusNum != 200) {
    sprintf(event, "SYSTEM: FlashAir HTTP response error(status %d %s)", statusNum, statusMsg);
    saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
  }
  return true;
}

#ifdef PROVIDER_WORLD_CLOCK_API
boolean syncTime(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  char formattedHttpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, month, day, hour, minute, second;
  char datetime[40];
  char WdayName[4], monthName[4];
  const char monthNames[][4] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  const char WdayNames[][4] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  tmElements_t tm;

  isTimeSynchronized = false;
  // http://worldclockapi.com/api/json/utc/now
  for (numberOfTimes = 1; numberOfTimes <= MAX_NUMBER_OF_TIMES; numberOfTimes++) {
    httpResponse[0] = '\0';

    if (numberOfTimes == 1) {
      strcpy(httpCommandArg1, "worldclockapi.com");
      strcpy(httpCommandArg2,
             "GET /api/json/utc/now HTTP/1.1\r\n"
             "Host: worldclockapi.com\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
             "AppleWebKit/537.36 (KHTML, like Gecko) "
             "Chrome/133.0.0.0 "
             "Safari/537.36 "
             "Edg/133.0.0.0\r\n"
             "\r\n");
    } else {
      delay(3000);
      resetFlashAir();
    }
    if ((iSDIO_sendHttpRequest(nextSequenceId) && iSDIO_waitHttpResponse(nextSequenceId))) {
      if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
        isTimeSynchronized = true;
        nextSequenceId++;
        break;
      }
    }
    nextSequenceId++;
  }
  tm.Year = 0;
  tm.Month = 0;
  tm.Wday = 0;
  tm.Day = 0;
  tm.Hour = 0;
  tm.Minute = 0;
  tm.Second = 0;
  if (isTimeSynchronized == false) {
    printEx(F("Failed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printlnEx(buf);
    requestFlashAirReset();
  } else {
    //整形出力
    strcpy(formattedHttpResponse, httpResponse);
    replaceChar(formattedHttpResponse, '{', '\n');  // 左かっこを改行に置換
    replaceChar(formattedHttpResponse, ',', '\n');  // カンマを改行に置換
    replaceChar(formattedHttpResponse, '}', '\n');  // 右かっこを改行に置換

    // formattedHttpResponseから時刻情報のある行を取り出す
    // 正常時のレスポンス例
    // "currentDateTime":"2025-02-09T07: 02Z"
    // "currentFileTime":133835626846972638
    // currentDateTimeは秒の情報がないためcurrentFileTime(Windows時間(NTタイムエポック))(100ナノ秒単位)を使用
    //
    isTimeSynchronized = findLine(formattedHttpResponse, "\"currentFileTime\":", datetime, sizeof(datetime), 0);
    if (isTimeSynchronized == false) {
      isTimeSynchronized = findLine(httpResponse, "Date:", datetime, sizeof(datetime), 0);
      if (isTimeSynchronized == false) {
        printEx(F("Time information not found"));
        delay(3000);
        printlnEx();
      } else {
        // 503 Service Unavailable 時も日時情報を取得するよう追加
        // 502 Bad Gateway 時も同様に取得できることを確認
        // Date: Tue, 11 Feb 2025 13:54:50 GMT
        // Sscanf comma separated string and value
        // https://forum.arduino.cc/t/sscanf-comma-separated-string-and-value/919885/3
        sscanf(datetime, "Date: %[^,], %02d %3s %04d %02d:%02d:%02d GMT",
               WdayName, &day, monthName, &year, &hour, &minute, &second);
        //時差の9時間を加算する
        //TimeLib.h内に記述あり
        //#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year
        //#define  CalendarYrToTm(Y)   ((Y) - 1970)
        tm.Year = CalendarYrToTm(year);
        for (unsigned int i = 1; i <= 12; i++) {
          if (strcmp(monthNames[i], monthName) == 0) {
            tm.Month = i;
            break;
          }
        }
        for (unsigned int i = 1; i <= 7; i++) {
          if (strcmp(WdayNames[i], WdayName) == 0) {
            tm.Wday = i;
            break;
          }
        }
        tm.Day = day;
        tm.Hour = hour;
        tm.Minute = minute;
        tm.Second = second;
        time_t t = makeTime(tm);
        t += 32400UL;  // JST - GMT = 9時間 = 32400秒
        breakTime(t, tm);
        adjustTimeByJst(tmYearToCalendar(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
      }
    } else {
      sscanf(datetime, "\"currentFileTime\":%s", buf);

      // 100ナノ秒から秒単位にする
      // 10000000で割る
      buf[strlen(buf) - 7] = '\0';
      // 文字列からlong long型の数値に変換
      // long long型　8バイト(64ビット)
      char dh[9], dl[9];
      strncpy(dh, buf, strlen(buf) - 8);
      dh[strlen(buf) - 8] = '\0';
      strncpy(dl, buf + (strlen(buf) - 8), 9);
      long long fileTime;
      fileTime = (long long)atol(dh) * 100000000LL + (long long)atol(dl);

      // fileTimeを日時に変換
      // time_t型 = unsigned long型 4バイト(32ビット) ※2106年2月7日6時28分15秒（閏秒を考慮しない場合）まで表現可
      // - 11644473600LL=  Unixエポック(1970-01-01 00:00:00 UTC)とNTタイムエポック(1601-01-01 00:00:00 UTC)の差(秒)
      // + 32400LL =  JST - UTC = 9時間 = 32400秒
      time_t unixTime = (time_t)(fileTime - 11644473600LL + 32400LL);
      breakTime(unixTime, tm);
      adjustTimeByJst(tmYearToCalendar(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
    }
  }
  //tm.Monthがずれる時があるためファイルに記録する(2025/4/5)
  sprintf(datetime, "datetime: %04d/%02d/%02d(%3s) %02d:%02d:%02d",
          tmYearToCalendar(tm.Year), tm.Month, tm.Day, WdayNames[tm.Wday], tm.Hour, tm.Minute, tm.Second);
  saveHttpResponseToSdCard(httpResponse, datetime);

  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lXh ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isTimeSynchronized;
}
#endif

#ifdef PROVIDER_API_NINJAS
boolean syncTime(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, month, day, hour, minute, second;
  long microsecond;
  char datetime[60];

  isTimeSynchronized = false;
  // https://www.api-ninjas.com/api/worldtime
  for (numberOfTimes = 1; numberOfTimes <= MAX_NUMBER_OF_TIMES; numberOfTimes++) {
    if (numberOfTimes == 1) {
      strcpy(httpCommandArg1, "api.api-ninjas.com");
      //東京の経度緯度 lat=35.6828387&lon=139.7594549
      //宇都宮の経度緯度 lat=36.5521&lon=139.88
      strcpy(httpCommandArg2,
             "GET /v1/worldtime?lat=36.5521&lon=139.88&X-Api-Key=uLJYasLxNspvbOKBFNDDvQ==nAUYcm49p6Nc3S2J HTTP/1.1\r\n"
             "Host: api.api-ninjas.com\r\n"
             "\r\n");
    } else {
      resetFlashAir();
    }
    if ((iSDIO_sendHttpRequest(nextSequenceId, PROTOCOL_HTTPS) && iSDIO_waitHttpResponse(nextSequenceId))) {
      if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
        isTimeSynchronized = true;
        nextSequenceId++;
        break;
      }
    }
    nextSequenceId++;
  }
  if (isTimeSynchronized == false) {
    printEx(F("Failed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printlnEx(buf);
    requestFlashAirReset();
  } else {
    // httpResponseから時刻情報のある行を取り出す
    // "datetime": "2022-09-18 19:16:43"
    isTimeSynchronized = findLine(httpResponse, "\"datetime\":", datetime, sizeof(datetime), 0);
    if (isTimeSynchronized == false) {
      printlnEx(F("Time information not found"));
    } else {
      sscanf(datetime, "\"dateTime\": \"%04d-%02d-%02d %02d-%02d:%02d\"",
             &year, &month, &day, &hour, &minute, &second);
      adjustTimeByJst(year, month, day, hour, minute, second);
    }
  }
  // 2025/2/11現在、正常処理ならず
  // 処理結果をLCD画面で確認のため一時的に挿入
  abort();

  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lXh ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isTimeSynchronized;
}
#endif

#ifdef PROVIDER_TIME_API
boolean syncTime(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, month, day, hour, minute, second;
  long microsecond;
  char datetime[250];

  isTimeSynchronized = false;
  // https://www.timeapi.io/api/time/current/zone?timeZone=Asia%2FTokyo
  for (numberOfTimes = 1; numberOfTimes <= MAX_NUMBER_OF_TIMES; numberOfTimes++) {
    if (numberOfTimes == 1) {
      strcpy(httpCommandArg1, "www.timeapi.io");
      strcpy(httpCommandArg2,
             "GET /api/time/current/zone?timeZone=Asia%2FTokyo HTTP/1.1\r\n"
             "Host: www.timeapi.io\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/132.0.0.0 Safari/537.36 Edg/132.0.0.0\r\n"
             "accept: application/json\r\n"
             "\r\n");
    } else {
      delay(3000);
      resetFlashAir();
    }
    if ((iSDIO_sendHttpRequest(nextSequenceId, PROTOCOL_HTTPS) && iSDIO_waitHttpResponse(nextSequenceId))) {
      if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
        isTimeSynchronized = true;
        nextSequenceId++;
        break;
      }
    }
    nextSequenceId++;
  }
  if (isTimeSynchronized == false) {
    printEx(F("Failed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printlnEx(buf);
    requestFlashAirReset();
  } else {
    replaceChar(httpResponse, ',', '\n');  // カンマを改行に置換
    // httpResponseから時刻情報のある行を取り出す
    // "dateTime":"2025-02-07T03:31:46.9305341"
    isTimeSynchronized = findLine(httpResponse, "\"datetime\":", datetime, sizeof(datetime), 0);
    if (isTimeSynchronized == false) {
      printEx(F("Time information not found"));
      delay(3000);
      printlnEx();
    } else {
      sscanf(datetime, "\"dateTime\":\"%04d-%02d-%02dT%02d-%02d:%02d.%07\"",
             &year, &month, &day, &hour, &minute, &second, &microsecond);
      // microsecondの値は使わない
      adjustTimeByJst(year, month, day, hour, minute, second);
    }
  }
  // 2025/2/11現在、正常処理ならず
  // 処理結果をLCD画面で確認のため一時的に挿入
  abort();

  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lXh ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isTimeSynchronized;
}
#endif

#ifdef PROVIDER_WORLD_TIME_API
boolean syncTime(void) {
  char buf[BUFFER_LEN_HALF];
  int numberOfTimes;
  const int MAX_NUMBER_OF_TIMES = 2;
  char httpResponse[MAX_HTTP_RESPONSE_LEN + 1];
  int year, month, day, hour, minute, second;
  long microsecond;
  char datetime[60];

  isTimeSynchronized = false;
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
      if ((iSDIO_sendHttpRequest(nextSequenceId) && iSDIO_waitHttpResponse(nextSequenceId))) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isTimeSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    } else {
      delay(3000);
      resetFlashAir();
      if (iSDIO_sendHttpRequest(nextSequenceId)
          && iSDIO_waitHttpResponse(nextSequenceId)) {
        if (iSDIO_readHttpResponse(httpResponse, sizeof(httpResponse))) {
          isTimeSynchronized = true;
          nextSequenceId++;
          break;
        }
      }
      nextSequenceId++;
    }
  }
  if (isTimeSynchronized == false) {
    printEx(F("Failed or waiting. errorCode="));
    sprintf(buf, "%X", card.errorCode());
    printlnEx(buf);
    requestFlashAirReset();
  } else {
    // httpResponseから時刻情報のある行を取り出す
    // datetime: 2021-01-23T20:38:02.410914+09:00 ('\0'含めて43文字)
    isTimeSynchronized = findLine(httpResponse, "datetime:", datetime, sizeof(datetime), 0);
    if (isTimeSynchronized == false) {
      printEx(F("Time information not found"));
      delay(3000);
      printlnEx();
    } else {
      //datetime: 2025-02-07T04:49:09.462195+09:00
      sscanf(datetime, "datetime: %04d-%02d-%02dT%02d:%02d:%02d.%06ld+09:00",
             &year, &month, &day, &hour, &minute, &second, &microsecond);
      // microsecondの値は使わない
      adjustTimeByJst(year, month, day, hour, minute, second);
    }
  }
  Time t = getClockTime();
  sprintf(syncTimeInfo, "%02d/%02d %02d:%02d %04lXh ", t.mon, t.date, t.hr, t.min, nextSequenceId - 1);
  return isTimeSynchronized;
}
#endif

// NICTによる時刻配信サービス停止期間は使用不可
#ifdef PROVIDER_NICT
//削除
#endif

void syncTimeOnceADay(void) {
  char event[BUFFER_LEN_EVENT];
  int prevScreenMode;

  if (hasBlackout) {
    return;
  }

  if (existsFlashAir == false) {
    return;
  }
  if (eepromAutoTimeSync == ON) {
    Time t = getClockTime();
    if ((t.date != syncDay) && (t.hr == eepromTimeSyncHour) && (t.min == eepromTimeSyncMin)) {
      syncDay = t.date;
      prevScreenMode = currScreenMode;
      turnOnScreen();
      if (isResetFlashAirRequested) {
        resetFlashAir();
      }
      if (syncTime() == true) {
        sprintf(event, "SYSTEM: [%04lXh]Time synchronized", nextSequenceId - 1);
        saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
      } else {
        sprintf(event, "SYSTEM: [%04lXh]Time synchronization failed", nextSequenceId - 1);
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

void onOffFlashAir(int sec) {
  if (existsFlashAir == false) {
    return false;
  }
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(VGA_WHITE);
  digitalWrite(PIN_SDCARD_VDD, LOW);
  printlnEx();
  printlnEx(F("FlashAir turned off"));
  delay(sec * 1000);
  digitalWrite(PIN_SDCARD_VDD, HIGH);
  printlnEx(F("FlashAir turned on"));
  delay(5000);
  printlnEx();
}

boolean initFlashAir(void) {
  // Initialize SD card.
  printEx(F("Initializing FlashAir..."));
  if (card.init(SPI_HALF_SPEED, PIN_SDCARD_CS)) {
    printlnEx(F("OK"));
  } else {
    printlnEx(F("NG"));
    printEx(F("If the screen touched, the system reset"));
    while (getButtonNum() == BTN_NOT_PRESSED)
      ;
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
    printlnEx();
    printEx(F("Failed to read FlashAir memory."));
    nextSequenceId = 0;
    saveEventToSdCard(F("SYSTEM: Failed to read FlashAir memory"), currScreenMode, PRINTMODE_LINE);
    delay(3000);
    return false;
  }

  if (!iSDIO_status()) {
    printlnEx();
    printEx(F("Failed to read FlashAir status."));
    saveEventToSdCard(F("SYSTEM: Failed to read FlashAir status"), currScreenMode, PRINTMODE_LINE);
    delay(3000);
    return false;
  }
  return true;
}

void requestFlashAirReset(void) {
  isResetFlashAirRequested = true;
  syncTimeInfo[RESET_FLASHAIR_REQUEST_MARK] = 'R';
  //popupWindow表示中は画面表示に関わる処理はしない
  //if (popupWindowOffTimer != POPUPWINDOWOFFTIMER_END) {
  //  return;
  //}
  if (currScreenMode == SCREEN_POPUP) {
    return;
  }
  if (currScreenMode == SCREEN_ON) {
    printSyncTimeInfo();
  }
}

void resetFlashAir(int sec) {
  if (existsFlashAir == false) {
    return false;
  }
  //clearScreen();
  onOffFlashAir(sec);
  memset(iSDIO_buf, 0, 512);
  nextSequenceId = 0;
  syncTimeInfo[RESET_FLASHAIR_REQUEST_MARK] = ' ';
  isResetFlashAirRequested = false;
  initFlashAir();
  saveEventToSdCard(F("SYSTEM: FlashAir reset"), currScreenMode, PRINTMODE_LINE);
}

// ************************************************************************************
// 文字列操作関数
// str内を検索しvalが含まれる行をlineにコピーする
boolean findLine(String str, const char *val, char *line, size_t lineSize, int indexVal) {
  unsigned int from = 0;
  int index, indexStart, indexEnd;

  while (1) {
    index = str.indexOf(val, from);
    if (index == -1) {
      return false;
    }
    indexStart = str.lastIndexOf("\n", index);
    if (indexStart == -1) {
      indexStart = 0;
    } else {
      indexStart++;
    }
    indexEnd = str.indexOf("\n", indexStart);
    if (indexEnd == -1) {
      indexEnd = str.length() + 1;
    }
    if ((indexEnd - indexStart) > lineSize) {
      return false;
    }
    str.substring(indexStart, indexEnd).toCharArray(line, lineSize);
    //valの位置を指定していない
    if (indexVal == -1) {
      return true;
    }
    //指定した位置にvalが見つかった
    if ((int)(strstr(line, val) - line) == indexVal) {
      return true;
    }
    //指定した位置にvalが見つからない
    //次を検索
    from = indexEnd;
  }
}

// 文字列内の改行を取り除く関数
//http://goldilocks-engineering.blogspot.com/2015/09/c.html
char *trimCrLf(char *str) {
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

// 文字列内の指定文字を置換する
void replaceChar(char *string, char find, char replace) {
  for (unsigned int pos = 0; pos < strlen(string); pos++) {
    if (string[pos] == find) {
      string[pos] = replace;
    }
  }
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

int getDayOfWeek(int y, int m, int d) {
  //Zellerの公式
  //1・2月は前年の13・14月として計算する
  if (m < 3) {
    y--;
    m += 12;
  }
  // 戻り値:曜日 -> 0:日曜日 1:月曜日 ... 6:土曜日
  return (y + y / 4 - y / 100 + y / 400 + (13 * m + 8) / 5 + d) % 7;
}

Time getClockTime(void) {
  if (clockMode == CLOCK_DS1302) {
    noInterrupts();
    Time t = ds1302.time();
    interrupts();
    return t;
  }
  tmElements_t tm;
  breakTime(now(), tm);
  //Time t(2013, 9, 22, 1, 38, 50, Time::kSunday);
  Time t(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second,
         Time::Day(getDayOfWeek(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day) + 1));
  return t;
}

void setClockTime(Time t) {
  if (clockMode == CLOCK_DS1302) {
    noInterrupts();
    ds1302.time(t);
    interrupts();
    return;
  }
  tmElements_t tm;
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
  if (delayMillis < 30000) {  // 30秒以上掛かっている場合は何か異常が発生しているものとして補正しない
    adjustTime(int(delayMillis / 1000) + 1);
  }
  breakTime(now(), tm);
  //Time t(2013, 9, 22, 1, 38, 50, Time::kSunday);
  Time t(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second,
         Time::Day(getDayOfWeek(uint16_t(tmYearToCalendar(tm.Year)), tm.Month, tm.Day) + 1));
  return t;
}

void adjustTimeByJst(int year, int month, int day, int hour, int minute, int second) {
  char buf[50];  // 時刻情報の行が格納できるサイズ

  if (year < 2025) {
    return;
  }
  clearScreen();
  // RTC
  Time t = getClockTime();
  sprintf(buf, "RTC: %04d/%02d/%02d %3s %02d:%02d:%02d",
          t.yr, t.mon, t.date, dayShortStr(t.day), t.hr, t.min, t.sec);
  printlnEx(buf);
  // JST
  t = adjustIntlClock(year, month, day, hour, minute, second);
  noInterrupts();
  ds1302.time(t);
  interrupts();
  // TimeLib.h > DateStrings.cpp > dayShortStr() 引数:戻り値 -> 0:Err 1:Sun 2:Mon ... 7:Sat
  sprintf(buf, "JST: %04d/%02d/%02d %3s %02d:%02d:%02d",
          t.yr, t.mon, t.date, dayShortStr(t.day), t.hr, t.min, t.sec);
  printlnEx(buf);
  sprintf(jst, "%02d/%02d/%02d %02d:%02d:%02d",
          t.yr % 100, t.mon, t.date, t.hr, t.min, t.sec);
  syncDay = t.date;
  syncHour = t.hr;
  syncMin = t.min;
  printlnEx(F("Synchronization succeeded"));
}

void adjustIntlClockByRtc(void) {
  if (clockMode == CLOCK_DS1302) {
    return;
  }
  // 内蔵RTCの精度が悪いため、内蔵RTCを基準の時計として使用時
  // 1分に1回(30秒の時)内蔵RTCの時刻をDS1302に合わせる
  Time t = getClockTime();
  if ((t.sec == 30) && ((t.min * 60 + t.sec) != prevSec)) {
    prevSec = t.min * 60 + t.sec;
    //割り込み許可の状態では、DS1302から帰ってくる時刻が異常な値になる
    noInterrupts();
    t = ds1302.time();
    interrupts();
    setClockTime(t);
  }
}

#define CalendarToTmYear(year) = ((year) + 1970)

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
      } else {
        delay(50);
      }
      delay(50);
      break;
    case BEEP_HIGH:
      if (eepromSound == ON) {
        tone(PIN_SPEAKER, 800, 50);
      } else {
        delay(50);
      }
      delay(50);
      break;
    case BEEP_PIPO:  // PC9800シリーズ起動音
      tone(PIN_SPEAKER, 2000, 150);
      delay(150);
      tone(PIN_SPEAKER, 1000, 150);
      delay(150);
      break;
    case BEEP_800MS:  // タイマー不使用の TimerFreeTone() を使用
      if (eepromSound == ON) {
        TimerFreeTone(PIN_SPEAKER, 800, 800);
      } else {
        delay(800);
      }
      delay(50);
      break;
    default:
      //何もしない
      break;
  }
}

// ************************************************************************************
// 電力量計算関数
void sampleFromSensor(int sensorNum) {
  unsigned long t1, t2;
  int r, v1, a1, a2, v2;
  float vv, aa;

  t1 = micros();
  for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
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
  for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
    v1 = VASamples[(i * 4) + 0];
    a1 = VASamples[(i * 4) + 1];
    a2 = VASamples[(i * 4) + 2];
    v2 = VASamples[(i * 4) + 3];
    vv = ((((v1 + v2) / 2) * VCC) / 1024) * KVT;
    vv = vv * (float)eepromVtPercentage / 100.0;  // 誤差補正
    aa = ((((a1 + a2) / 2) * VCC) / 1024) / KCT[sensorNum];
    aa = aa * (float)eepromCtPercentage[sensorNum] / 100.0;  // 誤差補正
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
    default:
      //何もしない
      break;
  }
  // 0.2A未満は測定誤差として切り捨て
  if (irms[sensorNum] < 0.2) {
    irms[sensorNum] = 0.0;
  }
  irms_max[sensorNum] = max(irms_max[sensorNum], irms[sensorNum]);
  // 電圧と電流が逆相の場合の対策
  watt[sensorNum] = abs(watt[sensorNum]);
  // 20W(200V回路のセンサーは40W)未満は測定誤差として切り捨て
  if (watt[sensorNum] < (0.2 * eepromCtVoltage[sensorNum])) {
    watt[sensorNum] = 0.0;
  }
  watt_max[sensorNum] = max(watt_max[sensorNum], watt[sensorNum]);
}

void timer1(void) {
  timer1_status = TIMER1_STS_CALCULATE;
  sampleFromSensor(1);
  sampleFromSensor(2);
  sampleFromSensor(3);
  watt[0] = watt[1] + watt[2] + watt[3];
  watt_max[0] = max(watt_max[0], watt[0]);
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
      saveEventToSdCard(F("SYSTEM: Power down"));
      //popupWindowを閉じてメインに戻る
      if (currScreenMode == SCREEN_POPUP) {
        popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
      }
      timer1_status = TIMER1_STS_IDLE;
      return;
    }
  } else if (vrms[1] < 80.0) {
    if (hasBrownout == false) {
      hasBrownout = true;
      saveEventToSdCard(F("SYSTEM: Power voltage dropped"));
      if (currScreenMode == SCREEN_POPUP) {
        popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
      }
      timer1_status = TIMER1_STS_IDLE;
      return;
    }
  }
  // 復電、電源電圧復帰検出
  if (vrms[1] >= 90.0) {
    if (hasBlackout) {
      hasBlackout = false;
      hasBrownout = false;
      requestFlashAirReset();
      saveEventToSdCard(F("SYSTEM: Power restored"));
      if (currScreenMode == SCREEN_POPUP) {
        popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
      }
    }
    if (hasBrownout) {
      hasBrownout = false;
      requestFlashAirReset();
      saveEventToSdCard(F("SYSTEM: Power voltage recovered"));
      if (currScreenMode == SCREEN_POPUP) {
        popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
      }
    }
  }
  timer1_status = TIMER1_STS_IDLE;
}

// ************************************************************************************
// ボタン押下判定関数
bool isButtonPressed(int btnNum, int x, int y) {
  if ((x >= btn[btnNum].x1) && (x <= btn[btnNum].x2) && (y >= btn[btnNum].y1) && (y <= btn[btnNum].y2)) {
    return true;
  }
  return false;
}

int getButtonNum(void) {
  int x, y;

  if (myTouch.dataAvailable()) {
    beep(BEEP_LOW);
    if ((currScreenMode == SCREEN_BLANK) || (currScreenMode == SCREEN_CLOCK)) {
      return BTN_SCREEN_ON;
    }
    myTouch.read();
    x = myTouch.getX();
    y = myTouch.getY();
    if (isButtonPressed(BTN_RESET, x, y)) {
      return BTN_RESET;
    }
    if (isButtonPressed(BTN_DATE_TIME, x, y)) {
      return BTN_DATE_TIME;
    }
    if (isButtonPressed(BTN_CONFIG, x, y)) {
      return BTN_CONFIG;
    }
    if (isButtonPressed(BTN_SD_CARD, x, y)) {
      return BTN_SD_CARD;
    }
    if (isButtonPressed(BTN_SCREEN_OFF, x, y)) {
      return BTN_SCREEN_OFF;
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
  int len;

  len = s.length();
  int i = 0;
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

void printlnEx(String s) {
  printEx(s);
  printEx(F("\n"));
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

void printBytes(uint8_t *p, uint32_t len) {
  for (int i = 0; i < len; ++i) {
    printByte(p[i]);
  }
}

void convIPAddressToString(uint8_t *p, char *s) {
  // 変数sのサイズは16byte以上必要
  sprintf(s, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
}

void printIPAddress(uint8_t *p) {
  char buf[16];

  convIPAddressToString(p, buf);
  printEx(buf);
}

void printHex(uint8_t *p, uint32_t len) {
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

void printFreeMemory(int x, int y) {
  char buf[BUFFER_LEN_FULL];
  extern int __heap_start, *__brkval;
  int v;

  sprintf(buf, "Heap:%4d/Stack:%3d",
          (__brkval == 0 ? (int)&__heap_start : (int)__brkval), (int)&v);
  printStringEx(buf, x, y, VGA_WHITE, VGA_BLACK);
}

void printString(char *s1, int x1, int y1, int status) {
  if (status == STS_NORMAL) {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);
  } else {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_WHITE);
  }
  myGLCD.print(s1, x1, y1);
}

void printStringEx(char *s, int x, int y, unsigned int c, unsigned int bc) {
  // 座標、色を指定する
  myGLCD.setColor(c);
  myGLCD.setBackColor(bc);
  myGLCD.print(s, x, y);
}

// 変化のあった桁のみ更新して表示を高速化する
// 桁、座標、文字サイズ、色を指定する
void printStringExEx(char *s1, char *s2, unsigned int digit, int x, int y, int font, unsigned long c) {
  char s[2];
  uint8_t *pFont;
  int fontSizeX;

  switch (font) {
    case FONT_BIG_FONT:
      myGLCD.setFont(BigFont);
      fontSizeX = 16;
      break;
    case FONT_SEVEN_SEG_NUM_FONT:
      myGLCD.setFont(SevenSegNumFont);
      fontSizeX = 32;
      break;
    /*
      case FONT_SIXTEEN_SEGMENT_32X48:
      myGLCD.setFont(SixteenSegment32x48);
      fontSizeX = 32;
      break;
    */
    case FONT_GROTESK_BOLD_32X64:
      myGLCD.setFont(GroteskBold32x64);
      fontSizeX = 32;
      break;
    case FONT_SEVEN_SEG_XXXL_NUM:
      myGLCD.setFont(SevenSeg_XXXL_Num);
      fontSizeX = 64;
      break;
    default:
    // フォールスルーさせて
    // 対象の定数がない場合にFONT_SMALL_FONTで表示させる
    case FONT_SMALL_FONT:
      myGLCD.setFont(SmallFont);
      fontSizeX = 8;
      break;
  }
  pFont = myGLCD.getFont();
  if (c != VGA_NO_CHANGE) {
    myGLCD.setColor(c);
  }
  for (unsigned int i = 0; i < digit; i++) {
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
/*
  void countupTimer(void) {
  if (offTimerInterval.check() != 1) {
    return;
  }
  //1分毎に
  //停電中
  if (hasBlackout) {
    if ((screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_STOP)
        && (screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_END)) {
      screenOffTimerInBlackout++;
    }
    return;
  }
  //通電中
  if ((eepromScreenOffTimer != NEVER_SCREEN_OFF) && (currScreenMode == SCREEN_ON)) {
    screenOffTimer++;
  }
  }
*/
void setScreenMode(void) {
  //停電中
  if (hasBlackout) {
    //停電発生時1回だけ実行
    if (screenOffTimerInBlackout == SCREENOFFTIMERINBLACKOUT_STOP) {
      screenOffTimerInBlackout = SCREENOFFTIMERINBLACKOUT_START;
      if (currScreenMode != SCREEN_ON) {
        currScreenMode = printMainWindow(SCREEN_ON);
        screenOffTimer = SCREENOFFTIMER_START;
        offTimerInterval.reset();
      }
      return;
    }
    //停電から一定時間が経過したら
    if (screenOffTimerInBlackout == SCREENOFFTIMERINBLACKOUT_END) {
      //screenOffTimerInBlackout = SCREENOFFTIMERINBLACKOUT_STOP;
      //無限ループを避けるため
      //ここではSCREENOFFTIMERINBLACKOUT_ENDのままにしておいて
      //復電時にSCREENOFFTIMERINBLACKOUT_STOPに変える
      //画面OFF
      currScreenMode = printMainWindow(SCREEN_BLANK);
    }
    return;
  }

  //通電中
  //復電時1回だけ実行
  if (screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_STOP) {
    screenOffTimerInBlackout = SCREENOFFTIMERINBLACKOUT_STOP;
    if (currScreenMode != SCREEN_ON) {
      //画面ON
      currScreenMode = printMainWindow(SCREEN_ON);
      screenOffTimer = SCREENOFFTIMER_START;
      offTimerInterval.reset();
    }
    return;
  }
  if (currScreenMode != SCREEN_ON) {
    return;
  }
  if (eepromScreenOffTimer == NEVER_SCREEN_OFF) {
    return;
  }
  if (screenOffTimer < eepromScreenOffTimer) {
    return;
  }
  //最後の操作から一定時間が経過したら表示を消す
  if (eepromScreenOffMode == MODE_CLOCK) {
    currScreenMode = printMainWindow(SCREEN_CLOCK);
  } else {
    currScreenMode = printMainWindow(SCREEN_BLANK);
  }
}

int printMainWindow(int screenMode) {
  switch (screenMode) {
    case SCREEN_BLANK:
      myGLCD.clrScr();
      turnOffScreen();
      return screenMode;
#ifdef ENABLE_MODE_CLOCK
    case SCREEN_CLOCK:
      if (hasBlackout == false) {
        printClock();
      } else {
        myGLCD.clrScr();
        turnOffScreen();
      }
      return screenMode;
#endif
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
  myGLCD.print("No.1         Wh(30m)", 0, 70);
  myGLCD.print("No.2         Wh(30m)", 0, 83);
  myGLCD.print("No.3         Wh(30m)", 0, 96);
  myGLCD.print("Total        Wh(30m)", 0, 109);
  myGLCD.setColor(VGA_FUCHSIA);
  myGLCD.print("No.1       W(30m Pk)", 0, 122);
  myGLCD.print("No.2       W(30m Pk)", 0, 135);
  myGLCD.print("No.3       W(30m Pk)", 0, 148);
  myGLCD.print("Total      W(30m Pk)", 0, 161);
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
  printBarGraphFrame(screenMode);
  // 時刻（日本標準時）取得情報を表示
  printSyncTimeInfo(screenMode);
  // ファンクションボタン
  printButtonEx(BTN_RESET, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME, STS_NORMAL);
  printButtonEx(BTN_CONFIG, STS_NORMAL);
  printButtonEx(BTN_SD_CARD, STS_NORMAL);
  printButtonEx(BTN_SCREEN_OFF, STS_NORMAL);
  printSdStatus(screenMode);
  printElapsedDays(screenMode);
  printResetTime(screenMode);
  return screenMode;
}

// SmallFont専用
void printButtonEx(int btnNum, int sts) {
  printButton(btn[btnNum].name1, btn[btnNum].name2, btn[btnNum].x1, btn[btnNum].y1, btn[btnNum].x2, btn[btnNum].y2, sts);
}

//検証・コンパイル時警告発生のためchar*をconst char*に修正
//note:   initializing argument 1 of 'void printButton(char*, char*, int, int, int, int, int)'
//note:   initializing argument 2 of 'void printButton(char*, char*, int, int, int, int, int)'
void printButton(const char *s1, const char *s2, int x1, int y1, int x2, int y2, int sts) {
  int x3;  // s1の開始X座標
  int x4;  // s2の開始X座標

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

void printDateTime(void) {
  char buf[BUFFER_LEN_HALF];

  if ((currScreenMode == SCREEN_BLANK) || (currScreenMode == SCREEN_CLOCK)) {
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

  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  switch (screenMode) {
    case SCREEN_ON:
      x = 0;
      y = 174;
      break;
    case SCREEN_CLOCK:
      x = 0;
      y = 210;
      break;
    default:
      return;
  }
  myGLCD.setBackColor(VGA_BLACK);
  if (isTimeSynchronized) {
    myGLCD.setColor(VGA_LIME);
  } else {
    myGLCD.setColor(VGA_RED);
  }
  myGLCD.setFont(SmallFont);
  sprintf(buf, "T%s", syncTimeInfo);
  myGLCD.print(buf, x, y);
}

void printBarGraphFrame(int screenMode) {
  int x, y;

  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  switch (screenMode) {
    case SCREEN_ON:
      x = 142;
      y = 187;
      break;
    case SCREEN_CLOCK:
      x = 142;
      y = 223;
      break;
    default:
      return;
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

  //描画の左上の基点を設定
  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  switch (screenMode) {
    case SCREEN_ON:
      x = 142;
      y = 187;
      break;
    case SCREEN_CLOCK:
      x = 142;
      y = 223;
      break;
    default:
      return;
  }
  // CONFIG画面で設定したkWがフルスケールになるようにlevelを計算する
  level = (int)(watt[0] * 15.0 / (float)eepromBarGraphFullscale / 1000.0);
  for (segment = 1; segment < 16; segment++) {
    //バーの色を設定
    if (level <= 15) {  // オーバーレンジしていない
      // 描画不要な目盛りに対するスキップ処理
      if (prevLevel <= 15) {  // 直前もオーバーレンジしていない
        // 増加
        if (prevLevel < level) {
          // 増加した部分以外の目盛り
          if ((segment <= prevLevel) || (segment > level)) {
            continue;  // 何もしない
          }
        }
        // 減少
        if (prevLevel > level) {
          // 減少した部分以外の目盛り
          if ((segment <= level) || (segment > prevLevel)) {
            continue;  // 何もしない
          }
        }
        // 変化なし
        if (prevLevel == level) {
          continue;  // 何もしない
        }
      }
      // 目盛りの描画処理
      if (segment <= level) {  // 指示値以内
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
      } else {  // 指示値を超える部分は黒色に
        myGLCD.setColor(VGA_BLACK);
      }
    } else {                       // オーバーレンジ時
      if (prevLevel <= 15) {       // 直前はオーバーレンジしていない
        myGLCD.setColor(VGA_RED);  // 全ての目盛りを赤色に
      } else {                     // 直前もオーバーレンジしている
        continue;                  // 何もしない
      }
    }
    //バーを描画
    myGLCD.fillRect(x + (segment * 8), y + 2, x + 5 + (segment * 8), y + 9);
  }
  prevLevel = level;
  //kW表示
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  dtostrf(watt[0] / 1000.0, 4, 1, buf);
  myGLCD.print(buf, x + 130, y);
}

#ifdef ENABLE_MODE_CLOCK
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
#endif

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
  if (hasBlackout == false) {
    return;
  }
  //停電中
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_RED);
  myGLCD.print(BLACKOUT, 64, 18);
  beep(BEEP_800MS);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(BLACKOUT, 64, 18);
}

void printWattHour(void) {
  char buf[BUFFER_LEN_HALF];

  if ((currScreenMode == SCREEN_BLANK) || (currScreenMode == SCREEN_CLOCK)) {
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
    }
  }
}

void printElapsedDays(int screenMode) {
  char buf[BUFFER_LEN_HALF];

  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  if ((screenMode == SCREEN_BLANK) || (screenMode == SCREEN_CLOCK)) {
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

  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setBackColor(VGA_BLACK);
  sprintf(buf, "%05ld", (long)(wattHourTimeperiod / 1000.0));
  switch (screenMode) {
    case SCREEN_BLANK:
      return;
    case SCREEN_CLOCK:
      printStringExEx(prevWattHourTimeperiod, buf, WATTHOURTIMEPERIOD_LEN, 176, 197, FONT_BIG_FONT);
      strcpy(prevWattHourTimeperiod, buf);
      return;
    default:
      printStringExEx(prevWattHourTimeperiod, buf, WATTHOURTIMEPERIOD_LEN, 160, 109, FONT_SEVEN_SEG_NUM_FONT);
      strcpy(prevWattHourTimeperiod, buf);
      break;
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

  if (popupWindowOffTimer != POPUPWINDOWOFFTIMER_END) {
    return;
  }
  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
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
    case SCREEN_BLANK:
      return;
    default:
      //何もししない
      break;
  }
  if (printMode == PRINTMODE_LINE) {
    if (sdSts != SD_SUCCESS) {
      printEx(F("\n"));
      printEx(sdStsMsg[sdSts]);
    }
    return;
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

  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  switch (screenMode) {
    case SCREEN_BLANK:
      return;
    case SCREEN_ON:
      x = 208;
      y = 31;
      break;
    case SCREEN_CLOCK:
      x = 48;
      y = 197;
      break;
    default:
      //何もししない
      break;
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
  printButtonEx(BTN_RESET, STS_INVERSE);
  currScreenMode = SCREEN_POPUP;
  printWindowFrame(24, 62, 296, 160);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("RESET THE INTEGRATED VALUES?", 32, 90);
  myGLCD.setColor(VGA_LIME);
  printButtonEx(BTN_RESET_SYSTEM_RESET, STS_NORMAL);
  printButtonEx(BTN_RESET_OK, STS_NORMAL);
  printButtonEx(BTN_RESET_CANCEL, STS_NORMAL);
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (popupWindowOffTimer == POPUPWINDOWOFFTIMER_END) {
      break;
    }
    if (offTimerInterval.check() == 1) {
      popupWindowOffTimer++;
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      // SYSTEM RESET
      if (isButtonPressed(BTN_RESET_SYSTEM_RESET, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_RESET_SYSTEM_RESET, STS_INVERSE);
        systemReset();
        break;
      }
      // OK
      if (isButtonPressed(BTN_RESET_OK, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_RESET_OK, STS_INVERSE);
        saveIntegratedValuesToSdCard(SCREEN_BLANK);
        saveEventToSdCard(F("RESET : Integrated values reset"));
        break;
      }
      // CANCEL
      if (isButtonPressed(BTN_RESET_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        break;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

// SYSTEM RESETボタン押下
void systemReset(void) {
  int x, y;

  //beep(BEEP_HIGH);
  printWindowFrame(24, 62, 296, 160);
  currScreenMode = SCREEN_POPUP;
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("RESET THE SYSTEM?", 32, 76);
  myGLCD.print("IF YOU DO, THE INTEGRATED VALUES", 32, 90);
  myGLCD.print("WILL ALSO BE RESET!", 32, 104);
  myGLCD.setColor(VGA_LIME);
  printButtonEx(BTN_SYSTEM_RESET_OK, STS_NORMAL);
  printButtonEx(BTN_SYSTEM_RESET_CANCEL, STS_NORMAL);
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (popupWindowOffTimer == POPUPWINDOWOFFTIMER_END) {
      break;
    }
    if (offTimerInterval.check() == 1) {
      popupWindowOffTimer++;
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      // OK
      if (isButtonPressed(BTN_SYSTEM_RESET_OK, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_SYSTEM_RESET_OK, STS_INVERSE);
        saveIntegratedValuesToSdCard(SCREEN_BLANK);
        saveEventToSdCard(F("RESET : Integrated values reset"), SCREEN_BLANK);
        saveEventToSdCard(F("RESET : System reset"), SCREEN_BLANK);
        resetSystem();
        //ここへは到達しない
        break;
      }
      // CANCEL
      if (isButtonPressed(BTN_SYSTEM_RESET_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        break;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
}

void (*resetFunc)(void) = 0;

void resetSystem() {
  clearScreen();
  myGLCD.print(F("Resetting the system..."), 0, 0);
  curX = 0;
  curY = 14;
  if (existsFlashAir) {
    onOffFlashAir();
  }
  myGLCD.clrScr();
  resetFunc();
  //ここへは到達しない
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
  Time t = getClockTime();
  prevDate = t.date;
  elapsedDays = 1;
}

void saveIntegratedValuesToSdCard(int screenMode) {
  File file;
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
    file = SD.open(fileName, FILE_WRITE);
    file.println("yyyy/mm/dd,hh:mm:ss,7-10(kWh),10-17(kWh),17-23(kWh),23-7(kWh)");
    file.close();
  }
  // ここからデータ行を書き込み
  file = SD.open(fileName, FILE_WRITE);
  if (file) {
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
    file.println(dataString);
    file.close();
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
  resetIntegratedValues();
  //リセット直後のデータを書き込み
  saveDataToSdCard(screenMode);
}

// ************************************************************************************
// DATE TIMEボタン押下
void adjustDateTime(void) {
  int x, y;
  int mode;
  // 各月の日数
  int dayset[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  char event[BUFFER_LEN_EVENT];

  beep(BEEP_HIGH);
  printButtonEx(BTN_DATE_TIME, STS_INVERSE);
  currScreenMode = SCREEN_POPUP;
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
  printButtonEx(BTN_DATE_TIME_UP, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_MINUS, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_PLUS, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_DOWN, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_SYNC, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_OK, STS_NORMAL);
  printButtonEx(BTN_DATE_TIME_CANCEL, STS_NORMAL);
  mode = TIME_YEAR;
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (popupWindowOffTimer == POPUPWINDOWOFFTIMER_END) {
      break;
    }
    if (offTimerInterval.check() == 1) {
      popupWindowOffTimer++;
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
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
      // DOWN
      if (isButtonPressed(BTN_DATE_TIME_DOWN, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_DATE_TIME_DOWN, STS_INVERSE);
        printTempTime(t, mode, STS_NORMAL);
        mode = changeValue(mode, 1, 0, 6);
        printTempTime(t, mode, STS_INVERSE);
        printButtonEx(BTN_DATE_TIME_DOWN, STS_NORMAL);
        continue;
      }
      // UP
      if (isButtonPressed(BTN_DATE_TIME_UP, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_DATE_TIME_UP, STS_INVERSE);
        printTempTime(t, mode, STS_NORMAL);
        mode = changeValue(mode, -1, 0, 6);
        printTempTime(t, mode, STS_INVERSE);
        printButtonEx(BTN_DATE_TIME_UP, STS_NORMAL);
        continue;
      }
      // +
      if (isButtonPressed(BTN_DATE_TIME_PLUS, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_DATE_TIME_PLUS, STS_INVERSE);
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
        printButtonEx(BTN_DATE_TIME_PLUS, STS_NORMAL);
        continue;
      }
      // -
      if (isButtonPressed(BTN_DATE_TIME_MINUS, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_DATE_TIME_MINUS, STS_INVERSE);
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
        printButtonEx(BTN_DATE_TIME_MINUS, STS_NORMAL);
        continue;
      }
      // SYNC
      if (isButtonPressed(BTN_DATE_TIME_SYNC, x, y)) {
        if (existsFlashAir) {
          beep(BEEP_HIGH);
          printButtonEx(BTN_DATE_TIME_SYNC, STS_INVERSE);
          currScreenMode = SCREEN_SETUP;
          clearScreen();
          if (isResetFlashAirRequested) {
            resetFlashAir();
          }
          isTimeSynchronized = syncTime();
          if (isTimeSynchronized) {
            sprintf(event, "SYNC  : [%04lXh]Time synchronized", nextSequenceId - 1);
            saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
          } else {
            sprintf(event, "SYNC  : [%04lXh]Time synchronization failed", nextSequenceId - 1);
            saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
          }
          delay(3000);
        }
        break;
      }
      // OK
      if (isButtonPressed(BTN_DATE_TIME_OK, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_DATE_TIME_OK, STS_INVERSE);
        setClockTime(t);
        break;
      }
      // CANCEL
      if (isButtonPressed(BTN_DATE_TIME_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        break;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
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
      myGLCD.print(buf, 152, 34);
      break;
    case TIME_MONTH:
      sprintf(buf, "%02d", tt.mon);
      myGLCD.print(buf, 168, 48);
      break;
    case TIME_DAY:
      sprintf(buf, "%02d", tt.date);
      myGLCD.print(buf, 168, 62);
      break;
    case TIME_DAYNAME:
      sprintf(buf, "%3s", dayShortStr(tt.day));
      myGLCD.print(buf, 160, 76);
      break;
    case TIME_HOUR:
      sprintf(buf, "%02d", tt.hr);
      myGLCD.print(buf, 168, 90);
      break;
    case TIME_MINUTE:
      sprintf(buf, "%02d", tt.min);
      myGLCD.print(buf, 168, 104);
      break;
    case TIME_SECOND:
      sprintf(buf, "%02d", tt.sec);
      myGLCD.print(buf, 168, 118);
      break;
  }
}

int changeValue(int value, int step, int lowerLimit, int upperLimit) {
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
  char buf[BUFFER_LEN_HALF];
  const int COL_LEFT = 16;
  const int COL_WIDTH = 8;
  const int ROW_TOP = 26;
  const int ROW_HEIGHT = 14;
  int index;
  //項目の表示順を設定
  //要素を行単位で入れ替えることで表示順を変更可能
  //横に並んでいる要素はバラさない(同じ行に表示する前提のため)
  int mode[EEPROM_MODE_MAX] = {
    EEPROM_AUTO_RESET_DATE,
    EEPROM_AUTO_TIME_SYNC,
    EEPROM_TIME_SYNC_HOUR,
    EEPROM_TIME_SYNC_MIN,
    EEPROM_VT_PERCENTAGE,
    EEPROM_CT1_VOLTAGE,
    EEPROM_CT1_PERCENTAGE,
    EEPROM_CT2_VOLTAGE,
    EEPROM_CT2_PERCENTAGE,
    EEPROM_CT3_VOLTAGE,
    EEPROM_CT3_PERCENTAGE,
    EEPROM_BAR_GRAPH_FULLSCALE,
    EEPROM_SCREEN_OFF_TIMER,
    EEPROM_SCREEN_OFF_MODE,
    EEPROM_SOUND,
  };
  struct ITEM item[EEPROM_MODE_MAX];

  beep(BEEP_HIGH);
  printButtonEx(BTN_CONFIG, STS_INVERSE);
  currScreenMode = SCREEN_POPUP;
  printWindowFrame(8, 20, 312, 230);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  y = ROW_TOP;
  myGLCD.print(VER, COL_LEFT, y);
  y += ROW_HEIGHT;
  if (existsFlashAir) {
    sprintf(buf, "IP       %15s", flashAirIP);
    myGLCD.print(buf, COL_LEFT, y);
  }
  //設定項目の名前を画面に表示
  //nameとformatの最大文字数に注意
  for (index = 0; index < EEPROM_MODE_MAX; index++) {
    switch (mode[index]) {
      case EEPROM_AUTO_RESET_DATE:
        //Auto reset date
        y += ROW_HEIGHT;
        item[EEPROM_AUTO_RESET_DATE].name = "Auto reset date";
        myGLCD.print(item[EEPROM_AUTO_RESET_DATE].name, COL_LEFT, y);
        item[EEPROM_AUTO_RESET_DATE].value = eepromAutoResetDate;
        item[EEPROM_AUTO_RESET_DATE].format = "%02d";
        item[EEPROM_AUTO_RESET_DATE].x = COL_LEFT + COL_WIDTH * 22;
        item[EEPROM_AUTO_RESET_DATE].y = y;
        break;
      case EEPROM_VT_PERCENTAGE:
        //Voltage
        y += ROW_HEIGHT;
        item[EEPROM_VT_PERCENTAGE].name = "Voltage";
        sprintf(buf, "%s        100V", item[EEPROM_VT_PERCENTAGE].name);
        myGLCD.print(buf, COL_LEFT, y);
        item[EEPROM_VT_PERCENTAGE].value = eepromVtPercentage;
        item[EEPROM_VT_PERCENTAGE].format = "%3d%%";
        item[EEPROM_VT_PERCENTAGE].x = COL_LEFT + COL_WIDTH * 20;
        item[EEPROM_VT_PERCENTAGE].y = y;
        break;
      case EEPROM_CT1_VOLTAGE:
        //Sensor1
        y += ROW_HEIGHT;
        item[EEPROM_CT1_VOLTAGE].name = "Sensor1";
        myGLCD.print(item[EEPROM_CT1_VOLTAGE].name, COL_LEFT, y);
        item[EEPROM_CT1_VOLTAGE].value = eepromCtVoltage[1];
        item[EEPROM_CT1_VOLTAGE].format = "%3dV";
        item[EEPROM_CT1_VOLTAGE].x = COL_LEFT + COL_WIDTH * 15;
        item[EEPROM_CT1_VOLTAGE].y = y;
        break;
      case EEPROM_CT1_PERCENTAGE:
        item[EEPROM_CT1_PERCENTAGE].name = item[EEPROM_CT1_VOLTAGE].name;
        item[EEPROM_CT1_PERCENTAGE].value = eepromCtPercentage[1];
        item[EEPROM_CT1_PERCENTAGE].format = "%3d%%";
        item[EEPROM_CT1_PERCENTAGE].x = COL_LEFT + COL_WIDTH * 20;
        item[EEPROM_CT1_PERCENTAGE].y = y;
        break;
      case EEPROM_CT2_VOLTAGE:
        //Sensor2
        y += ROW_HEIGHT;
        item[EEPROM_CT2_VOLTAGE].name = "Sensor2";
        myGLCD.print(item[EEPROM_CT2_VOLTAGE].name, COL_LEFT, y);
        item[EEPROM_CT2_VOLTAGE].value = eepromCtVoltage[2];
        item[EEPROM_CT2_VOLTAGE].format = "%3dV";
        item[EEPROM_CT2_VOLTAGE].x = COL_LEFT + COL_WIDTH * 15;
        item[EEPROM_CT2_VOLTAGE].y = y;
        break;
      case EEPROM_CT2_PERCENTAGE:
        item[EEPROM_CT2_PERCENTAGE].name = item[EEPROM_CT2_VOLTAGE].name;
        item[EEPROM_CT2_PERCENTAGE].value = eepromCtPercentage[2];
        item[EEPROM_CT2_PERCENTAGE].format = "%3d%%";
        item[EEPROM_CT2_PERCENTAGE].x = COL_LEFT + COL_WIDTH * 20;
        item[EEPROM_CT2_PERCENTAGE].y = y;
        break;
      case EEPROM_CT3_VOLTAGE:
        //Sensor3
        y += ROW_HEIGHT;
        item[EEPROM_CT3_VOLTAGE].name = "Sensor3";
        myGLCD.print(item[EEPROM_CT3_VOLTAGE].name, COL_LEFT, y);
        item[EEPROM_CT3_VOLTAGE].value = eepromCtVoltage[3];
        item[EEPROM_CT3_VOLTAGE].format = "%3dV";
        item[EEPROM_CT3_VOLTAGE].x = COL_LEFT + COL_WIDTH * 15;
        item[EEPROM_CT3_VOLTAGE].y = y;
        break;
      case EEPROM_CT3_PERCENTAGE:
        item[EEPROM_CT3_PERCENTAGE].name = item[EEPROM_CT3_VOLTAGE].name;
        item[EEPROM_CT3_PERCENTAGE].value = eepromCtPercentage[3];
        item[EEPROM_CT3_PERCENTAGE].format = "%3d%%";
        item[EEPROM_CT3_PERCENTAGE].x = COL_LEFT + COL_WIDTH * 20;
        item[EEPROM_CT3_PERCENTAGE].y = y;
        break;
      case EEPROM_BAR_GRAPH_FULLSCALE:
        //Bar graph FS
        y += ROW_HEIGHT;
        item[EEPROM_BAR_GRAPH_FULLSCALE].name = "Bar graph fullscale";
        myGLCD.print(item[EEPROM_BAR_GRAPH_FULLSCALE].name, COL_LEFT, y);
        item[EEPROM_BAR_GRAPH_FULLSCALE].value = eepromBarGraphFullscale;
        item[EEPROM_BAR_GRAPH_FULLSCALE].format = "%2dkW";
        item[EEPROM_BAR_GRAPH_FULLSCALE].x = COL_LEFT + COL_WIDTH * 20;
        item[EEPROM_BAR_GRAPH_FULLSCALE].y = y;
        break;
      case EEPROM_SCREEN_OFF_TIMER:
        //Screen off timer
        y += ROW_HEIGHT;
        item[EEPROM_SCREEN_OFF_TIMER].name = "Screen off timer";
        myGLCD.print(item[EEPROM_SCREEN_OFF_TIMER].name, COL_LEFT, y);
        item[EEPROM_SCREEN_OFF_TIMER].value = eepromScreenOffTimer;
        item[EEPROM_SCREEN_OFF_TIMER].format = "%2dmin";
        item[EEPROM_SCREEN_OFF_TIMER].x = COL_LEFT + COL_WIDTH * 19;
        item[EEPROM_SCREEN_OFF_TIMER].y = y;
        break;
      case EEPROM_SCREEN_OFF_MODE:
        //Screen off mode
        y += ROW_HEIGHT;
        item[EEPROM_SCREEN_OFF_MODE].name = "Screen off mode";
        myGLCD.print(item[EEPROM_SCREEN_OFF_MODE].name, COL_LEFT, y);
        item[EEPROM_SCREEN_OFF_MODE].value = eepromScreenOffMode;
        item[EEPROM_SCREEN_OFF_MODE].format = "%5s";
        item[EEPROM_SCREEN_OFF_MODE].x = COL_LEFT + COL_WIDTH * 19;
        item[EEPROM_SCREEN_OFF_MODE].y = y;
        break;
      case EEPROM_SOUND:
        //Sound
        y += ROW_HEIGHT;
        item[EEPROM_SOUND].name = "Sound";
        myGLCD.print(item[EEPROM_SOUND].name, COL_LEFT, y);
        item[EEPROM_SOUND].value = eepromSound;
        item[EEPROM_SOUND].format = "%3s";
        item[EEPROM_SOUND].x = COL_LEFT + COL_WIDTH * 21;
        item[EEPROM_SOUND].y = y;
        break;
      case EEPROM_AUTO_TIME_SYNC:
        //Auto time sync
        y += ROW_HEIGHT;
        item[EEPROM_AUTO_TIME_SYNC].name = "Auto time sync";
        sprintf(buf, "%s       :", item[EEPROM_AUTO_TIME_SYNC].name);
        myGLCD.print(buf, COL_LEFT, y);  //':'は時と分の間のコロン
        item[EEPROM_AUTO_TIME_SYNC].value = eepromAutoTimeSync;
        item[EEPROM_AUTO_TIME_SYNC].format = "%3s";
        item[EEPROM_AUTO_TIME_SYNC].x = COL_LEFT + COL_WIDTH * 15;
        item[EEPROM_AUTO_TIME_SYNC].y = y;
        break;
      case EEPROM_TIME_SYNC_HOUR:
        item[EEPROM_TIME_SYNC_HOUR].name = "Auto time sync hour";
        item[EEPROM_TIME_SYNC_HOUR].value = eepromTimeSyncHour;
        item[EEPROM_TIME_SYNC_HOUR].format = "%02d";
        item[EEPROM_TIME_SYNC_HOUR].x = COL_LEFT + COL_WIDTH * 19;
        item[EEPROM_TIME_SYNC_HOUR].y = y;
        break;
      case EEPROM_TIME_SYNC_MIN:
        item[EEPROM_TIME_SYNC_MIN].name = "Auto time sync min";
        item[EEPROM_TIME_SYNC_MIN].value = eepromTimeSyncMin;
        item[EEPROM_TIME_SYNC_MIN].format = "%02d";
        item[EEPROM_TIME_SYNC_MIN].x = COL_LEFT + COL_WIDTH * 22;
        item[EEPROM_TIME_SYNC_MIN].y = y;
        break;
      default:
        //ここが実行されることはない
        break;
    }
  }
  //設定項目の値を画面に表示
  index = 0;
  printItems(item, mode[index]);
  //ボタンを表示
  printButtonEx(BTN_CONFIG_UP, STS_NORMAL);
  printButtonEx(BTN_CONFIG_MINUS, STS_NORMAL);
  printButtonEx(BTN_CONFIG_PLUS, STS_NORMAL);
  printButtonEx(BTN_CONFIG_DOWN, STS_NORMAL);
  printButtonEx(BTN_CONFIG_LOAD_DEFAULT, STS_NORMAL);
  printButtonEx(BTN_CONFIG_OK, STS_NORMAL);
  printButtonEx(BTN_CONFIG_CANCEL, STS_NORMAL);
  //タッチスクリーンを読み取り
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (popupWindowOffTimer == POPUPWINDOWOFFTIMER_END) {
      break;
    }
    if (offTimerInterval.check() == 1) {
      popupWindowOffTimer++;
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      // DOWN
      if (isButtonPressed(BTN_CONFIG_DOWN, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_DOWN, STS_INVERSE);
        printItem(item, mode[index], STS_NORMAL);
        index = changeValue(index, 1, 0, EEPROM_MODE_MAX - 1);
        printItem(item, mode[index], STS_INVERSE);
        printButtonEx(BTN_CONFIG_DOWN, STS_NORMAL);
        continue;
      }
      // UP
      if (isButtonPressed(BTN_CONFIG_UP, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_UP, STS_INVERSE);
        printItem(item, mode[index], STS_NORMAL);
        index = changeValue(index, -1, 0, EEPROM_MODE_MAX - 1);
        printItem(item, mode[index], STS_INVERSE);
        printButtonEx(BTN_CONFIG_UP, STS_NORMAL);
        continue;
      }
      // +
      if (isButtonPressed(BTN_CONFIG_PLUS, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_PLUS, STS_INVERSE);
        switch (mode[index]) {
          case EEPROM_SCREEN_OFF_TIMER:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 0, 99);
            break;
          case EEPROM_SCREEN_OFF_MODE:
#ifdef ENABLE_MODE_CLOCK
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, MODE_BLANK, MODE_CLOCK);
#endif
            break;
          case EEPROM_AUTO_RESET_DATE:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 0, 28);
            break;
          case EEPROM_CT1_VOLTAGE:
          case EEPROM_CT2_VOLTAGE:
          case EEPROM_CT3_VOLTAGE:
            item[mode[index]].value = changeValue(item[mode[index]].value, 100, 0, 200);
            break;
          case EEPROM_VT_PERCENTAGE:
          case EEPROM_CT1_PERCENTAGE:
          case EEPROM_CT2_PERCENTAGE:
          case EEPROM_CT3_PERCENTAGE:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 1, 254);
            break;
          case EEPROM_SOUND:
          case EEPROM_AUTO_TIME_SYNC:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, OFF, ON);
            break;
          case EEPROM_TIME_SYNC_HOUR:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 0, 23);
            break;
          case EEPROM_TIME_SYNC_MIN:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 0, 59);
            break;
          case EEPROM_BAR_GRAPH_FULLSCALE:
            item[mode[index]].value = changeValue(item[mode[index]].value, 1, 1, 49);
            break;
        }
        printItem(item, mode[index], STS_INVERSE);
        printButtonEx(BTN_CONFIG_PLUS, STS_NORMAL);
        continue;
      }
      // -
      if (isButtonPressed(BTN_CONFIG_MINUS, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_MINUS, STS_INVERSE);
        switch (mode[index]) {
          case EEPROM_SCREEN_OFF_TIMER:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 0, 99);
            break;
          case EEPROM_SCREEN_OFF_MODE:
#ifdef ENABLE_MODE_CLOCK
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, MODE_BLANK, MODE_CLOCK);
#endif
            break;
          case EEPROM_AUTO_RESET_DATE:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 0, 28);
            break;
          case EEPROM_CT1_VOLTAGE:
          case EEPROM_CT2_VOLTAGE:
          case EEPROM_CT3_VOLTAGE:
            item[mode[index]].value = changeValue(item[mode[index]].value, -100, 0, 200);
            break;
          case EEPROM_VT_PERCENTAGE:
          case EEPROM_CT1_PERCENTAGE:
          case EEPROM_CT2_PERCENTAGE:
          case EEPROM_CT3_PERCENTAGE:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 1, 254);
            break;
          case EEPROM_SOUND:
          case EEPROM_AUTO_TIME_SYNC:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, OFF, ON);
            break;
          case EEPROM_TIME_SYNC_HOUR:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 0, 23);
            break;
          case EEPROM_TIME_SYNC_MIN:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 0, 59);
            break;
          case EEPROM_BAR_GRAPH_FULLSCALE:
            item[mode[index]].value = changeValue(item[mode[index]].value, -1, 1, 49);
            break;
        }
        printItem(item, mode[index], STS_INVERSE);
        printButtonEx(BTN_CONFIG_MINUS, STS_NORMAL);
        continue;
      }
      // LOAD DEFAULT
      if (isButtonPressed(BTN_CONFIG_LOAD_DEFAULT, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_LOAD_DEFAULT, STS_INVERSE);
        printItem(item, mode[index], STS_NORMAL);
        item[EEPROM_SCREEN_OFF_TIMER].value = DEFAULT_SCREEN_OFF_TIMER;
        item[EEPROM_AUTO_RESET_DATE].value = DEFAULT_AUTO_RESET_DATE;
        item[EEPROM_CT1_VOLTAGE].value = DEFAULT_CT1_VOLTAGE;
        item[EEPROM_CT2_VOLTAGE].value = DEFAULT_CT2_VOLTAGE;
        item[EEPROM_CT3_VOLTAGE].value = DEFAULT_CT3_VOLTAGE;
        item[EEPROM_VT_PERCENTAGE].value = DEFAULT_VT_PERCENTAGE;
        item[EEPROM_CT1_PERCENTAGE].value = DEFAULT_CT1_PERCENTAGE;
        item[EEPROM_CT2_PERCENTAGE].value = DEFAULT_CT2_PERCENTAGE;
        item[EEPROM_CT3_PERCENTAGE].value = DEFAULT_CT3_PERCENTAGE;
        item[EEPROM_SOUND].value = DEFAULT_SOUND;
        item[EEPROM_SCREEN_OFF_MODE].value = DEFAULT_SCREEN_OFF_MODE;
        item[EEPROM_AUTO_TIME_SYNC].value = DEFAULT_AUTO_TIME_SYNC;
        item[EEPROM_TIME_SYNC_HOUR].value = DEFAULT_TIME_SYNC_HOUR;
        item[EEPROM_TIME_SYNC_MIN].value = DEFAULT_TIME_SYNC_MIN;
        item[EEPROM_BAR_GRAPH_FULLSCALE].value = DEFAULT_BAR_GRAPH_FULLSCALE;
        printItems(item, mode[index]);
        printButtonEx(BTN_CONFIG_LOAD_DEFAULT, STS_NORMAL);
        continue;
      }
      // OK
      if (isButtonPressed(BTN_CONFIG_OK, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_CONFIG_OK, STS_INVERSE);
        eepromScreenOffTimer = writeValueToEEPROM(eepromScreenOffTimer, item, EEPROM_SCREEN_OFF_TIMER);
        eepromAutoResetDate = writeValueToEEPROM(eepromAutoResetDate, item, EEPROM_AUTO_RESET_DATE);
        eepromCtVoltage[1] = writeValueToEEPROM(eepromCtVoltage[1], item, EEPROM_CT1_VOLTAGE);
        eepromCtVoltage[2] = writeValueToEEPROM(eepromCtVoltage[2], item, EEPROM_CT2_VOLTAGE);
        eepromCtVoltage[3] = writeValueToEEPROM(eepromCtVoltage[3], item, EEPROM_CT3_VOLTAGE);
        eepromVtPercentage = writeValueToEEPROM(eepromVtPercentage, item, EEPROM_VT_PERCENTAGE);
        eepromCtPercentage[1] = writeValueToEEPROM(eepromCtPercentage[1], item, EEPROM_CT1_PERCENTAGE);
        eepromCtPercentage[2] = writeValueToEEPROM(eepromCtPercentage[2], item, EEPROM_CT2_PERCENTAGE);
        eepromCtPercentage[3] = writeValueToEEPROM(eepromCtPercentage[3], item, EEPROM_CT3_PERCENTAGE);
        eepromSound = writeValueToEEPROM(eepromSound, item, EEPROM_SOUND);
        eepromScreenOffMode = writeValueToEEPROM(eepromScreenOffMode, item, EEPROM_SCREEN_OFF_MODE);
        eepromAutoTimeSync = writeValueToEEPROM(eepromAutoTimeSync, item, EEPROM_AUTO_TIME_SYNC);
        eepromTimeSyncHour = writeValueToEEPROM(eepromTimeSyncHour, item, EEPROM_TIME_SYNC_HOUR);
        eepromTimeSyncMin = writeValueToEEPROM(eepromTimeSyncMin, item, EEPROM_TIME_SYNC_MIN);
        eepromBarGraphFullscale = writeValueToEEPROM(eepromBarGraphFullscale, item, EEPROM_BAR_GRAPH_FULLSCALE);
        break;
      }
      // CANCEL
      if (isButtonPressed(BTN_CONFIG_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        break;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

// <summary>
// item[mode].valueとvalueが一致しなければ
// ・item[mode.valueの値をEEPROMに書き込む
// ・SDカードにイベントを書き込む
// item[mode.valueの値を返す
// </summary>
// <param name="value">元の設定値</param>
// <param name="*item">CONFIGの設定項目表示用のパラメータを保持するITEM構造体変数</param>
// <param name="mode">*item変数の添字(要素を指定する定数)</param>
// <returns>item[mode.value</returns>
byte writeValueToEEPROM(byte value, struct ITEM *item, int mode) {
  char event[BUFFER_LEN_EVENT];
  char oldValue[11];
  char newValue[11];

  if (item[mode].value != value) {
    EEPROM.write(mode, item[mode].value);
    formatValue(oldValue, item, mode, value);
    formatValue(newValue, item, mode, item[mode].value);
    sprintf(event, "CONFIG: %s changed %s -> %s", item[mode].name, oldValue, newValue);
    saveEventToSdCard(event);
  }
  return item[mode].value;
}

void formatValue(char *str, struct ITEM *item, int mode, byte value) {
  switch (mode) {
    case EEPROM_SCREEN_OFF_MODE:
      sprintf(str, item[mode].format, (value == MODE_CLOCK) ? "Clock" : "Blank");
      break;
    case EEPROM_SOUND:
    case EEPROM_AUTO_TIME_SYNC:
      sprintf(str, item[mode].format, (value == ON) ? "ON" : "OFF");
      break;
    default:
      sprintf(str, item[mode].format, value);
      break;
  }
}

void printItem(struct ITEM *item, int mode, int sts) {
  char buf[BUFFER_LEN_HALF];

  if (mode >= EEPROM_MODE_MAX) {
    return;
  }
  if (sts == STS_NORMAL) {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);
  } else {
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_WHITE);
  }
  formatValue(buf, item, mode, item[mode].value);
  myGLCD.print(buf, item[mode].x, item[mode].y);
}

void printItems(struct ITEM *item, int mode) {
  if (mode >= EEPROM_MODE_MAX) {
    return;
  }
  for (int i = 0; i < EEPROM_MODE_MAX; i++) {
    printItem(item, i, (i == mode) ? STS_INVERSE : STS_NORMAL);
  }
}

// ************************************************************************************
// SD CARDボタン押下
void sdCard(void) {
  int x, y;
  int fileNum;
  int currPage, nextPage;
  char event[BUFFER_LEN_EVENT];

  beep(BEEP_HIGH);
  printButtonEx(BTN_SD_CARD, STS_INVERSE);
  currScreenMode = SCREEN_POPUP;
  fileNum = 0;
  printFileListWindow(fileNum);
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (popupWindowOffTimer == POPUPWINDOWOFFTIMER_END) {
      break;
    }
    if (offTimerInterval.check() == 1) {
      popupWindowOffTimer++;
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();

      // PAGE UP
      if (isButtonPressed(BTN_SD_CARD_PAGE_UP, x, y)) {
        if (numberOfFiles <= 10) {
          beep(BEEP_LOW);
          continue;
        }
        beep(BEEP_HIGH);
        printButtonEx(BTN_SD_CARD_PAGE_UP, STS_INVERSE);
        currPage = (int)(fileNum / 10.0) + 1;
        nextPage = (int)((numberOfFiles - 1) / 10.0) + 1;
        if (currPage > 1) {
          fileNum = (currPage - 2) * 10;
        } else {
          fileNum = (nextPage - 1) * 10;
        }
        printFileList(fileNum);
        printButtonEx(BTN_SD_CARD_PAGE_UP, STS_NORMAL);
        continue;
      }
      // CURSOR UP
      if (isButtonPressed(BTN_SD_CARD_CURSOR_UP, x, y)) {
        if (numberOfFiles < 1) {
          beep(BEEP_LOW);
          continue;
        }
        beep(BEEP_HIGH);
        printButtonEx(BTN_SD_CARD_CURSOR_UP, STS_INVERSE);
        if (fileNum > 0) {
          printFile(fileNum, fileNum - 1);
          fileNum--;
        } else {
          printFile(fileNum, numberOfFiles - 1);
          fileNum = numberOfFiles - 1;
        }
        printButtonEx(BTN_SD_CARD_CURSOR_UP, STS_NORMAL);
        continue;
      }
      // CURSOR DOWN
      if (isButtonPressed(BTN_SD_CARD_CURSOR_DOWN, x, y)) {
        if (numberOfFiles < 1) {
          beep(BEEP_LOW);
          continue;
        }
        beep(BEEP_HIGH);
        printButtonEx(BTN_SD_CARD_CURSOR_DOWN, STS_INVERSE);
        if ((fileNum + 1) < numberOfFiles) {
          printFile(fileNum, fileNum + 1);
          fileNum++;
        } else {
          printFile(fileNum, 0);
          fileNum = 0;
        }
        printButtonEx(BTN_SD_CARD_CURSOR_DOWN, STS_NORMAL);
        continue;
      }
      // PAGE DOWN
      if (isButtonPressed(BTN_SD_CARD_PAGE_DOWN, x, y)) {
        if (numberOfFiles <= 10) {
          beep(BEEP_LOW);
          continue;
        }
        beep(BEEP_HIGH);
        printButtonEx(BTN_SD_CARD_PAGE_DOWN, STS_INVERSE);
        currPage = (int)(fileNum / 10.0) + 1;
        nextPage = (int)((numberOfFiles - 1) / 10.0) + 1;
        if (currPage < nextPage) {
          fileNum = currPage * 10;
        } else {
          fileNum = 0;
        }
        printFileList(fileNum);
        printButtonEx(BTN_SD_CARD_PAGE_DOWN, STS_NORMAL);
        continue;
      }
      // DELETE ALL
      if (isButtonPressed(BTN_SD_CARD_DELETE_ALL, x, y)) {
        beep(BEEP_HIGH);
        printButtonEx(BTN_SD_CARD_DELETE_ALL, STS_INVERSE);
        if (deleteFile(FILENUM_DELETE_ALL) == true) {
          saveEventToSdCard(F("SDCARD: All files deleted"));
          fileNum = 0;
        }
        printFileListWindow(fileNum);
        continue;
      }
      // DELETE
      if (isButtonPressed(BTN_SD_CARD_DELETE, x, y)) {
        if (fileList[fileNum]->attribute == '+') {
          beep(BEEP_LOW);
          continue;
        }
        beep(BEEP_HIGH);
        myGLCD.setColor(VGA_FUCHSIA);
        printButtonEx(BTN_SD_CARD_DELETE, STS_INVERSE);
        if (deleteFile(fileNum) == true) {
          sprintf(event, "SDCARD: File \"%s\" deleted", fileList[fileNum]->name);
          saveEventToSdCard(event);
          if (fileNum > 0) {
            fileNum--;
          }
        }
        printFileListWindow(fileNum);
        continue;
      }
      // CANCEL
      if (isButtonPressed(BTN_SD_CARD_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        break;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
  currScreenMode = printMainWindow(SCREEN_REDRAW);
}

void getNumberOfFiles(File dir) {
  File entry;

  while (true) {
    entry = dir.openNextFile();
    if (!entry) {
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
    entry = dir.openNextFile();
    if (!entry) {
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
      sprintf(fileList[numberOfFiles]->size, "%8u", entry.size());  // バイト単位
      fileUsage = fileUsage + entry.size();
    }
    numberOfFiles = numberOfFiles + 1;
    getFileNameAndSize(entry, level + 1);
    entry.close();
  }
}

int getFileList(void) {
  File root;

  if (!SD.begin(PIN_SDCARD_CS)) {
    return SD_CARD_NOT_PRESENT;
  }
  root = SD.open("/");
  if (!root) {
    return SD_OPEN_ERROR;
  }
  for (int i = 0; i < numberOfFiles; i++) {
    delete fileList[i];
  }
  numberOfFiles = 0;
  root.rewindDirectory();  // openNextFile() が参照するすファイル位置をディレクトリの最初のファイルに戻す
  getNumberOfFiles(root);
  fileList = new FILELIST *[numberOfFiles];
  fileUsage = 0;
  numberOfFiles = 0;
  root.rewindDirectory();  // openNextFile() が参照するすファイル位置をディレクトリの最初のファイルに戻す
  getFileNameAndSize(root, 0);
  root.close();
  return SD_SUCCESS;
}

void printFileList(int fileNum) {  // fileNum ファイルの通し番号(0～)
  int currPage;                    // 現在ページ(1～)
  int totalPage;                   // 総ページ(1～)
  int index;                       // 画面上の行位置(0～9)
  uint32_t volumesize;
  char buf[BUFFER_LEN_FULL];
  SdVolume volume;  // 総容量取得に使用

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
  currPage = (int)(fileNum / 10.0) + 1;
  index = fileNum % 10;
  int i = 0;
  int j;
  while (i < 10) {
    j = (currPage - 1) * 10 + i;
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
  totalPage = (int)((numberOfFiles - 1) / 10.0) + 1;
  sprintf(buf, "%02d/%02d", currPage, totalPage);
  myGLCD.print(buf, 24, 188);
  // SD.card.cardSize() return The number of 512 byte data blocks in the card or zero if an error occurs.
  // 16GBは正しく動作するも、32GBは正しく動作しない
  //volumesize = SD.card.cardSize() / 2LU; // 2(*512/1024)で割ることでキロバイト単位にする
  volumesize = 0;
  if (volume.init(card)) {
    volumesize = volume.blocksPerCluster();  // clusters are collections of blocks
    volumesize *= volume.clusterCount();     // we'll have a lot of clusters
    volumesize /= 2;                         // SD card blocks are always 512 bytes (2 blocks are 1KB)
  }
  if (volumesize != 0) {
    sprintf(buf, "%10lu", volumesize - fileUsage / 1024LU);  // 総容量-使用容量=空き容量(キロバイト単位)
  }
  myGLCD.print(buf, 216, 146);
  myGLCD.print(F("   kB Free"), 216, 160);

  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  sprintf(buf, "%2d%c%-13s%8s",
          fileList[fileNum]->num, fileList[fileNum]->attribute, fileList[fileNum]->name, fileList[fileNum]->size);
  myGLCD.print(buf, 24, 34 + index * 14);
}

void printFile(int currFileNum, int nextFileNum) {
  int currPage, nextPage;
  int currRow, nextRow;
  char buf[BUFFER_LEN_FULL];

  currPage = (int)(currFileNum / 10.0) + 1;
  nextPage = (int)(nextFileNum / 10.0) + 1;
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
  int x, y;
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
  printButtonEx(BTN_DELETE_FILE_OK, STS_NORMAL);
  printButtonEx(BTN_DELETE_FILE_CANCEL, STS_NORMAL);
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
  while (1) {
    if (offTimerInterval.check() == 1) {
      if (popupWindowOffTimer < POPUPWINDOWOFFTIMER_END) {
        popupWindowOffTimer++;
      } else {
        break;
      }
    }
    if (myTouch.dataAvailable()) {
      popupWindowOffTimer = POPUPWINDOWOFFTIMER_START;
      beep(BEEP_LOW);
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      // OK
      if (isButtonPressed(BTN_DELETE_FILE_OK, x, y)) {
        beep(BEEP_HIGH);
        //DELETE ALL
        if (fileNum == FILENUM_DELETE_ALL) {
          for (int i = 1; i < numberOfFiles; i++) {
            if (fileList[fileNum]->attribute == ' ') {
              if (strstr(fileList[i]->name, ".CSV") != NULL) {
                SD.remove(fileList[i]->name);
              }
            }
          }
          if (getFileList() == 0) {
            return true;
          } else {
            return false;
          }
        } else {
          return SD.remove(fileList[fileNum]->name);
        }
      }
      // CANCEL
      if (isButtonPressed(BTN_DELETE_FILE_CANCEL, x, y)) {
        beep(BEEP_HIGH);
        return false;
      }
    }
  }
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
}

void printFileListWindow(int fileNum) {
  printWindowFrame(16, 20, 296, 216);
  sdSts = getFileList();
  printFileList(fileNum);
  printButtonEx(BTN_SD_CARD_PAGE_UP, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_CURSOR_UP, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_CURSOR_DOWN, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_PAGE_DOWN, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_DELETE_ALL, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_DELETE, STS_NORMAL);
  printButtonEx(BTN_SD_CARD_CANCEL, STS_NORMAL);
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
void dateTime(uint16_t *date, uint16_t *time) {
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
  File file;
  char dataString[MAX_DATASTRING_LEN];
  char fileName[MAX_FILE_NAME_LEN];
  char buf[BUFFER_LEN_HALF];

  if (sdSts == SD_CARD_NOT_PRESENT) {
    return;
  }
  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  Time t = getClockTime();
  sprintf(fileName, "%04d%02d.CSV", t.yr, t.mon);
  // ファイルがない場合は、タイトル行を書き込み
  if (!SD.exists(fileName)) {
    file = SD.open(fileName, FILE_WRITE);
    file.println(
      "yyyy/mm/dd,hh:mm,"
      "Integrated(Wh),"
      "Sensor1(Wh),Sensor2(Wh),Sensor3(Wh),Total(Wh),"
      "Sensor1(Peak W),Sensor2(Peak W),Sensor3(Peak W),Total(Peak W)");
    file.close();
  }
  // ここからデータ行を書き込み
  file = SD.open(fileName, FILE_WRITE);
  if (!file) {
    sdSts = SD_OPEN_ERROR;
    printSdStatus(screenMode);
    return;
  }
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
  if (file.println(dataString) > 0) {
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
  file.close();
}

void saveEventToSdCard(String event, int screenMode, int printMode) {
  File file;
  char dataString[MAX_DATASTRING_LEN];
  char fileName[MAX_FILE_NAME_LEN];

  if (sdSts == SD_CARD_NOT_PRESENT) {
    return;
  }
  if (screenMode == SCREEN_CURRENT) {
    screenMode = currScreenMode;
  }
  strcpy(fileName, "EVENT.CSV");
  // ファイルがない場合は、タイトル行を書き込み
  if (!SD.exists(fileName)) {
    file = SD.open(fileName, FILE_WRITE);
    file.println("yyyy/mm/dd,hh:mm:ss,event");
    file.close();
  }
  // ここからデータ行を書き込み
  file = SD.open(fileName, FILE_WRITE);
  if (!file) {
    sdSts = SD_OPEN_ERROR;
    printSdStatus(screenMode, printMode);
    return;
  }
  sdSts = SD_WRITING;
  printSdStatus(screenMode, printMode);
  Time t = getClockTime();
  // データの文字数を増やす場合は、dataString変数のサイズを超えないようにする
  sprintf(dataString, "%04d/%02d/%02d,%02d:%02d:%02d,%s",
          t.yr, t.mon, t.date, t.hr, t.min, t.sec, event.c_str());
  if (file.println(dataString) > 0) {
    sdSts = SD_SUCCESS;
    printSdStatus(screenMode, printMode);
  } else {
    sdSts = SD_WRITE_ERROR;
    printSdStatus(screenMode, printMode);
  }
  file.close();
}

void saveHttpResponseToSdCard(String httpResponse, String datetime) {
  File file;

  file = SD.open("HTTP_RES.TXT", O_RDWR | O_CREAT | O_TRUNC);
  if (!file) {
    sdSts = SD_OPEN_ERROR;
    printSdStatus();
  } else {
    if ((file.println(httpResponse) == 0) || (file.println(datetime) == 0)) {
      sdSts = SD_WRITE_ERROR;
      printSdStatus();
    }
    file.close();
  }
}

void saveWattHourTimeperiodToSdCard(void) {
  //総電力量(kWh)の画面表示桁数が5桁のため
  //wattHourTimeperiodが99999000Wh(99999kWh)を上回ったら積算値をSDカードに保存して強制的にリセット
  if (wattHourTimeperiod > 99999000.0) {
    saveIntegratedValuesToSdCard(currScreenMode);
    saveEventToSdCard(F("SYSTEM: Integrated values reset (overflow)"));
  }

  //設定した時間間隔毎に積算値をSDカードに保存
  Time t = getClockTime();
  sprintf(currTime, "%02d:%02d", t.hr, t.min);
  myGLCD.setFont(SmallFont);
  char buf[20];
  sprintf(buf, "%s %s", prevTime, currTime);
  //printStringEx(buf, 168, 174, VGA_WHITE, VGA_BLACK);
  printStringEx(" ", 312, 174, VGA_WHITE, VGA_BLACK);
  if ((t.min == 0 || t.min == 30) && (strcmp(currTime, prevTime) != 0)) {
    saveDataToSdCard();
    strcpy(prevTime, currTime);
  }
  //計量日の00:00になったら積算値をSDカードに保存してリセット
  if ((t.date == eepromAutoResetDate) && (strcmp(currTime, "00:00") == 0)) {
    if (needsSaveIntegratedValues) {
      saveIntegratedValuesToSdCard(currScreenMode);
      saveEventToSdCard(F("SYSTEM: Integrated values reset"));
      needsSaveIntegratedValues = false;
    }
  }
}

void popupWindow(void) {
  int btnNum;
  btnNum = getButtonNum();
  switch (btnNum) {
    case BTN_RESET:
      reset();
      break;
    case BTN_DATE_TIME:
      adjustDateTime();
      break;
    case BTN_CONFIG:
      config();
      break;
    case BTN_SD_CARD:
      sdCard();
      break;
    case BTN_SCREEN_OFF:
      beep(BEEP_HIGH);
      if (eepromScreenOffMode == MODE_CLOCK) {
        currScreenMode = printMainWindow(SCREEN_CLOCK);
      } else {
        currScreenMode = printMainWindow(SCREEN_BLANK);
      }
      return;
      break;
    case BTN_SCREEN_ON:
      beep(BEEP_HIGH);
      currScreenMode = printMainWindow(SCREEN_ON);
      break;
    default:
      //何もしない
      break;
  }
  // メニューボタンを押して戻ってきたらSCREEN_BLANK(またはSCREEN_CLOCK)にするタイマーをリセットする
  if (btnNum != BTN_NOT_PRESSED) {
    screenOffTimer = SCREENOFFTIMER_START;
    if (hasBlackout && (screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_STOP)) {
      screenOffTimerInBlackout = SCREENOFFTIMERINBLACKOUT_START;
    }
    offTimerInterval.reset();
  }
}

void countupTimer(void) {
  if (offTimerInterval.check() == 1) {
    if (hasBlackout) {
      //停電中
      if ((screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_STOP)
          && (screenOffTimerInBlackout != SCREENOFFTIMERINBLACKOUT_END)) {
        screenOffTimerInBlackout++;
      }
    } else {
      //通電中
      if ((eepromScreenOffTimer != NEVER_SCREEN_OFF) && (currScreenMode == SCREEN_ON)) {
        screenOffTimer++;
      }
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
  turnOnScreen();
  myGLCD.InitLCD();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  clearScreen();
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setFont(BigFont);
  myGLCD.print(F("Watt Monitor"), 0, 0);
  myGLCD.setFont(SmallFont);
  myGLCD.print(VER, 0, 18);
  curX = 0;
  curY = 36;  //12ドット*3行=36ドット
  myGLCD.setColor(VGA_WHITE);
  pinMode(PIN_SDCARD_VDD, OUTPUT);
  digitalWrite(PIN_SDCARD_VDD, HIGH);
  // 各パラメーターの初期値
  eepromScreenOffTimer = DEFAULT_SCREEN_OFF_TIMER;
  eepromAutoResetDate = DEFAULT_AUTO_RESET_DATE;
  eepromCtVoltage[1] = DEFAULT_CT1_VOLTAGE;
  eepromCtVoltage[2] = DEFAULT_CT2_VOLTAGE;
  eepromCtVoltage[3] = DEFAULT_CT3_VOLTAGE;
  eepromVtPercentage = DEFAULT_VT_PERCENTAGE;
  eepromCtPercentage[1] = DEFAULT_CT1_PERCENTAGE;
  eepromCtPercentage[2] = DEFAULT_CT2_PERCENTAGE;
  eepromCtPercentage[3] = DEFAULT_CT3_PERCENTAGE;
  eepromSound = DEFAULT_SOUND;
  eepromScreenOffMode = DEFAULT_SCREEN_OFF_MODE;
  eepromAutoTimeSync = DEFAULT_AUTO_TIME_SYNC;
  eepromTimeSyncHour = DEFAULT_TIME_SYNC_HOUR;
  eepromTimeSyncMin = DEFAULT_TIME_SYNC_MIN;
  eepromBarGraphFullscale = DEFAULT_BAR_GRAPH_FULLSCALE;
  // EEPROM内のデータが有効であれば各パラメーターをEEPROM内のデータで上書きする
  value = EEPROM.read(EEPROM_SCREEN_OFF_TIMER);
  if ((value >= 0) && (value <= 99)) eepromScreenOffTimer = value;
  value = EEPROM.read(EEPROM_AUTO_RESET_DATE);
  if ((value >= 0) && (value <= 28)) eepromAutoResetDate = value;
  value = EEPROM.read(EEPROM_CT1_VOLTAGE);
  if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[1] = value;
  value = EEPROM.read(EEPROM_CT2_VOLTAGE);
  if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[2] = value;
  value = EEPROM.read(EEPROM_CT3_VOLTAGE);
  if ((value == 0) || (value == 100) || (value == 200)) eepromCtVoltage[3] = value;
  value = EEPROM.read(EEPROM_SOUND);
  if ((value == OFF) || (value == ON)) eepromSound = value;
  value = EEPROM.read(EEPROM_SCREEN_OFF_MODE);
  if ((value == MODE_CLOCK) || (value == MODE_BLANK)) eepromScreenOffMode = value;
#ifndef ENABLE_MODE_CLOCK
  value = MODE_BLANK;
#endif
  value = EEPROM.read(EEPROM_AUTO_TIME_SYNC);
  if ((value == OFF) || (value == ON)) eepromAutoTimeSync = value;
  value = EEPROM.read(EEPROM_TIME_SYNC_HOUR);
  if ((value >= 0) && (value <= 23)) eepromTimeSyncHour = value;
  value = EEPROM.read(EEPROM_TIME_SYNC_MIN);
  if ((value >= 0) && (value <= 59)) eepromTimeSyncMin = value;
  value = EEPROM.read(EEPROM_BAR_GRAPH_FULLSCALE);
  if ((value >= 1) && (value <= 49)) eepromBarGraphFullscale = value;
  // EEPROMデータの書込値255はデータ消去状態を表すため、有効なデータ範囲は1～254とする
  value = EEPROM.read(EEPROM_VT_PERCENTAGE);
  if ((value >= 1) && (value <= 254)) eepromVtPercentage = value;
  value = EEPROM.read(EEPROM_CT1_PERCENTAGE);
  if ((value >= 1) && (value <= 254)) eepromCtPercentage[1] = value;
  value = EEPROM.read(EEPROM_CT2_PERCENTAGE);
  if ((value >= 1) && (value <= 254)) eepromCtPercentage[2] = value;
  value = EEPROM.read(EEPROM_CT3_PERCENTAGE);
  if ((value >= 1) && (value <= 254)) eepromCtPercentage[3] = value;
  resetIntegratedValues();
  ds1302.writeProtect(false);
  ds1302.halt(false);
  Time t = ds1302.time();
  prevSec = t.min * 60 + t.sec;
  prevDate = t.date;
  elapsedDays = 1;
  setClockTime(t);
  setSdLastUpdatedTime();
  sprintf(resetTime, "%02d/%02d/%02d %02d:%02d", t.yr % 100, t.mon, t.date, t.hr, t.min);
  pinMode(PIN_SS, OUTPUT);
  SdFile::dateTimeCallback(&dateTime);  // ファイル書込時タイムスタンプ更新のため関数を登録
  //SDカードの状態確認
  sdSts = getFileList();
  if (sdSts != SD_SUCCESS) {
    printlnEx();
    printlnEx(sdStsMsg[sdSts]);
    printEx(F("If the screen touched, the system reset"));
    while (getButtonNum() == BTN_NOT_PRESSED)
      ;
    resetSystem();
    //ここへは到達しない
    abort();
  }
  currScreenMode = SCREEN_SETUP;
  saveEventToSdCard(F("SETUP : System started"), currScreenMode, PRINTMODE_LINE);
  existsFlashAir = false;
#ifdef FLASH_AIR
  //FlashAirの無線LAN機能の起動
  delay(5000);  //電源ONからFlashAirイニシャライズ開始までの待ち時間がないと処理に失敗する
  existsFlashAir = initFlashAir();
  if (existsFlashAir == true) {
    //日本標準時の取得
    isTimeSynchronized = syncTime();
    if (isTimeSynchronized) {
      sprintf(event, "SETUP : [%04lXh]Time synchronized", nextSequenceId - 1);
      saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
    } else {
      sprintf(event, "SETUP : [%04lXh]Time synchronization failed", nextSequenceId - 1);
      saveEventToSdCard(event, currScreenMode, PRINTMODE_LINE);
    }
  }
#endif
  delay(3000);
  screenOffTimer = SCREENOFFTIMER_START;
  screenOffTimerInBlackout = SCREENOFFTIMERINBLACKOUT_STOP;
  popupWindowOffTimer = POPUPWINDOWOFFTIMER_END;
  currScreenMode = printMainWindow(SCREEN_REDRAW);
  hasBlackout = false;
  hasBrownout = false;
  isResetFlashAirRequested = false;
  offTimerInterval.reset();

  //タイマー割り込み
  timer1_status = TIMER1_STS_IDLE;
  Timer1.initialize(1000000);      // 1000ms間隔で割り込み
  Timer1.disablePwm(8);            // PWM TIMER1A
  Timer1.disablePwm(30);           // PWM TIMER1B
  Timer1.attachInterrupt(timer1);  // 割り込み時に呼び出す関数を指定
}

void loop() {
  if (timer1_status == TIMER1_STS_IDLE) {
    timer1_status = TIMER1_STS_PRINT;
    popupWindow();  // ボタンにタッチした時の処理
    if (currScreenMode == SCREEN_ON) {
      myGLCD.setFont(SmallFont);
      printStringEx("                  ", 168, 174, VGA_WHITE, VGA_BLACK);
    }
    countupTimer();                    // 無操作時に画面表示をOFFにするタイマーを動かす
    setScreenMode();                   // 画面表示ON/OFF制御
    saveWattHourTimeperiodToSdCard();  // 設定した時間間隔(30分)毎にSDカードにデータを保存する
    incrElapsedDays();                 // 1日1回累積日数の加算
    syncTimeOnceADay();                // 1日1回RTCとシステム時刻を日本標準時に合わせる
    adjustIntlClockByRtc();            // 1分に1回(30秒の時)内蔵RTCの時刻をDS1302に合わせる
    switch (currScreenMode) {
      case SCREEN_ON:
        myGLCD.setFont(SmallFont);
        printStringEx("printing          ", 168, 174, VGA_WHITE, VGA_BLACK);
        printDateTime();            // 現在日時を表示
        printVoltAmpere();          // 電圧・電流を表示
        printWattHour();            // 設定した時間単位の電力量、ピーク電力を表示
        printWattHourTimeperiod();  // 時間帯別積算電力量を表示
        printBarGraph();            // 消費電力のバーグラフを表示
        printBlackout();            // 停電時の表示
        myGLCD.setFont(SmallFont);
        printStringEx("calculating       ", 168, 174, VGA_WHITE, VGA_BLACK);
        //printFreeMemory(168, 174);
        break;
      case SCREEN_CLOCK:
        printClockDateTime();
        printWattHourTimeperiod();
        printBarGraph();
        break;
      default:
        //何もしない
        break;
    }
    timer1_status = TIMER1_STS_CALCULATE;
  }
}
