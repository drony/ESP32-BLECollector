
HardwareSerial GPS(1); // uart 1
#define GPS_RX 39 // io pin number
#define GPS_TX 35 // io pin number
#define GPS_BAUDRATE 9600

#include <TinyGPS++.h> // https://github.com/mikalhart/TinyGPSPlus
TinyGPSPlus gps;

static unsigned long LastGPSChange = millis();
static unsigned long NoGPSSignalSince = 0;
static bool GPSHasFix = false;
static bool GPSHasDateTime = false;
static float GPSLat = 0.00;
static float GPSLng = 0.00;


static void GPSInit() {
  GPS.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX, GPS_TX);
  // todo: launch a task to check for GPS health
}


static void GPSRead() {
  while(GPS.available()) {
    gps.encode( GPS.read() );
  }
  if( gps.location.isValid() ) {
    GPSLat = gps.location.lat();
    GPSLng = gps.location.lng();
    GPSHasFix = true;
  } else {
    GPSLat = 0.00;
    GPSLng = 0.00;
  }
  if(gps.date.isUpdated() && gps.date.isValid() && gps.time.isValid()) {
    LastGPSChange = millis();
    GPSHasDateTime = true;
    NoGPSSignalSince = 0;
  } else {
    NoGPSSignalSince = millis() - LastGPSChange;
  }
}


static void setGPSTime( void * param ) {
  if(gps.date.isValid() && gps.time.isValid()) {
    DateTime UTCTime = DateTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    DateTime LocalTime = UTCTime.unixtime() + timeZone*3600;
    #if HAS_EXTERNAL_RTC
      RTC.adjust( LocalTime );
    #endif
    setTime( LocalTime.unixtime() );
    Serial.printf("Time adjusted to: %04d-%02d-%02d %02d:%02d:%02d\n",
      LocalTime.year(),
      LocalTime.month(),
      LocalTime.day(),
      LocalTime.hour(),
      LocalTime.minute(),
      LocalTime.second()
    );
    logTimeActivity(SOURCE_GPS, LocalTime.unixtime());
    lastSyncDateTime = LocalTime;
  } else {
    Serial.printf("Can't set GPS Time (no signal since %d seconds)\n", NoGPSSignalSince/1000);
  }
}
