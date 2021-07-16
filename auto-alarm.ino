#include <WiFi.h>
#include <M5Stack.h>
#include <HTTPClient.h>
#include "time.h"

// ***************** settings *******************
// configuration of WiFi / WiFi の接続設定
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

#define CALNUM 3 // number of google calendar / 読み込む google カレンダーの数

struct gcal {
  String name; // short name of each calendar shown on display / それぞれのカレンダーの略称（表示用）
  int margin;  // time between alarm and event, in minutes / イベントの何分前に目覚ましを鳴らすか
  String url;  // iCalendar address of google calendar / google カレンダーの iCal のアドレス
} cal[CALNUM] = { // Google calendar settings
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

#define ALARM_LENGTH 60 // time to ring the alarm / 目覚ましを鳴らす秒数

//ntp (time server) config / ntp （時刻サーバ）の設定
#define TIMEZONE 9 // time zone / œ時間帯（Japan : +9）
const char* ntpServer = "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 3600 * TIMEZONE; // time difference in seconds / 時差（秒）
const int   daylightOffset_sec = 0;

// *************** basic settnings end ******************

HTTPClient http;

int brightness = 100;

// iCalendar time recognition / iCalender フォーマットの時刻読み込み
struct tm parseTime(String s) {
  struct tm t;

  t.tm_year = s.substring( 0, 4).toInt() - 1900;
  t.tm_mon  = s.substring( 4, 6).toInt() - 1;
  t.tm_mday = s.substring( 6, 8).toInt();
  t.tm_hour = s.substring( 9,11).toInt();
  t.tm_min  = s.substring(11,13).toInt();
  t.tm_sec  = s.substring(13,15).toInt();
  t.tm_isdst = 0;

  time_t tt = mktime(&t);
  tt += 3600 * TIMEZONE; // from GMT to localtime / 世界標準時から現地時間に変換
  t = *(localtime(&tt));
  return t;
}

#define WIFI_RETRY 20
// connect to WiFi / WiFi の接続
void initWiFi(void) {
  int retryCount = 0;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.print("Connecting");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
    retryCount++;
    if(retryCount == WIFI_RETRY) {
      M5.Power.reset();
    }
  }
  M5.Lcd.println("\nConnect");
  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());
}

void setup()
{
    M5.begin();
    delay(100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setBrightness(brightness);

    initWiFi();
}

// generate time string / 時刻だけの文字列の生成
String tm2time(struct tm t) {
  char str[100];

  sprintf(str, "%02d:%02d ", 
    t.tm_hour,
    t.tm_min);
  return String(str);
}

// generate date and time string / 日付・時刻の文字列生成
String tm2str(struct tm t) {
  char str[100];

  sprintf(str, "%02d/%02d/%02d %02d:%02d:%02d", 
    t.tm_year - 100, // 2021 --> 21
    t.tm_mon + 1,
    t.tm_mday,
    t.tm_hour,
    t.tm_min,
    t.tm_sec);
  return String(str);
}

// read google calendar iCal / google calendar の iCal を読み込む
String getSched(String url) {
  static int failcount; // number of failure of calendar access / カレンダーアクセス失敗回数
  String response = "";
    
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    response = http.getString();
    Serial.println("http.GET success : " + String(response.length()) + " bytes");
  }
  else {
    WiFi.begin(ssid, password); // retry connection / 再接続を試みる
  }
  http.end();
  return response;
}

#define FAILCOUNT_TO_RESET 10

