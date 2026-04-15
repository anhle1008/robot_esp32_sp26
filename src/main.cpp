#include <Arduino.h>   // Core Arduino functions like setup(), loop(), Serial, delay
#include <WiFi.h>      // ESP32 Wi-Fi features
#include <WebServer.h> // Simple built-in web server for ESP32
#include <Stepper.h>   // Arduino Stepper library

// --------------------------------------------------
// Wi-Fi access point settings
// Your phone or laptop will connect to this network
// --------------------------------------------------
const char *AP_SSID = "Team 9 Robot";
const char *AP_PASS = "12345678";

// --------------------------------------------------
// ULN2003 input pins connected to the ESP32
// --------------------------------------------------
#define IN1 21
#define IN2 19
#define IN3 18
#define IN4 5

// --------------------------------------------------
// Stepper motor configuration
// 28BYJ-48 is commonly treated as 2048 steps/revolution
// with this ULN2003 example setup.
// Pin order for this motor is commonly IN1, IN3, IN2, IN4.
// --------------------------------------------------
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// --------------------------------------------------
// Motor state variables
// --------------------------------------------------
bool motorEnabled = false; // true = keep spinning continuously
int directionSign = 1;     // 1 = clockwise, -1 = counterclockwise
int motorSpeedRPM = 10;    // between 0 and 15 rpm

// --------------------------------------------------
// Web server on port 80
// --------------------------------------------------
WebServer server(80);

// --------------------------------------------------
// Build the HTML page shown in the browser
// --------------------------------------------------
String htmlPage()
{
  String html;

  html += "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Stepper Control</title>";

  html += "<style>";
  html += "*{box-sizing:border-box;margin:0;padding:0;}";
  html += "body{";
  html += "font-family:Arial,sans-serif;";
  html += "background:linear-gradient(135deg,#0f172a,#1e293b,#334155);";
  html += "color:#f8fafc;";
  html += "min-height:100vh;";
  html += "display:flex;";
  html += "justify-content:center;";
  html += "align-items:flex-start;";
  html += "padding:24px;";
  html += "}";
  html += ".container{width:100%;max-width:760px;}";
  html += ".title{font-size:30px;font-weight:bold;text-align:center;margin-bottom:20px;}";
  html += ".subtitle{text-align:center;color:#cbd5e1;margin-bottom:28px;font-size:15px;}";
  html += ".card{";
  html += "background:rgba(255,255,255,0.08);";
  html += "backdrop-filter:blur(10px);";
  html += "border:1px solid rgba(255,255,255,0.12);";
  html += "border-radius:18px;";
  html += "padding:20px;";
  html += "margin-bottom:20px;";
  html += "box-shadow:0 8px 30px rgba(0,0,0,0.25);";
  html += "}";
  html += ".status-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:14px;}";
  html += ".stat-box{background:rgba(255,255,255,0.06);padding:14px;border-radius:14px;}";
  html += ".stat-label{font-size:13px;color:#cbd5e1;margin-bottom:6px;}";
  html += ".stat-value{font-size:20px;font-weight:bold;}";
  html += ".badge{display:inline-block;padding:6px 12px;border-radius:999px;font-size:14px;font-weight:bold;}";
  html += ".running{background:#16a34a;color:white;}";
  html += ".stopped{background:#dc2626;color:white;}";
  html += ".cw{background:#2563eb;color:white;}";
  html += ".ccw{background:#7c3aed;color:white;}";
  html += ".button-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:12px;}";
  html += "form{margin:0;}";
  html += "button{";
  html += "width:100%;";
  html += "border:none;";
  html += "border-radius:14px;";
  html += "padding:14px 18px;";
  html += "font-size:16px;";
  html += "font-weight:bold;";
  html += "cursor:pointer;";
  html += "transition:transform 0.15s ease, opacity 0.15s ease;";
  html += "}";
  html += "button:active{transform:scale(0.98);}";
  html += ".run-btn{background:#22c55e;color:white;}";
  html += ".stop-btn{background:#ef4444;color:white;}";
  html += ".cw-btn{background:#3b82f6;color:white;}";
  html += ".ccw-btn{background:#8b5cf6;color:white;}";
  html += ".speed-form{display:flex;flex-wrap:wrap;gap:12px;align-items:center;}";
  html += "label{font-size:15px;color:#e2e8f0;font-weight:bold;}";
  html += "input{";
  html += "flex:1;";
  html += "min-width:140px;";
  html += "padding:12px 14px;";
  html += "font-size:16px;";
  html += "border-radius:12px;";
  html += "border:1px solid rgba(255,255,255,0.2);";
  html += "background:rgba(255,255,255,0.08);";
  html += "color:white;";
  html += "outline:none;";
  html += "}";
  html += "input::placeholder{color:#cbd5e1;}";
  html += ".speed-btn{background:#f59e0b;color:#111827;}";
  html += ".footer{text-align:center;color:#94a3b8;font-size:13px;margin-top:10px;}";
  html += "@media(max-width:600px){";
  html += ".title{font-size:24px;}";
  html += ".stat-value{font-size:18px;}";
  html += "}";
  html += "</style></head><body>";

  html += "<div class='container'>";
  html += "<div class='title'>Team 9 Remote Control</div>";
  html += "<div class='subtitle'>ESP32 Stepper Motor Dashboard</div>";

  html += "<div class='card'>";
  html += "<div class='status-grid'>";

  html += "<div class='stat-box'>";
  html += "<div class='stat-label'>Status</div>";
  html += "<div class='stat-value'><span class='badge ";
  html += (motorEnabled ? "running'>Running" : "stopped'>Stopped");
  html += "</span></div>";
  html += "</div>";

  html += "<div class='stat-box'>";
  html += "<div class='stat-label'>Direction</div>";
  html += "<div class='stat-value'><span class='badge ";
  html += (directionSign == 1 ? "cw'>Clockwise" : "ccw'>Counterclockwise");
  html += "</span></div>";
  html += "</div>";

  html += "<div class='stat-box'>";
  html += "<div class='stat-label'>Speed</div>";
  html += "<div class='stat-value'>" + String(motorSpeedRPM) + " RPM</div>";
  html += "</div>";

  html += "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='button-grid'>";
  html += "<form action='/run' method='get'><button class='run-btn' type='submit'>Run</button></form>";
  html += "<form action='/stop' method='get'><button class='stop-btn' type='submit'>Stop</button></form>";
  html += "<form action='/clockwise' method='get'><button class='cw-btn' type='submit'>Clockwise</button></form>";
  html += "<form action='/counterclockwise' method='get'><button class='ccw-btn' type='submit'>Counterclockwise</button></form>";
  html += "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<form class='speed-form' action='/speed' method='get'>";
  html += "<label for='speedInput'>Speed (RPM)</label>";
  html += "<input id='speedInput' type='number' name='v' min='1' max='15' step='1' value='" + String(motorSpeedRPM) + "'>";
  html += "<button class='speed-btn' type='submit'>Set Speed</button>";
  html += "</form>";
  html += "</div>";

  html += "<div class='footer'>ESP32 Web Controller</div>";
  html += "</div>";

  html += "</body></html>";
  return html;
}

