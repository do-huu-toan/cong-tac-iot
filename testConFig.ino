/*
   Code By Đỗ Hữu Toàn
*/
//Nếu không kết nối đc với wifi
int count = 0;
int count2 = 0; 
//Thời gian
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"2.asia.pool.ntp.org",7*3600);
//---------------------------------

const byte ResetPin = D7;
//-------------------------------------------------------------------------------------
#include <FirebaseESP8266.h> // Thư viện FireBase
#include <ESP8266WiFi.h>

// Cấu hình.
#define FIREBASE_HOST "internetcontrol-36b12.firebaseio.com"  //Đổi lại nhé
#define FIREBASE_AUTH ""
bool khoiTaoSTA = true;
//-----------------------------------------------------------------------------------
int buttonState = 0;
int lastButtonState = 0;
String trangThaiRelay = "";
const int relayPin = D2;
const int ledPin = D8;
//---------------------------------------------------------------------------------
const int buttonPin = D6;
unsigned long time1;
unsigned long time2 = 0;
//-----------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
//Thiết lập cho Access Point:
const char* ssid = "Toan_InternetOfThings";
const char* password = "12345678";
//Biến lưu SSID, PASS để kết nối
String connect_SSID;
String connect_PASS;
String ESSID = "";
String EPASS = "";
ESP8266WebServer server(80);
//Biến lưu số lượng điểm kết nối
int soLuongWiFi;
//Dữ liệu gửi lên websever
String GuiDuLieu = "";

const char ThietLap[] =
  "<!DOCTYPE HTML>"
  "<html>"
  "<head>"
  "<meta charset='utf-8'>"
  "<meta name = 'viewport' content = 'width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0'>"
  "<title>IoT - Đỗ Hữu Toàn</title>"
  "</head>"
  "<body>"
  "<center>"
  "<h1>Thiết lập cấu hình WiFi</h1>"
  "<FORM action='/' method='post'>"
  "<P>"
  "Tên WiFi: <INPUT type='text' name='ssid'"
  "<br></br>"
  "<br></br>"
  "Mật khẩu: <INPUT type='text' name='pass'"
  "<br></br>"
  "<br></br>"
  "<INPUT type='submit' value='Kết nối'>"
  "</P>"
  "<br><br><br>"
  "<hr  width='100%' align='center' /> "
  "<h3>Danh sách các điểm truy cập xung quanh:</h3>";


void handleRoot()
{
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", GuiDuLieu);
  }
}


void handleSubmit()
{
  if (server.hasArg("ssid") && server.hasArg("pass"))
  {
    connect_SSID = server.arg("ssid");
    connect_PASS = server.arg("pass");
  }
  Serial.println("SSID:" + connect_SSID);
  Serial.println("PASS:" + connect_PASS);
  server.send(200, "text/html", GuiDuLieu);
  GhiDataEEPROM(connect_SSID, connect_PASS);
}





void setup(void)
{
  pinMode(ResetPin, OUTPUT);
  digitalWrite(ResetPin, HIGH);
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);digitalWrite(relayPin,HIGH);
  pinMode(ledPin,OUTPUT);
  Serial.begin(9600);
  EEPROM.begin(512);
  Serial.println("");
  WiFi.mode(WIFI_AP);
  delay(1000);
  Serial.print(".");
  WiFi.softAP(ssid, password);
  delay(500);
  soLuongWiFi = WiFi.scanNetworks();

  GuiDuLieu += ThietLap;
  for (int i = 0; i < soLuongWiFi; i++)
  {
    GuiDuLieu += WiFi.SSID(i);
    GuiDuLieu += "<br>";
  }
  GuiDuLieu += "</center>";
  GuiDuLieu += "</FORM>";
  GuiDuLieu += "</body>";
  GuiDuLieu += "</html>";
  GuiDuLieu += "</html>";
  server.on("/", handleRoot);
  server.begin();
  Serial.print("Connect to http://");
  Serial.println(WiFi.softAPIP());
  DocEEPROM();

}

