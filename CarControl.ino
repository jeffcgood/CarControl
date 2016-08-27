#include <WiFi.h>
#include "rgb_lcd.h"
#include <TimerOne.h>

rgb_lcd lcd;

/*
 * The background color of the lcd
 */
const int colorR = 255;
const int colorG = 255;
const int colorB = 255;

/*
 * /the control pins of the motor
 */
const int DPinDir_l = 13;   //The pin to control the direction of the left motor
const int DPinPul_1 = 11;  //The pin to give a pulse of the left motor
const int DPinDir_2 = 12;   //The pin to control the direction of the right motor
const int DPinPul_2 = 10;  //The pin to give a pulse of the right motor

const int timerInterval = 10*1000;  //1ms的定时器周期
const int B = 3975;

/*
 * 数字输入输出口
 */
const int DPinRelay = 2;  //The relay
const int DPinLed = 3;  //The led
const int DPinBuzzer = 4;  //The relay
const int DPinRadar_left_trig = 6;  //超声波测距左边
const int DPinRadar_right_trig = 7;  //超声波测距右边
const int DPinRadar_left_echo = 8;  //超声波测距左边
const int DPinRadar_right_echo = 9;  //超声波测距右边

/*
 * 模拟量输入口
 */
const int APinTemperature = 0;  //The Temperature sensor
const int APinSound = 1;  //The Sound sensor
const int APinLight = 2;  //The Light sensor

int status = WL_IDLE_STATUS;//用来保存现在网络连接状态，初始为无连接
WiFiServer server(8080);//定义一个服务器的对象server，端口号8080

void setup() {
 Serial1.begin(9600); //初始化串口调试器
 Serial1.println("** Scan Networks **");
 int numSsid = WiFi.scanNetworks();
 Serial1.print("number of available:");
 Serial1.println(numSsid);
 
 showNetworks(numSsid);//查看附近网络详细信息

 while(status != WL_CONNECTED){
  Serial1.print("Attempting to connect to Networt named ");
  Serial1.println("Intel-Smart");
  status = WiFi.begin("Intel-Smart","21507229");
  delay(10000);
 }

 Serial1.print("SSID:");
 Serial1.println(WiFi.SSID());
 IPAddress ip = WiFi.localIP();
 Serial1.print("IP Address:");
 Serial1.println(ip);

 server.begin();//启动服务器

  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(6, 1);
  lcd.print("jeffc_good");

  pinMode(DPinDir_l,OUTPUT);
  pinMode(DPinPul_1,OUTPUT);
  pinMode(DPinDir_2,OUTPUT);
  pinMode(DPinPul_2,OUTPUT);
  pinMode(DPinRelay,OUTPUT);
  pinMode(DPinLed,OUTPUT);
  pinMode(DPinBuzzer,OUTPUT);
  pinMode(DPinRadar_left_trig,OUTPUT);
  pinMode(DPinRadar_right_trig,OUTPUT);
  pinMode(DPinRadar_left_echo,INPUT);
  pinMode(DPinRadar_right_echo,INPUT);

   Timer1.initialize(timerInterval); 
   Timer1.attachInterrupt(MotorControl); 
}

