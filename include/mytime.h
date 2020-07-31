#ifndef my_time
#define my_time

#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <WiFiUdp.h>

// Define NTP properties
#define NTP_OFFSET 60 * 60            // In seconds
#define NTP_INTERVAL 60 * 1000        // In miliseconds
#define NTP_ADDRESS "es.pool.ntp.org" // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client_MQTT
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// time_t myUpdateTime ;
// time_t myUpdatePass ;

String TimeStr(time_t mytime)
{
  String date;

  // now format the Time variables into strings with proper names for month, day etc
  date = "";
  if (hour(mytime) < 10)
    date += "0";
  date += hour(mytime);
  date += ":";
  if (minute(mytime) < 10) // add a zero if minute is under 10
    date += "0";
  date += minute(mytime);
  date += ":";
  if (second(mytime) < 10) // add a zero if minute is under 10
    date += "0";
  date += second(mytime);
  return date;
}
String TimeStrShort(time_t mytime, bool h12) // h12 -> true 12h false 24h
{
  String date;
  const char *ampm[] = {"AM", "PM"};

  // now format the Time variables into strings with proper names for month, day etc
  date = "";
  if (!h12)
    date += hour(mytime);
  else
    date += hourFormat12(mytime);
  date += ":";
  if (minute(mytime) < 10) // add a zero if minute is under 10
    date += "0";
  date += minute(mytime);
  if (h12)
  {
    date += " ";
    date += ampm[isPM(mytime)];
  }
  return date;
}
String DateStrShort(time_t mytime)
{
  String date;

  // now format the Time variables into strings with proper names for month, day etc
  date = "";
  if (day(mytime) < 10)
    date += "0";
  date += day(mytime);
  date += "/";
  if (month(mytime) < 10) // add a zero if minute is under 10
    date += "0";
  date += month(mytime);
  date += "/";
  date += year(mytime) - 2000;
  return date;
}
String DateStr(time_t mytime)
{
  String date;
  const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

  // now format the Time variables into strings with proper names for month, day etc
  date = days[weekday(mytime) - 1];
  date += ",";
  date += months[month(mytime) - 1];
  date += " ";
  date += day(mytime);
  date += ",";
  date += year(mytime) - 2000;

  return date;
}

String TimeDateStr(time_t mytime)
{
  return DateStrShort(mytime) + ", " + TimeStr(mytime);
}

time_t millisOffLine = 0;
time_t myTimeOffline = 0;

time_t UTC2Local(time_t utc)
{
  //Central European Time (Frankfurt, Paris, Barcelona)
  TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 25, 60}; //Central European Summer Time
  TimeChangeRule CET = {"CET ", Last, Sun, Oct, 29, 0};   //Central European Standard Time

  Timezone myTime(CEST, CET);
  return myTime.toLocal(utc);
}

time_t myRealTime()
{
  // update the NTP client_MQTT and get the UNIX UTC timestamp
  time_t local, utc;
  if (timeClient.update())
  {
    unsigned long epochTime = timeClient.getEpochTime();
    // convert received time stamp to time_t object
    utc = epochTime;
    local = UTC2Local(utc);
    myTimeOffline = local;
    millisOffLine = millis() / 1000;
  }
  else
    local = myTimeOffline + millis() / 1000 - millisOffLine;
  return local;
}

time_t convertISO2DateTime(const String &calTimestamp)
{
  TimeElements tm;
  int yr, mnth, d, h, m, s;
  const char *tmp;
  if (calTimestamp.length() > 16)
    // tmp = "%4d-%2d-%2dT%2d:%2d:%06.2fZ";
    tmp = "%04d-%02d-%02dT%02d:%02d:%02d.000Z";
  else
    tmp = "%4d%2d%2dT%2d%2d%2dZ";
  sscanf(calTimestamp.c_str(), tmp, &yr, &mnth, &d, &h, &m, &s);
  tm.Year = yr - 1970;
  tm.Month = mnth;
  tm.Day = d;
  tm.Hour = h;
  tm.Minute = m;
  tm.Second = s;
  time_t t = makeTime(tm);
  return t;
}
String convertDateTime2ISO(time_t t)
{
  char buf[34];
  // sprintf(buf, "%02d-%02d-%02dT%02d:%02d:%06.2fZ", year(t), month(t), day(t), hour(t), minute(t), second(t));
  sprintf(buf, "%02d-%02d-%02dT%02d:%02d:%02d.000Z", year(t), month(t), day(t), hour(t), minute(t), second(t));
  String r = String(buf);
  return r;
}
void setup_time()
{
  timeClient.begin(); // Start the NTP UDP client_MQTT
  delay(500);
  Serial.print("updated time: ");
  Serial.print(TimeStrShort(myRealTime(), false));
  Serial.print(", ");
  Serial.println("done!");
}
#endif