void loop(void)
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    Serial.println(c);
    if (c == 'a') {
      XoaEEPROM();
    }
  }


  if (ESSID != "")
  {
    if (khoiTaoSTA)
    {
      WiFi.mode(WIFI_STA);
      delay(500);
      WiFi.begin(ESSID.c_str(), EPASS.c_str());
      //Serial.print("connecting");
      while (WiFi.status() != WL_CONNECTED)
      {
        Serial.print(".");
        delay(500);
        count++;
        if(count >= 12)
        {
          XoaEEPROM();
          count = 0;
          ResetBoard();
        }
      }
      Serial.println();
      Serial.print("connected: ");
      Serial.println(WiFi.localIP());

      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      khoiTaoSTA = false;
    }
    // ĐỌC DỮ LIỆU TỪ FIREBASE
    if (Firebase.failed()) {
      count2++;
      if(count2 < 10)
      {
        return;
      }
      else
      {
        XoaEEPROM();
        ResetBoard();
      }
    }
    Serial.println(Firebase.getString("/IoT/led"));
    Serial.println(Firebase.getString("/IoT/Time"));
    trangThaiRelay = Firebase.getString("/IoT/led");
    String henGio = Firebase.getString("/IoT/Time");
    timeClient.update();
    String gioHienTai = "\""+String(timeClient.getHours())+":"+String(timeClient.getMinutes())+"\"";
    Serial.println(gioHienTai);
    if(gioHienTai == henGio)
    {
      Firebase.setString("IoT/led", "0");
      Firebase.setString("IoT/Time", "None");
      
    }
    if (trangThaiRelay == "1")
    {
      digitalWrite(relayPin, LOW);
      digitalWrite(ledPin,HIGH);
    }
    if (trangThaiRelay == "0")
    {
      digitalWrite(relayPin, HIGH);
      digitalWrite(ledPin,LOW);
    }
    //Nhấn nút----------------------------------------------------------------------------------------------------------------------------------
    buttonState = digitalRead(buttonPin);

    // so sánh với giá trị trước đó
    if (buttonState != lastButtonState) {
      if (buttonState == HIGH) {
        time1 = millis();
      }
      else {
        //Bật đèn
        if (trangThaiRelay == "1")  Firebase.setString("IoT/led", "0");
        if (trangThaiRelay == "0")  Firebase.setString("IoT/led", "1");

      }
    }
    // lưu lại trạng thái button cho lần kiểm tra tiếp theo
    lastButtonState = buttonState;
    if ((buttonState == HIGH) && (millis() - time1 > 5000))
    {
      XoaEEPROM();
      ResetBoard();
    }
    if (buttonState == LOW) time1 = millis();
    //------------------------------------------------------------------------------------------------
  }

  else
  {
    nhayLed();
    server.handleClient();

  }

}


void XoaEEPROM()
{
  Serial.println("Xoa EEPROM...");
  delay(500);
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
  delay(500);
}
void GhiDataEEPROM(String sssid, String passs)
{
  XoaEEPROM();
  EEPROM.begin(512);
  for (int i = 0; i < sssid.length(); i++)
  {
    EEPROM.write(i, sssid[i]);
  }
  for (int i = 0; i < passs.length(); i++)
  {
    EEPROM.write(32 + i, passs[i]);
  }
  EEPROM.commit();
  Serial.println("Da ghi du lieu thanh cong...");
  delay(500);
  ResetBoard();
}

void DocEEPROM()
{
  for (int i = 0; i < 32; i++)
  {
    ESSID += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; i++)
  {
    EPASS += char(EEPROM.read(i));
  }
}

void ResetBoard()
{
  ledReset();
  digitalWrite(ResetPin, LOW);
  delay(500);
  digitalWrite(ResetPin, HIGH);
}
void ledReset()
{
  digitalWrite(ledPin,HIGH);
  delay(50);
  digitalWrite(ledPin,LOW);
  delay(50);
}
inline void nhayLed()
{
  if ( (unsigned long) (millis() - time2) > 1000 )
    {
        if ( digitalRead(ledPin) == LOW )
        {
            digitalWrite(ledPin, HIGH);
        } else {
            digitalWrite(ledPin, LOW );
        }
        time2 = millis();
    }
}
