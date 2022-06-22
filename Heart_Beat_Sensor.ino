#include <Adafruit_GFX.h>        //OLED libraries
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
String etatC;
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
const char* ssid = "AndroidAP";
const char* password = "dppw8204";
ESP8266WebServer server(80);
String ID ="Admin";
String Pass ="Admin";
bool Log = true;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,              
0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,  };

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00  };


void setup() {  
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display 0x3C D1xD2
  display.display();
  delay(3000);
  // Initialize sensor
  particleSensor.begin(Wire, I2C_SPEED_FAST); 
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on ( "/", handleRoot );
  server.begin();
}

void handleLogin(){
  String user;
  String password;
  String ValLog;
  ValLog = server.arg("LogOut");
  user = server.arg("user");
  password = server.arg("pwd");
  if (ValLog == "Deconnecter"){
    Log= true;}
  else if (ValLog == "sign-in"){
    if (user == ID && password == Pass){ 
      Log = false;}}
  }
  
  void handleRoot(){ 
  if ( server.hasArg("LogOut")){
    handleLogin();
  }
  if (Log){
    server.send ( 200, "text/html", LoginPage() );
    }else{
        server.send ( 200, "text/html", HomePage() );
    }
  }
void loop() {
  server.handleClient();
  
  long irValue = particleSensor.getIR();   
                                          
if(irValue > 7000){                                          
    display.clearDisplay();                                 
    display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);     
    display.setTextSize(2);                                   
    display.setTextColor(WHITE); 
    display.setCursor(50,0);                
    display.println("BPM");             
    display.setCursor(50,18);                
    display.println(beatAvg); 
    display.display();
    
  if (checkForBeat(irValue) == true)                      
  {
    display.clearDisplay();                               
    display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);    
    display.setTextSize(2);                               
    display.setTextColor(WHITE);             
    display.setCursor(50,0);                
    display.println("BPM");             
    display.setCursor(50,18);                
    display.println(beatAvg); 
    display.display();
    delay(100);

    long delta = millis() - lastBeat;                   
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);          

    if (beatsPerMinute < 255 && beatsPerMinute > 20)               
    {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE; //Wrap variable

      
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

}
  if (irValue < 7000){     
     beatAvg=0;
     display.clearDisplay();
     display.setTextSize(1);                    
     display.setTextColor(WHITE);             
     display.setCursor(30,5);                
     display.println("Please Place "); 
     display.setCursor(30,15);
     display.println(" can you please put your finger ");  
     display.display();
     }
     if (beatAvg >40 && beatAvg<50){
      etatC="vous etes sain(e) et saufe.";
     }else if (beatAvg<40){
      etatC="boire de l'eau regulierement (1,5 a 2 litres par jour).";
     }
}
String LoginPage(){
   String page = "<html lang=fr-FR><head><meta http-equiv='refresh' content='10'/>";
   page += "<style>";
   page += "body{margin:0; padding:0; font-family:Cursive;";
   page += " background:url('https://voyage-onirique.com/wp-content/uploads/2020/11/Paysage-fantasy.jpg') no-repeat; background: fill;}";
   page += ".login-box{width:280px; position:absolute; top:50%; left:50%; transform:translate(-50%,-50%);";
   page += "color:white; background-color:#282828; border-radius:5% 10%; opacity:0.8;}";
   page += ".login-box h1{float:center; font-size:40px; border-bottom:6px #4CAF50; margin-bottom:50px; padding:13px;}";
   page += ".textbox i{width:26px; float:left; text-align:center;}";
   page += ".textbox input{border:none; outline:none; background:none; color:black; font-size:18px; width:80%; float:left; margin:10px 0;}";
   page += ".btn{width:100%; background:none; border:2px solid #4CAF50; color:white; padding:5px; font-size:18px; cursor:pointer; margin:12px 0;}";
   page += ".textbox{width:100%; overflow:hidden; font-size:20px; padding:8px 0; margin:8px 0; border-bottom:1px solid #4CAF50}</style>";
   page += "<title>Commande LED de la carte ESP8266 D1 mini</title></head>";
   page += "<body><div class='login-box'><h1>Login</h1>";
   page += "<form action='/' method='POST'><div class='textbox'><input type='text' placeholder='Username' name='user'></div><br>";
   page += "<div class='textbox'><input type='text' placeholder='Password' name='pwd'></div><br>";
   page += "<input type='submit' name='LogOut' value='sign-in' onclick='return'></form></body></html>";
  return page;
  }

String HomePage(){
    String page = "<html lang=fr-Fr><head><meta http-equiv='refresh' content='10'/>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1.0' />";
    page += "<style src='https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap'>;";
    page += "*{box-sizing: border-box;margin: 0;padding: 0;}";
    page += "body {font-family: 'Roboto', sans-serif;display: flex;flex-direction: column;align-items: center;justify-content: center;height: 100vh;overflow: hidden;margin: 0;}";
    page += ".slider-container {position: relative;overflow: hidden;width: 100vw;height: 100vh;}";
    page += ".left-slide {height: 100%;width: 35%;position: absolute;top: 0;left: 0;transition: transform 1s ease-in-out;}";
    page += ".left-slide > div {height: 100%;width: 100%;display: flex;flex-direction: column;align-items: center;justify-content: center;color: white;}";
    page += ".left-slide h1{font-size: 40px;margin-bottom: 10px;margin-top: -30px;}";
    page += ".left-slide p{text-align: center;}";
    page += ".right-slide {height: 100%;position: absolute;top: 0;left: 35%;width: 65%;transition: transform 1s ease-in-out;}";
    page += ".right-slide > div {background-repeat:  no-repeat;background-size: cover;background-position: center center;height: 100%;width: 100%;}";
    page += ".deco {background-color: white;border: none;color: #aaa;cursor: pointer;font-size: 16px;padding: 15px;}";
    page += "</style>";
//----------Structure Page HTML------------
    page += "<body><div class='left-slide'>";
    page += "<div style='background-color:darkslategrey;'><h1>BMP</h1><br/>";
    page += "BMP:   ";
    page += beatAvg;
    page += "<br/><br/>";
    page += etatC;
    page += "<form action='/' method='POST'>";
    page += "<input type='submit' name='LogOut' value='Deconnecter' onclick='return'>";
    
    page += "</div></div><div class='right-slide'>";
    page += "<div class='Rs' style='background-image: url(https://i.pinimg.com/564x/df/2d/d7/df2dd75d401afc9a872bfe28e7cc3090.jpg);'></div>";
    page += "</div>";
    page += "<div class='action-buttons'>";
    page += "<div class='deco'></div></from>";
    page += "<button class='down-button'>";
    page += "<i class='fa-solid fa-down'></i></button>";
    page += "</div><div class='action-buttons'>";
    page += "<button class='up-button'>";
    page += "<i class='fa-solid fa-up'></i></button>";
    page += "</div></div></body></html>";
    return page;
    }