void loop() {
  int val = analogRead(APinTemperature);
  float resistance = (float)(1023-val)*10000/val;
  float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;
  lcd.setCursor(6, 0);
  lcd.print(temperature);
  //监听8080端口是否有客户端发送的请求
  WiFiClient client = server.available();
  if(client){
    //如果监听到客户端的请求，通过串行通信显示说有新的客户端
    Serial1.println("new client");
    String currentLine = ""; //定义一个字符串用来保存这些请求信息
    while ( client.connected()){
      if(client.available())
      {
        char c = client.read();//读取信息并将信息同通过串行通信显示在串口监视器中
        Serial1.write(c);
        if(c == '\n'){
          if(currentLine.length() == 0){
            HtmlFunction(temperature);
            break;
          }
          else{
            currentLine = "";
          }
         } else if(c!='\r'){
            currentLine += c;
            if(currentLine.endsWith("GET /On")){
              digitalWrite(DPinRelay,HIGH);
              digitalWrite(DPinDir_l,LOW);
              digitalWrite(DPinDir_2,HIGH);
            }
            if(currentLine.endsWith("GET /Off")){
              digitalWrite(DPinRelay,LOW);
            }
           // if(currentLine.endsWith("GET /Auto"))
           // {
           //   digitalWrite(DPinRelay,LOW);
           // }
           // if(currentLine.endsWith("GET /Manul"))
           // {
           //   digitalWrite(DPinRelay,LOW);
           // }
           // if(currentLine.endsWith("GET /Alarm")){
            //  digitalWrite(DPinRelay,LOW);
            //}
            if(currentLine.endsWith("GET /Front")){
              digitalWrite(DPinDir_l,LOW);
              digitalWrite(DPinDir_2,HIGH);
            }
            if(currentLine.endsWith("GET /Back")){
              digitalWrite(DPinDir_l,HIGH);
              digitalWrite(DPinDir_2,LOW);
            }
            if(currentLine.endsWith("GET /Left")){
              digitalWrite(DPinDir_l,HIGH);
              digitalWrite(DPinDir_2,HIGH);
            }
            if(currentLine.endsWith("GET /Right")){
              digitalWrite(DPinDir_l,LOW);
              digitalWrite(DPinDir_2,LOW);
            }
          }
        }
      }
      client.stop();
      Serial1.println("client disconnected");
    } 
  }

/*
 * This function show the detail information of the wifi envirionment.
 */
 void showNetworks(int numSsid)
 {
  for(int thisNet = 0;thisNet < numSsid; thisNet ++){
    Serial1.print(thisNet);
    Serial1.print(".");
    Serial1.print(WiFi.SSID(thisNet));
    Serial1.print(" ");
    Serial1.print("Signal:");
    Serial1.print(WiFi.RSSI(thisNet));
    Serial1.print(" dBm");
    Serial1.print(" ");
    Serial1.print("Encryption:");
    //调用此函数显示网络加密方式
    printEncryptionType(WiFi.encryptionType(thisNet));
    Serial1.println("");
  }
 }

 /*
  * 函数功能：将加密方式的编码对应到字符串并通过串口输出显示
  * 入口参数：thisType--加密方式编码
  * 出口参数：无
  */
void printEncryptionType(int thisType)
{
  switch(thisType)
  {
    case ENC_TYPE_WEP: Serial1.println("WEP");break;
    case ENC_TYPE_TKIP: Serial1.println("WPA");break;
    case ENC_TYPE_CCMP: Serial1.println("WPA2");break;
    case ENC_TYPE_NONE: Serial1.println("None");break;
    case ENC_TYPE_AUTO: Serial1.println("auto");break;
  }
}
void HtmlFunction(float temprature){
  server.println("<html>");
            server.println("<head>");
            server.println("<title>Intel Edison");
            server.println("</title>");
            server.println("<meta http-equiv=\"refresh\" content=\"10;url=/\">");
            server.println("</head>");
            server.println("<body>");
            server.println("<h1>Car Control System</h1>");
            server.println("<h2>Powered by Intel Edison</h2>");
            server.println("<a href=\" /On\">turnon</a> the System<br>");
            server.println("<a href=\" /Off\">turnoff</a> the System<br>");
            server.println("<a href=\" /Front\">Forward</a> the System<br>");
            server.println("<a href=\" /Back\">Backward</a> the System<br>");
            server.println("<a href=\" /Left\">Left</a> the System<br>");
            server.println("<a href=\" /Right\">Right</a> the System<br>");
            //server.println("<a href=\" /Auto\">Auto</a> the System<br>");
           // server.println("<a href=\" /Manul\">Manul</a> the System<br>");
            //server.println("<a href=\" /Alarm\">Alarm</a> the System<br>");
            server.println("The value of the Temperature is ");
            server.println(temprature);
            server.println("<br/>");
            server.println("</body>");
            server.println("<head>");
            server.println("<title>");
            server.println("</html>");
            server.println();
}

void MotorControl()
{
   digitalWrite(DPinPul_2, !digitalRead(DPinPul_2));
   digitalWrite(DPinPul_1, !digitalRead(DPinPul_1));
}