void loop(){
  struct tm curtime;
  time_t now; // current time / 現在時刻
  time_t minSched = 0; // alarm time calculated from events / スケジュールから求めた，最も早いアラーム時刻
  static time_t alarmTime; // alarm time to be kept / アラームを鳴らす時刻．スケジュール取得失敗時は保持
  static int prev_get = -1; // previous calendar acquiring time / 前回カレンダー取得時刻
  static int prev_day = -1; // to detect date change / 日の変化の検出用
  static int alarmSet = false; // is alarm set / アラームがセットされているかどうか
  static int alarmStop = false; // is alarm stopped / アラームを手動で止めたかどうか
  static int failCount = 0;

  delay(100);
  if(!getLocalTime(&curtime)) { // get current time / 現在時刻の取得
    Serial.println("fail to get time");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    return;
  }
  M5.Lcd.setCursor(6,210);
  M5.Lcd.setTextSize(3);  
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.print(tm2str(curtime)); // show current time / 時刻の画面表示
  M5.Lcd.setTextSize(2);  

  // adjust brightness / 輝度調整
  M5.update();
  if(M5.BtnA.isPressed()) { 
    brightness = max(0, brightness - 10);
    M5.Lcd.setBrightness(brightness);
    delay(100);
  }
  else if(M5.BtnC.isPressed()) { 
    brightness = min(250, brightness + 10);
    M5.Lcd.setBrightness(brightness);
    delay(100);
  }
  // ring alarm / アラームを鳴らす
  now = mktime(&curtime);
  if(now > alarmTime && now < alarmTime + ALARM_LENGTH && alarmSet && !alarmStop) {
    M5.Speaker.begin();
    M5.Speaker.setVolume(2);
    for(int i = 0; i < 4; i++) {
      M5.Speaker.beep();
      delay(100);
      M5.Speaker.mute();
      delay(100);
    }
    if(M5.BtnB.isPressed()) { // stop by center button / 中央ボタンで停止
      alarmStop = true;
    }
    return; // do not acquire events when ring alarm / カレンダーの再取得はせずにアラームを鳴らす
  }
  
  if(prev_get + 60 > now) { // duration to get events / イベントを取得する間隔
    return;
  }
  prev_get = now;
  if(prev_day != curtime.tm_mday) { // date change / 日付が変わる
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // adjust by ntp / ntp 再取得
    prev_day = curtime.tm_mday;
    alarmStop = false; // set alarm / アラームを鳴るようにする
    alarmSet = false; // get new data / 新しいデータに更新
  }

  M5.Lcd.setCursor(0,0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);  
  M5.Lcd.setTextColor(YELLOW, BLACK);
  M5.Lcd.println("Events:");
  M5.Lcd.setTextColor(WHITE, BLACK);
  for(int calNo = 0; calNo < CALNUM; calNo++) {
    String response = getSched(cal[calNo].url);
    if(response.length() == 0 && alarmSet == false) { // if alarm is not set yet / アラーム未設定
      failCount++;
      if(failCount > FAILCOUNT_TO_RESET) {
        M5.Power.reset();
      }
      continue;
    }
    failCount = 0;
    M5.Lcd.print(cal[calNo].name);
    while(1) {
      int i = response.indexOf("DTSTART");
      if(i < 0) break;
      response = response.substring(i);
      int j = response.indexOf("\n");
      String dt = response.substring(8, j);
      response = response.substring(j);
      if(dt.charAt(0) != '2') continue; // ignore all day event / 終日のイベントは無視

      struct tm t = parseTime(dt); // read time string / 時刻の文字列を読み取り
      if(t.tm_year == curtime.tm_year && t.tm_mon == curtime.tm_mon && t.tm_mday == curtime.tm_mday) {
        // today's event / 当日の予定
        M5.Lcd.print(tm2time(t));
        time_t et = mktime(&t) - cal[calNo].margin * 60; // calc alarm time / アラームを鳴らす時刻を計算
        if(minSched == 0 || et < minSched) { // first or earlier event / 最初の１件 or これまでの時刻よりも早い
          minSched = et; // revise alarm time / アラーム時刻を更新
        }
        Serial.print("Today ");
        Serial.println(tm2str(t));
      }
    }
    M5.Lcd.print("\n");
  }
  if(minSched != 0) { // succeeded / 時刻取得成功
    alarmSet = true;
    alarmTime = minSched;
  }
  if(alarmSet) { // show alarm time / 時刻を画面表示
    M5.Lcd.setTextSize(3);  
    M5.Lcd.print("\n  ");
    M5.Lcd.setTextColor(BLUE, WHITE);
    M5.Lcd.print(" Alarm ");
    M5.Lcd.println(tm2time(*localtime(&alarmTime)));
  }
  M5.Lcd.setTextColor(WHITE, BLACK);
}
