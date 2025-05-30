# WattMonitor
電力モニター  
<2023/1/31>訂正  
実体配線図と回路図  
・R11 200Ω -> 10kΩ  
・R12 10kΩ -> 200Ω  
回路図  
・C5,C6 0.1μF -> 22pF  
<2025/5/30>追加  
・メイン回路_AliExpress_LCD.jpg  
<2025/5/31>訂正  
・メイン回路.jpgをメイン回路_Aitendo_LCD.jpgに差し替え  

■ハード  
・Atmega1284P-PU 3.3V 12MHz bobuinoピンレイアウト  
・電流センサー　3回路まで  
・電圧検出用のトランス  
・タッチパネル対応のLCD  
・RTC(DS1302)  
・SDカードスロット（FlashAir使用可)  
・電源バックアップ回路
  
■機能  
・電力モニターの電源電圧  
・センサー毎の電流値  
・センサー毎の30分間の積算電力量  
・センサー毎の30分間の最大電力  
・前回リセット時からの各センサー合計の時間帯別積算電力量  
・各センサー合計の瞬間電力  
等表示  
・各種設定  
・SDカードに30分毎にデータを記録  
・SDカード内のファイル一覧を表示・ファイル削除  
・停電時は内蔵バッテリーに自動切替し継続動作  
  
FlashAir使用時は、  
・パソコン・スマホからWiFi越しにSDカードに記録されているデータを参照・コピー  
・1日1回インターネットから標準時刻を取得しRTCを修正  

■コンパイルに必要なその他のファイルについて(スケッチと同じフォルダへ入れておく)  
・FlashAir関係  
　iSdio.cpp, iSdio.h  
　本家のサイトは閉鎖されています。入手する場合はネットで検索してみてください。  
・SDカード関係  
　Sd2Card.h, Sd2CardExt.cpp,Sd2CardExt.h, Sd2PinMap.h,SdInfo.h  
　「iSDIO拡張コマンドの作成」に説明あり  
　https://flashair-developers.github.io/website/docs/tutorials/arduino/2.html  
・フォント関係  
　GroteskBold32x64.c  
　https://github.com/JavierVLAB/Calps/blob/master/GroteskBold32x64.c  
　SevenSeg_XXXL_Num.c  
　https://github.com/iot-playground/Arduino/blob/master/external_libraries/UTFT/SevenSeg_XXXL_Num.c  
　SixteenSegment32x48.c  
　https://github.com/joyqi/UTFT/blob/master/SixteenSegment32x48.c  
※その他必要なライブラリはスケッチ内にURLを記述しています。  
  
![本体前面](https://user-images.githubusercontent.com/114241917/209243269-b8c7d9c2-f591-4082-9b9e-081770475712.jpg)
![内部](https://user-images.githubusercontent.com/114241917/209243399-f9e89ccb-9ef8-4686-b888-825ebc829a69.jpg)
![センサー](https://user-images.githubusercontent.com/114241917/209243423-99bfbb07-b247-4e75-a755-934a447dcfdc.jpg)