// --------------------------------------------------
// HTTP route handlers
// --------------------------------------------------
void handleRoot()
{
  server.send(200, "text/html", htmlPage());
}

void handleRun()
{
  motorEnabled = true;
  Serial.println("[+] Command received: Run");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop()
{
  motorEnabled = false;
  Serial.println("[+] Command received: Stop");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleClockwise()
{
  directionSign = 1;
  Serial.println("[+] Command received: Clockwise");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCounterclockwise()
{
  directionSign = -1;
  Serial.println("[+] Command received: Counterclockwise");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSpeed()
{
  if (server.hasArg("v"))
  {
    int v = server.arg("v").toInt();

    if (v < 1)
      v = 1;
    if (v > 15)
      v = 15;

    motorSpeedRPM = v;
    myStepper.setSpeed(motorSpeedRPM);

    Serial.print("[+] Command received: Set speed to ");
    Serial.print(motorSpeedRPM);
    Serial.println(" RPM");
  }

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not found");
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("[*] Starting the Robot ...");
  delay(1000);

  // Set the initial motor speed
  myStepper.setSpeed(motorSpeedRPM);
  motorEnabled = false;

  // Start the ESP32 as its own Wi-Fi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();

  Serial.println("[*] Wi-Fi access point started");
  Serial.print("[+] SSID: ");
  Serial.println(AP_SSID);
  Serial.print("[+] IP address: ");
  Serial.println(ip);
  Serial.println("[*] Open a browser to http://192.168.4.1");

  // Register all web routes
  server.on("/", handleRoot);
  server.on("/run", handleRun);
  server.on("/stop", handleStop);
  server.on("/clockwise", handleClockwise);
  server.on("/counterclockwise", handleCounterclockwise);
  server.on("/speed", handleSpeed);
  server.onNotFound(handleNotFound);

  // Start the web server
  server.begin();
  Serial.println("[*] Web server started");
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------
void loop()
{
  // Handle incoming browser requests
  server.handleClient();

  // Move only one step each loop so the website stays responsive
  if (motorEnabled)
  {
    myStepper.step(directionSign);
  }
}
