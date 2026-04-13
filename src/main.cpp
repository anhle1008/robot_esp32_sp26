#include <Arduino.h>    // Core Arduino functions like setup(), loop(), Serial, delay
#include <WiFi.h>       // ESP32 Wi-Fi features
#include <WebServer.h>  // Simple built-in web server for ESP32
#include <Stepper.h>    // Arduino Stepper library

// --------------------------------------------------
// Wi-Fi access point settings
// Your phone or laptop will connect to this network
// --------------------------------------------------
const char* AP_SSID = "Team 9 Robot";
const char* AP_PASS = "12345678";

// --------------------------------------------------
// ULN2003 input pins connected to the ESP32
// --------------------------------------------------
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

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
bool motorEnabled = false;  // true = keep spinning continuously
int directionSign = 1;      // 1 = clockwise, -1 = counterclockwise
int motorSpeedRPM = 5;      // same idea as your earlier Stepper test

// --------------------------------------------------
// Web server on port 80
// --------------------------------------------------
WebServer server(80);

// --------------------------------------------------
// Build the HTML page shown in the browser
// --------------------------------------------------
String htmlPage() {
  String html;

  html += "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Stepper Control</title>";

  html += "<style>";
  html += "body{font-family:Arial;margin:24px;max-width:700px}";
  html += "button{font-size:18px;padding:12px 16px;margin:6px}";
  html += "input{font-size:18px;padding:8px;margin:6px;width:160px}";
  html += ".card{border:1px solid #ccc;border-radius:10px;padding:16px;margin-bottom:20px}";
  html += "</style></head><body>";

  html += "<h2>Team 9 Remote Control</h2>";

  html += "<div class='card'>";
  html += "<p><b>Status:</b> ";
  html += (motorEnabled ? "Running" : "Stopped");
  html += "</p>";

  html += "<p><b>Direction:</b> ";
  html += (directionSign == 1 ? "Clockwise" : "Counterclockwise");
  html += "</p>";

  html += "<p><b>Speed:</b> " + String(motorSpeedRPM) + " RPM</p>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<form action='/run' method='get'><button type='submit'>Run</button></form>";
  html += "<form action='/stop' method='get'><button type='submit'>Stop</button></form>";
  html += "<form action='/clockwise' method='get'><button type='submit'>Clockwise</button></form>";
  html += "<form action='/counterclockwise' method='get'><button type='submit'>Counterclockwise</button></form>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<form action='/speed' method='get'>";
  html += "<label>Speed (RPM): </label>";
  html += "<input type='number' name='v' min='1' max='15' step='1' value='" + String(motorSpeedRPM) + "'>";
  html += "<button type='submit'>Set Speed</button>";
  html += "</form>";
  html += "</div>";

  html += "</body></html>";
  return html;
}

// --------------------------------------------------
// HTTP route handlers
// --------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleRun() {
  motorEnabled = true;
  Serial.println("[+] Command received: Run");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop() {
  motorEnabled = false;
  Serial.println("[+] Command received: Stop");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleClockwise() {
  directionSign = 1;
  Serial.println("[+] Command received: Clockwise");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCounterclockwise() {
  directionSign = -1;
  Serial.println("[+] Command received: Counterclockwise");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSpeed() {
  if (server.hasArg("v")) {
    int v = server.arg("v").toInt();

    if (v < 1) v = 1;
    if (v > 15) v = 15;

    motorSpeedRPM = v;
    myStepper.setSpeed(motorSpeedRPM);

    Serial.print("[+] Command received: Set speed to ");
    Serial.print(motorSpeedRPM);
    Serial.println(" RPM");
  }

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("[*] Starting the Robot ...");
  delay(1000);

  // Set the initial motor speed
  myStepper.setSpeed(motorSpeedRPM);
  motorEnabled = true;

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
void loop() {
  // Handle incoming browser requests
  server.handleClient();

  // Move only one step each loop so the website stays responsive
  if (motorEnabled) {
    myStepper.step(directionSign);
  }
}