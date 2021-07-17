# M5stack 目覚し時計，Google Calendar から自動的に予定を取得して時刻セット

<a href="https://github.com/sh1ura/M5stack-wakeup-alarm-set-from-google-calendar/blob/main/README.md">English version here</a><p>

[![](https://img.youtube.com/vi/843B_bA_Ixc/0.jpg)](https://www.youtube.com/watch?v=843B_bA_Ixc)

M5stack を用いた目覚まし時計です．その日の予定を取得して，それに合わせてアラームの時刻を自動設定します．

- １日毎の予定を取り出してアラームを設定します
- 予定からどれぐらい前にアラームを鳴らすのかを，カレンダーごとに設定できます
  * アラーム用のカレンダーを作り，0分に設定すると，直接時刻設定できます
  * 予定に関するカレンダーでは，朝食や移動に要する時間を分単位で設定してください
- Google Calendar の iCal を用いて予定を取得するので，Amazon AWS や Google クラウドプラットフォームなどを使用する必要がありません

<img src="https://user-images.githubusercontent.com/86639425/126030181-32f42968-63cd-4532-8cf5-e8fa0d19f7a3.jpeg" width="600">

## 設定

### 1. WiFi の SSID とパスワードの設定
```C
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 2. カレンダーの設定

```C
#define CALNUM 3 // number of google calendar

struct gcal {
  String name; // 各カレンダーの略称（M5stack の画面上での表示用）
  int margin;  // 予定の何分前にアラームを鳴らすか
  String url;  // google calendar の iCal アドレス
} cal[CALNUM] = { // Google calendar 設定
    { "Alarm   ", 0, 
      "https://calendar.google.com/calendar/ical/XXXXXXXXXXXXXXXXXXXXXXXXXgroup.calendar.google.com/public/basic.ics"
    },
    { "Meeting ", 120,
      "https://calendar.google.com/calendar/ical/YYYYYYYYYYYYYYYYYYYYYYYYYgroup.calendar.google.com/public/basic.ics"
    },
    { "Office  ", 120,
      "https://calendar.google.com/calendar/ical/ZZZZZZZZZZZZZZZZZZZZZZZZZgroup.calendar.google.com/public/basic.ics"
    }
};

#define ALARM_LENGTH 60 // アラームを鳴らす時間
```

<img src="https://user-images.githubusercontent.com/86639425/126030185-4327083a-90e8-40b8-a9ac-e1d714806395.jpeg" width="600">

- iCal の URL を設定する．上の #1 と #2 のどちらでもよいが，安定性のためには #1 推奨．#2 は大きすぎて処理できないことあり
  * URL #1 を使うときは，そのカレンダーが公開されていなければなりません．この場合，予定の表示を，詳細を隠す設定にするのが良いです

### 3. ntp と時間帯（時差）の設定

```C
#define TIMEZONE 9 // 時間帯（日本 : +9）
const char* ntpServer = "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 3600 * TIMEZONE; // 世界標準時からのずれ
const int   daylightOffset_sec = 0;
```

### 4. Arduino IDE でコンパイル・インストール

 - M5stack の標準的な設定（ライブラリ）で動くはずです
 
## 諸注意

- 目覚ましがならずに遅刻した！などのトラブルに関して一切責任を持てません．自己責任で使用してください．
- 毎正時に予定を取得した後，１分に１回，新しい予定を取り込み，より早い時刻の予定が現れたときにはアラームを変更しますが，安全のため，予定がなくなったり予定が遅くなっても時刻は繰り下げません．WiFi にアクセスできない場合などもアラーム時刻の解除はしません．ただし再起動すると取り込み直しになりますので，アラーム時刻が繰り下げ
られたり消えたりすることがあります．
