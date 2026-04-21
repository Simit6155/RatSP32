#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= WIFI =================
const char* WIFI_SSID = "yourwifiSSID";
const char* WIFI_PASS = "yourwifipassword";
const char* PC_IP     = "ipOfTheDestination";
const uint16_t PC_PORT = 4444(custoomport);
const char* TOKEN = "supersecretpassword";

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define OLED_SDA 5
#define OLED_SCL 18

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= PINLER =================
#define BTN1_PIN 13
#define BTN2_PIN 12
#define BTN3_PIN 14
#define BTN4_PIN 27

#define LED1_PIN 19
#define LED2_PIN 22
#define LED3_PIN 21
#define LED4_PIN 23

#define BUZZER_PIN 4

// ================= ZAMANLAR =================
const uint32_t DEBOUNCE_MS      = 35;
const uint32_t DOUBLECLICK_MS   = 300;
const uint32_t LONGPRESS_MS     = 700;
const uint32_t VERYLONGPRESS_MS = 3000;
const uint32_t EXTREMEPRESS_MS  = 20000;

// ================= EVENTLER =================
enum ButtonEvent {
  EVENT_NONE,
  EVENT_SINGLE,
  EVENT_DOUBLE,
  EVENT_LONG,
  EVENT_VERYLONG,
  EVENT_EXTREME
};

struct ButtonState {
  uint8_t pin;
  bool stablePressed = false;
  bool lastReading = false;
  uint32_t lastDebounceTime = 0;
  uint32_t pressStartTime = 0;

  bool longFired = false;
  bool veryLongFired = false;
  bool extremeFired = false;

  uint8_t clickCount = 0;
  uint32_t firstClickTime = 0;
};

ButtonState btn1{BTN1_PIN};
ButtonState btn2{BTN2_PIN};
ButtonState btn3{BTN3_PIN};
ButtonState btn4{BTN4_PIN};

// ================= EKRAN DURUMU =================
String popupTitle = "";
String popupLine1 = "";
String popupLine2 = "";
uint32_t popupUntil = 0;

uint8_t menuPage = 0;
uint32_t lastPageSwitch = 0;

// ================= YARDIMCI =================
void allLedsOff() {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
}

void lightOnly(uint8_t pin) {
  allLedsOff();
  digitalWrite(pin, HIGH);
}

void beep(uint16_t ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void feedback(uint8_t ledPin, uint8_t times = 1, uint16_t onMs = 45, uint16_t offMs = 35) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(onMs);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(ledPin, LOW);
    if (i < times - 1) delay(offMs);
  }
}

bool sendCommandToPC(const String& cmd) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClient client;
  client.setTimeout(1000);

  if (!client.connect(PC_IP, PC_PORT)) {
    return false;
  }

  String packet = String(TOKEN) + "|" + cmd + "\n";
  client.print(packet);
  client.flush();
  client.stop();
  return true;
}

void showPopup(const String& title, const String& line1, const String& line2 = "", uint32_t ms = 1500) {
  popupTitle = title;
  popupLine1 = line1;
  popupLine2 = line2;
  popupUntil = millis() + ms;
}

// ================= OLED =================
void drawPopup() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.drawFastHLine(0, 14, 128, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(4, 4);
  display.println(popupTitle);

  display.setCursor(6, 22);
  display.println(popupLine1);

  display.setCursor(6, 38);
  display.println(popupLine2);

  display.display();
}

void drawMenuPage0() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);  display.println("RATSP32 MAIN CONTROL");
  display.setCursor(0, 14); display.println("1 = TAB");
  display.setCursor(0, 26); display.println("2 = ENTER");
  display.setCursor(0, 38); display.println("3 = ARROW DOWN");
  display.setCursor(0, 50); display.println("4 = ARROW UP");

  display.display();
}

void drawMenuPage1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);  display.println("DOUBLE CLICK");
  display.setCursor(0, 14); display.println("1x2 = YOUTUBE");
  display.setCursor(0, 26); display.println("2x2 = WHATSAPP");
  display.setCursor(0, 38); display.println("3x2 = NETFLIX");
  display.setCursor(0, 50); display.println("4x2 = Refresh");

  display.display();
}

void drawMenuPage2() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);  display.println("LONG PRESS");
  display.setCursor(0, 14); display.println("1 = VOLUME UP");
  display.setCursor(0, 26); display.println("2 = VOLUME DOWN");
  display.setCursor(0, 38); display.println("3 = EXIT");
  display.setCursor(0, 50); display.println("4 = Shutdown");

  display.display();
}

void drawMenuPage3() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);  display.println("EXTRA PRESS");
  display.setCursor(0, 14); display.println("1 = funny prank ");
  display.setCursor(0, 26); display.println("2 = Fake Deleted PC");
  display.setCursor(0, 50); display.println("3 = Download free RAM");

  display.display();
}

