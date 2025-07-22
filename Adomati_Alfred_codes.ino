#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

// Wi-Fi credentials
const char *ssid = "Alfred's A12";
const char *password = "12345678";

// Authentication
const char *ADMIN_USER = "Alfred";
const char *ADMIN_PASS = "12345678";

// LED pins
int ledPins[3] = {2, 4, 5};
bool ledState[3] = {false, false, false};
bool loggedIn = false;

WebServer server(80);

// HTML Header 
String htmlHeader(const String &title) {
  String h = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  h += "<title>" + title + "</title>";
  h += "<style>";
  h += "body{font-family:Segoe UI,sans-serif;margin:0;background:#f4f4f4;color:#333;text-align:center;}";
  h += "h1{background:#FF8C00;color:white;padding:20px;margin:0;}";
  h += ".container{padding:30px;}";
  h += "section{margin:30px auto;max-width:600px;border-radius:10px;background:white;box-shadow:0 0 10px rgba(0,0,0,0.1);padding:20px;}";
  h += "section h3{color:#FF8C00;}";
  h += "button{font-size:16px;padding:12px 25px;margin:10px;border:none;border-radius:8px;cursor:pointer;transition:all 0.3s ease;}";
  h += "button:hover{transform:scale(1.05);box-shadow:0 4px 12px rgba(0,0,0,0.2);}";
  h += ".on{background:#FF8C00;color:white;}";
  h += ".off{background:#555;color:white;}";
  h += ".logout{background:#999;color:white;font-size:14px;padding:10px 18px;margin-top:20px;}";
  h += ".status{font-weight:bold;color:#444;}";
  h += "@media(max-width:600px){button{width:90%;margin:12px auto;display:block;}}";
  h += "footer{margin-top:40px;font-size:14px;color:#777;border-top:1px solid #ccc;padding-top:10px;}";
  h += "</style></head><body>";
  return h;
}

// HTML Footer
String htmlFooter() {
  return "<footer>Developed by <strong>Alfred Systems</strong> | Group: Logical_Loopers</footer></body></html>";
}

// Login Page
String loginPage(const String &msg = "") {
  String html = htmlHeader("Login");
  html += "<h1>Login Form</h1><div class='container'>";
  if (msg.length()) html += "<p style='color:red;'>" + msg + "</p>";
  html += "<form method='POST' action='/login'>";
  html += "<input type='text' name='username' placeholder='Username' required><br><br>";
  html += "<input type='password' name='password' placeholder='Password' required><br><br>";
  html += "<input type='submit' value='Login' class='on'>";
  html += "</form></div>" + htmlFooter();
  return html;
}

// Dashboard Page
String controlPage() {
  String html = htmlHeader("Dashboard");
  html += "<h1>Lighting Dashboard</h1><div class='container'>";

  html += "<section><h3>All Lights Control</h3>";
  html += "<a href='/allon'><button class='on'>Turn ALL ON</button></a>";
  html += "<a href='/alloff'><button class='off'>Turn ALL OFF</button></a>";
  html += "</section>";

  html += "<section><h3>Individual Light Control</h3>";
  for (int i = 0; i < 3; i++) {
    html += "<p>Light " + String(i + 1) + ": ";
    html += "<span class='status'>" + String(ledState[i] ? "ON" : "OFF") + "</span><br>";
    html += "<a href='/on?led=" + String(i) + "'><button class='on'>ON</button></a>";
    html += "<a href='/off?led=" + String(i) + "'><button class='off'>OFF</button></a></p>";
  }
  html += "</section>";

  html += "<section>";
  html += "<a href='/logout'><button class='logout'>Logout</button></a>";
  html += "</section>";

  html += "</div>" + htmlFooter();
  return html;
}

// Authentication Check
bool ensureAuthed() {
  if (!loggedIn) {
    server.send(200, "text/html", loginPage());
    return false;
  }
  return true;
}

// Web Handlers
void handleRoot() {
  if (!ensureAuthed()) return;
  server.send(200, "text/html", controlPage());
}

void handleLogin() {
  if (server.method() == HTTP_POST) {
    String user = server.arg("username");
    String pass = server.arg("password");
    if (user == ADMIN_USER && pass == ADMIN_PASS) {
      loggedIn = true;
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      server.send(200, "text/html", loginPage("Invalid credentials."));
    }
  } else {
    server.send(200, "text/html", loginPage());
  }
}

void handleLogout() {
  loggedIn = false;
  server.send(200, "text/html", loginPage("You have been logged out."));
}

void handleOn() {
  if (!ensureAuthed()) return;
  int idx = server.arg("led").toInt();
  if (idx >= 0 && idx < 3) {
    digitalWrite(ledPins[idx], HIGH);
    ledState[idx] = true;
  }
  server.send(200, "text/html", controlPage());
}

void handleOff() {
  if (!ensureAuthed()) return;
  int idx = server.arg("led").toInt();
  if (idx >= 0 && idx < 3) {
    digitalWrite(ledPins[idx], LOW);
    ledState[idx] = false;
  }
  server.send(200, "text/html", controlPage());
}

void handleAllOn() {
  if (!ensureAuthed()) return;
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], HIGH);
    ledState[i] = true;
  }
  server.send(200, "text/html", controlPage());
}

void handleAllOff() {
  if (!ensureAuthed()) return;
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], LOW);
    ledState[i] = false;
  }
  server.send(200, "text/html", controlPage());
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// Auto Light Timer: 6:30 PM ON, 6:30 AM OFF
void autoLightingControl() {
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;

  if (hour == 18 && minute == 30) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[i], HIGH);
      ledState[i] = true;
    }
  }

  if (hour == 6 && minute == 30) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[i], LOW);
      ledState[i] = false;
    }
  }
}

// Setup Function
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: ");
  Serial.println(WiFi.localIP());

  configTime(3 * 3600, 0, "pool.ntp.org");  // Uganda timezone (UTC+3)

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/logout", handleLogout);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/allon", handleAllOn);
  server.on("/alloff", handleAllOff);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Web server started.");
}

// Loop Function
void loop() {
  server.handleClient();
  autoLightingControl();
}
