// ================================================================
//  SMARTMETER — ESP32 LOCAL WEB SERVER
//  No Firebase. No cloud. Pure local WiFi.
//  ESP32 hosts a JSON API on its own IP address.
//  Website fetches from that IP directly.
// ================================================================
//  HOW IT WORKS:
//    1. ESP32 connects to your hostel WiFi
//    2. Router gives ESP32 an IP like 192.168.1.105
//    3. ESP32 runs a tiny web server on that IP
//    4. Your website fetches http://192.168.1.105/data
//    5. Gets live JSON → shows on dashboard
//  EVERYONE ON SAME WIFI CAN SEE IT — no internet needed
// ================================================================
//  LIBRARY TO INSTALL:
//    Arduino IDE → Tools → Manage Libraries → search:
//    "ESPAsyncWebServer" by lacamera — Install
//    "AsyncTCP" by dvarrel — Install
// ================================================================

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// ================================================================
//  ★ CHANGE ONLY THESE 2 LINES ★
// ================================================================
#define WIFI_SSID     "YOUR_HOSTEL_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
// ================================================================

// Room & sensor config — change names to your roommates
const String ROOM_NAMES[4] = {"Ravi", "Arjun", "Priya", "Adi"};
const String ROOM_IDS[4]   = {"room1","room2","room3","room4"};
const int    SENSOR_PINS[4]= {32, 33, 34, 35};

// Billing
const float MONTHLY_QUOTA = 300.0;   // units per person
const float RATE_PER_UNIT = 6.50;    // rupees per kWh

// Runtime data — updated every 1 second
float wattsNow[4]  = {0,0,0,0};
float ampsNow[4]   = {0,0,0,0};
float kwhToday[4]  = {0,0,0,0};
float kwhMonth[4]  = {0,0,0,0};
int   lastDay      = -1;
int   lastMonth    = -1;

// Web server on port 80
AsyncWebServer server(80);

unsigned long lastRead = 0;
const long    READ_MS  = 1000; // read sensors every 1 second

// ================================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n╔═══════════════════════════════╗");
  Serial.println("║  SmartMeter — Local IP Mode   ║");
  Serial.println("║  No cloud. Pure WiFi server.  ║");
  Serial.println("╚═══════════════════════════════╝\n");

  for(int i=0;i<4;i++) pinMode(SENSOR_PINS[i], INPUT);

  // Connect WiFi
  connectWiFi();

  // Sync time for date tracking (IST = UTC+5:30)
  configTime(19800, 0, "pool.ntp.org");
  Serial.print("Syncing time");
  while(time(nullptr) < 100000){ delay(500); Serial.print("."); }
  Serial.println(" OK");

  // ── SETUP WEB SERVER ROUTES ──────────────────────────────────

  // Route 1: /data — returns JSON of all 4 rooms
  // Your website fetches this every 3 seconds
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest* req){

    // Build JSON response
    StaticJsonDocument<1024> doc;
    doc["last_updated"] = getTimestamp();
    doc["total_watts"]  = wattsNow[0]+wattsNow[1]+wattsNow[2]+wattsNow[3];

    JsonObject summary = doc.createNestedObject("summary");
    float totalKwh = kwhMonth[0]+kwhMonth[1]+kwhMonth[2]+kwhMonth[3];
    summary["total_kwh"]  = round(totalKwh*100)/100.0;
    summary["total_bill"] = round(totalKwh*RATE_PER_UNIT*100)/100.0;

    JsonArray rooms = doc.createNestedArray("rooms");
    for(int i=0;i<4;i++){
      JsonObject r = rooms.createNestedObject();
      float left = max(0.0f, MONTHLY_QUOTA - kwhMonth[i]);
      float bill = kwhMonth[i] * RATE_PER_UNIT;
      float pct  = (kwhMonth[i] / MONTHLY_QUOTA) * 100.0;

      r["id"]         = ROOM_IDS[i];
      r["name"]       = ROOM_NAMES[i];
      r["watts_now"]  = round(wattsNow[i]*10)/10.0;
      r["amps_now"]   = round(ampsNow[i]*1000)/1000.0;
      r["kwh_today"]  = round(kwhToday[i]*10000)/10000.0;
      r["kwh_month"]  = round(kwhMonth[i]*10000)/10000.0;
      r["units_left"] = round(left*100)/100.0;
      r["bill_rs"]    = round(bill*100)/100.0;
      r["pct_used"]   = round(pct*10)/10.0;
      r["appliance"]  = wattsNow[i]>10?"ON":"STANDBY";
    }

    String json;
    serializeJson(doc, json);

    // CORS header — allows browser on any device to fetch this
    AsyncWebServerResponse* response =
      req->beginResponse(200, "application/json", json);
    response->addHeader("Access-Control-Allow-Origin","*");
    req->send(response);
  });

  // Route 2: /reset — resets monthly data (open in browser to trigger)
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest* req){
    for(int i=0;i<4;i++){ kwhMonth[i]=0; kwhToday[i]=0; }
    req->send(200,"text/plain","Monthly cycle reset OK");
    Serial.println("=== Monthly reset triggered via /reset ===");
  });

  // Route 3: /status — simple health check page
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest* req){
    String html = "<html><body style='font-family:monospace;background:#08090f;color:#00e5a0;padding:24px'>";
    html += "<h2>SmartMeter Online</h2>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Uptime: " + String(millis()/1000) + "s</p>";
    html += "<p>Data endpoint: <a href='/data' style='color:#a78bfa'>/data</a></p>";
    html += "<p>Reset endpoint: <a href='/reset' style='color:#ff6b35'>/reset</a></p>";
    html += "</body></html>";
    req->send(200,"text/html",html);
  });

  server.begin();
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.printf ("║  Web server running!                  ║\n");
  Serial.printf ("║  Open in browser:                     ║\n");
  Serial.printf ("║  http://%-30s║\n", (WiFi.localIP().toString()+"/data").c_str());
  Serial.printf ("║  http://%-30s║\n", (WiFi.localIP().toString()+"/status").c_str());
  Serial.println("╚═══════════════════════════════════════╝\n");
  Serial.println("Paste the IP above into the website 'Connect' box.\n");
  Serial.println("Room       | Watts  | Amps  | kWh/day | kWh/month | Left");
  Serial.println("-----------|--------|-------|---------|-----------|------");
}