void drawHomeScreen() {
  if (millis() < popupUntil) {
    drawPopup();
    return;
  }

  if (millis() - lastPageSwitch > 3500) {
    lastPageSwitch = millis();
    menuPage = (menuPage + 1) % 4;
  }

  if (menuPage == 0) drawMenuPage0();
  else if (menuPage == 1) drawMenuPage1();
  else if (menuPage == 2) drawMenuPage2();
  else drawMenuPage3();
}

// ================= BOOT =================
void ledDance() {
  for (int round = 0; round < 2; round++) {
    digitalWrite(LED1_PIN, HIGH); delay(70); digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, HIGH); delay(70); digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, HIGH); delay(70); digitalWrite(LED3_PIN, LOW);
    digitalWrite(LED4_PIN, HIGH); delay(70); digitalWrite(LED4_PIN, LOW);
  }

  digitalWrite(LED1_PIN, HIGH);delay(100); digitalWrite(LED4_PIN, HIGH); delay(100);
  digitalWrite(LED1_PIN, LOW);  digitalWrite(LED4_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);delay(100); digitalWrite(LED3_PIN, HIGH); delay(100);
  digitalWrite(LED2_PIN, LOW);  digitalWrite(LED3_PIN, LOW);

  allLedsOff();
}

void drawBootFrame(const String& top, const String& mainText, int barWidth) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(26, 8);
  display.println(top);

  display.setTextSize(2);
  display.setCursor(14, 24);
  display.println(mainText);

  display.drawRect(14, 52, 100, 8, SSD1306_WHITE);
  if (barWidth > 0) display.fillRect(16, 54, barWidth, 4, SSD1306_WHITE);

  display.display();
}

void bootAnimation() {
  drawBootFrame("Made by", "RedSimit", 0);
  delay(1000);

  for (int w = 0; w <= 96; w += 12) {
    drawBootFrame("Booting", "RATSP32", w);
    delay(100);
  }

  ledDance();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(18, 20);
  display.println("WiFi baglaniyor...");
  display.display();
}

// ================= WIFI =================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    showPopup("WIFI CONNECTED", WiFi.localIP().toString(), "", 1800);
  } else {
    showPopup("WIFI FAILED", "Offline Mode", "", 1800);
  }
}

// ================= BUTTON =================
ButtonEvent updateButton(ButtonState& b) {
  bool reading = (digitalRead(b.pin) == LOW);

  if (reading != b.lastReading) {
    b.lastDebounceTime = millis();
    b.lastReading = reading;
  }

  if (millis() - b.lastDebounceTime > DEBOUNCE_MS) {
    if (reading != b.stablePressed) {
      b.stablePressed = reading;

      if (b.stablePressed) {
        b.pressStartTime = millis();
        b.longFired = false;
        b.veryLongFired = false;
        b.extremeFired = false;
      } else {
        if (!b.longFired && !b.veryLongFired && !b.extremeFired) {
          if (b.clickCount == 0) {
            b.clickCount = 1;
            b.firstClickTime = millis();
          } else if (b.clickCount == 1 && millis() - b.firstClickTime <= DOUBLECLICK_MS) {
            b.clickCount = 0;
            return EVENT_DOUBLE;
          }
        }
      }
    }
  }


  if (b.stablePressed && !b.longFired && millis() - b.pressStartTime >= LONGPRESS_MS) {
    b.longFired = true;
    b.clickCount = 0;
    return EVENT_LONG;
  } else if (b.stablePressed && b.longFired && !b.veryLongFired && millis() - b.pressStartTime >= VERYLONGPRESS_MS) {
    b.veryLongFired = true;
    b.clickCount = 0;
    return EVENT_VERYLONG;
  } else if (b.stablePressed && b.veryLongFired && !b.extremeFired && millis() - b.pressStartTime >= EXTREMEPRESS_MS) {
    b.extremeFired = true;
    b.clickCount = 0;
    return EVENT_EXTREME;
  }

  if (!b.stablePressed && b.clickCount == 1 && millis() - b.firstClickTime > DOUBLECLICK_MS) {
    b.clickCount = 0;
    return EVENT_SINGLE;
  }

  return EVENT_NONE;
}

