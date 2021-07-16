# M5stack wakeup alarm, automatically set from events in google calendar

M5stack app of wakeup alarm, time is automatically set from events in google calendar

- Time of wakeup alarm is automatically set from the events in the day.
- Time from the alarm to the event can be set independently to each calendar.
  * for 'alarm' calendar, set 0 min
  * for 'event' calendar, set necessary time for breakfast, move, etc.
- You do not need to use Amazon AWS or google cloud platform. It directly access iCal of google calendar.

![desc](https://user-images.githubusercontent.com/86639425/125974374-a5d1c232-e7c1-42a3-8b3e-16797ea01346.jpg)

## Settings

### 1. Set your WiFi SSID / password
```C
const char* ssid = "SSID_OF_YOUR_WIFI";         // ******** SET YOUR SSID ********
const char* password = "PASSWORD_OF_YOUR_WIFI"; // ******** SET YOUR WIFI PASSWORD ********
```

### 2. Set your calendar

```C
#define CALNUM 3 // number of accessed google calendars

String calendar[] = { // iCalendar address of google calendar
  "https://calendar.google.com/calendar/ical/xxxxxxxxxxxxxxxxxxxxxxxxxxgroup.calendar.google.com/public/basic.ics", // alarm time
  "https://calendar.google.com/calendar/ical/yyyyyyyyyyyyyyyyyyyyyyyyyygroup.calendar.google.com/public/basic.ics", // event calendar 1
  "https://calendar.google.com/calendar/ical/zzzzzzzzzzzzzzzzzzzzzzzzzzgroup.calendar.google.com/public/basic.ics"  // event calendar 2
};

String calName[] = { // short name of each calendar shown on display
  "Alarm   ", "Meeting ", "Office  "
};
```
- Set your iCal URL (either #1 or #2 can be used. I recommend #1 for stability)
  * If you use iCal URL #1, please make sure that the calendar is public. In this case, it is better to hide details to keep your privacy.
- Give short name to each calendar. It is shown on the display of M5stack
- Change number of accessed calendars to CALNUM

![desc2](https://user-images.githubusercontent.com/86639425/125977300-76b28b15-ead0-436c-83f3-ba58ab50cdd6.jpg)

### 3. ntp and timezone setting

```C
#define TIMEZONE 9 // 時間帯（Japan : +9） ******** SET YOUR TIMEZONE ********
const char* ntpServer = "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 3600 * TIMEZONE; // time difference in seconds
const int   daylightOffset_sec = 0;
```

### 4. Compile and install via Arduino IDE