// ================================================================
void loop() {
  unsigned long now = millis();
  if(now - lastRead >= READ_MS){
    lastRead = now;
    readAllSensors();
    checkReset();
  }
}

// ================================================================
void readAllSensors(){
  for(int i=0;i<4;i++){
    // Average 30 samples — reduces ADC noise
    float sumV=0;
    for(int s=0;s<30;s++){
      sumV += (analogRead(SENSOR_PINS[i])/4095.0)*3.3;
      delayMicroseconds(300);
    }
    float voltage = sumV/30.0;

    // ACS712 30A: zero=2.5V, sensitivity=66mV/A
    float amps = (voltage-2.5)/0.066;
    if(abs(amps)<0.05) amps=0;

    wattsNow[i] = abs(amps)*220.0;
    ampsNow[i]  = abs(amps);

    // 1 second tick → kWh increment
    float inc = (wattsNow[i]/1000.0)/3600.0;
    kwhToday[i]  += inc;
    kwhMonth[i]  += inc;

    float left = max(0.0f, MONTHLY_QUOTA-kwhMonth[i]);
    Serial.printf("%-10s | %6.1f | %5.3f | %7.4f | %9.4f | %4.1f\n",
      ROOM_NAMES[i].c_str(), wattsNow[i], ampsNow[i],
      kwhToday[i], kwhMonth[i], left);
  }
  Serial.println();
}

// ================================================================
void checkReset(){
  time_t now=time(nullptr);
  struct tm* t=localtime(&now);
  int d=t->tm_mday, m=t->tm_mon;

  if(lastDay!=-1 && d!=lastDay){
    Serial.println("=== Midnight reset: today kWh cleared ===");
    for(int i=0;i<4;i++) kwhToday[i]=0;
  }
  if(lastMonth!=-1 && m!=lastMonth){
    Serial.println("=== New month: full reset ===");
    for(int i=0;i<4;i++){ kwhMonth[i]=0; kwhToday[i]=0; }
  }
  lastDay=d; lastMonth=m;
}

// ================================================================
void connectWiFi(){
  Serial.printf("Connecting to: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries=0;
  while(WiFi.status()!=WL_CONNECTED && tries<40){
    delay(500); Serial.print("."); tries++;
  }
  if(WiFi.status()==WL_CONNECTED){
    Serial.printf("\n\nConnected! IP address: %s\n\n",
      WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi failed. Restarting...");
    delay(3000); ESP.restart();
  }
}

String getTimestamp(){
  time_t now=time(nullptr);
  struct tm* t=localtime(&now);
  char buf[32];
  strftime(buf,sizeof(buf),"%d-%m-%Y %H:%M:%S",t);
  return String(buf);
}