// ================= ACTIONS =================
void handleDefaultAction(uint8_t btn, ButtonEvent ev) {
  bool sent = false;

  if (btn == 1 && ev == EVENT_SINGLE) {
    lightOnly(LED1_PIN);
    beep(40);
    sent = sendCommandToPC("BTN1_SINGLE_YOUTUBE");
    showPopup("BUTTON 1", "TAB", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 2 && ev == EVENT_SINGLE) {
    lightOnly(LED2_PIN);
    beep(40);
    sent = sendCommandToPC("BTN2_SINGLE_WHATSAPP");
    showPopup("BUTTON 2", "ENTER", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 3 && ev == EVENT_SINGLE) {
    lightOnly(LED3_PIN);
    beep(40);
    sent = sendCommandToPC("BTN3_SINGLE_NETFLIX");
    showPopup("BUTTON 3", "GOING DOWN", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 4 && ev == EVENT_SINGLE) {
    lightOnly(LED4_PIN);
    beep(40);
    sent = sendCommandToPC("BTN4_SINGLE_FIREFOX");
    showPopup("BUTTON 4", "GOING UP", sent ? "Command Sent" : "PC Offline");
  }

  else if (btn == 1 && ev == EVENT_DOUBLE) {
    lightOnly(LED1_PIN);
    beep(25); delay(35); beep(25);
    sent = sendCommandToPC("BTN1_DOUBLE_ENTER");
    showPopup("BUTTON 1 DOUBLE", "Opening YouTube . . . ", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 2 && ev == EVENT_DOUBLE) {
    lightOnly(LED2_PIN);
    beep(25); delay(35); beep(25);
    sent = sendCommandToPC("BTN2_DOUBLE_PLAYPAUSE");
    showPopup("BUTTON 2 DOUBLE", "Opening WhatsApp . . . ", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 3 && ev == EVENT_DOUBLE) {
    lightOnly(LED3_PIN);
    beep(25); delay(35); beep(25);
    sent = sendCommandToPC("BTN3_DOUBLE_ALTTAB");
    showPopup("BUTTON 3 DOUBLE", "Opening Netflix . . . ", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 4 && ev == EVENT_DOUBLE) {
    lightOnly(LED4_PIN);
    beep(25); delay(35); beep(25);
    sent = sendCommandToPC("BTN4_DOUBLE_REFRESH");
    showPopup("BUTTON 4 DOUBLE", "Refreshing the Page", sent ? "Command Sent" : "PC Offline");
  }


  else if (btn == 1 && ev == EVENT_LONG) {
    lightOnly(LED1_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sent = sendCommandToPC("BTN1_LONG_vUP");
    showPopup("BUTTON 1 LONG", "Volume Up", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 2 && ev == EVENT_LONG) {
    lightOnly(LED2_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sent = sendCommandToPC("BTN2_LONG_vDOWN");
    showPopup("BUTTON 2 LONG", "Volume Down", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 3 && ev == EVENT_LONG) {
    lightOnly(LED3_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sent = sendCommandToPC("BTN3_LONG_EXIT");  // FIX: was "BTN2_LONG_EXIT"
    showPopup("BUTTON 3 LONG", "Exiting . . . ", sent ? "Command Sent" : "PC Offline");
  }
  else if (btn == 4 && ev == EVENT_LONG) {
    lightOnly(LED4_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sent = sendCommandToPC("BTN4_LONG_SHUTDOWN");
    showPopup("BUTTON 4 LONG", "Shutdown PC", sent ? "Command Sent" : "PC Offline");
  }
}

void handleCustomAction(uint8_t btn, ButtonEvent ev) {
  if (btn == 1 && ev == EVENT_EXTREME){
    lightOnly(LED1_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sendCommandToPC("BTN1_EXTREME_FUNNYPRANK");
    showPopup("BUTTON 1 EXTREME", "Make a funny prank");
 }else if (btn == 2 && ev == EVENT_EXTREME){
  lightOnly(LED1_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sendCommandToPC("BTN2_EXTREME_FAKEERROR");
    showPopup("BUTTON 3 EXTREME", "Destroying PC");
 }else if (btn == 3 && ev == EVENT_EXTREME){
  lightOnly(LED1_PIN);
    beep(70); delay(45); beep(70); delay(45); beep(70);
    sendCommandToPC("BTN3_EXTREME_DOWNLOADRAM");
    showPopup("BUTTON 3 EXTREME", "Downloading RAM");
}
}

void handleAction(uint8_t btn, ButtonEvent ev) {
  handleDefaultAction(btn, ev);
  handleCustomAction(btn, ev);
}

// ================= SETUP / LOOP =================
void setup() {
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  allLedsOff();
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for (;;);
  }

  bootAnimation();
  connectWiFi();
  lastPageSwitch = millis();
}

void loop() {
  ButtonEvent e1 = updateButton(btn1);
  ButtonEvent e2 = updateButton(btn2);
  ButtonEvent e3 = updateButton(btn3);
  ButtonEvent e4 = updateButton(btn4);

  if (e1 != EVENT_NONE) handleAction(1, e1);
  if (e2 != EVENT_NONE) handleAction(2, e2);
  if (e3 != EVENT_NONE) handleAction(3, e3);
  if (e4 != EVENT_NONE) handleAction(4, e4);



  drawHomeScreen();
  delay(10);
}
