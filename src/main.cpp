/*
接线说明:无

程序说明:烧录进源地的板子
        这是服务器端的程序,实现了客户端请求JSON数据
        本实例用于演示esp32S3的json数据通讯。
        操作测试本程序需要使用两台esp32s3开发板。其中一台为服务器端，一台为客户端。
        本程序为服务器程序，功能如下：
        1. 实时读取引脚 0 , 8 , 12 , 34的引脚输出
        2. 当有客户端请求信息时，将会通过http响应将引脚读数等信息发送给客户端。
          信息发送格式为json格式。
        3. 本程序使用了wifi.config对开发板的IP进行了配置。
        4.客户端可以请求服务器中的JSON数据中的某段数据,不用全部请求,通过访问请求不同的页面实现
        5.当按下BOOT按键时,客户端的灯灭,松开客户端的灯亮(可能会有时间延迟,我delay了2秒)


注意事项:当该程序只运行一点时,有可能不是他自己的问题,可能是客户端出现问题了,导致服务器无数据可处理,不运行
        json数据官网:https://arduinojson.org/
        在这可以进行数据解析,将JSON格式的数据转化为代码,
        不需要手动书写代码,但是需要自己写出JSON数据

        本例中服务器需要处理的JSON数据:
        {
          "info": {
            "name": "lingsou",
            "url": "www.bilibili.com",
            "email": "haoze20212021@outlook.com"
          },
          "digital_pin": {
            "digitPin": "digitPinValue",
            "BOOT" : "bootValue"
          },
          "analog_pin": {
            "analogPin": "analogPinValue",
            "capPin": "capPinValue"
          
          }
        }


函数示例:无

作者:灵首

时间:2023_4_5

*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WebServer esp32s3_webServe(80);//实例化一个网页服务的对象
WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi

//通过 ping ipconfig 方法实现找到以下设置参数
IPAddress local_IP(192, 168, 0, 123); // 设置ESP32s3-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP）

//这是需要获取数据的引脚 
#define BOOT 0    //这是按键引脚
#define analogPin 15  //这是模拟引脚
#define capPin 12   //这是电容式触摸引脚
#define digitPin 8 //这是数字引脚

static int bootValue;
static int analogPinValue;
static int capPinValue;
static int digitPinValue;



/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}



/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU1029","12345678");
  wifi_multi.addAP("haoze1029","12345678");
  wifi_multi.addAP("LINGSOU234","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/*
# brief 处理"/"页面的请求,会返回一个字符串(包含所有的JSON数据),以JSON数据为格式存储(其中的代码是在 https://arduinojson.org/ 网站生成的)
# param 无
# retval 返回一个字符串(JSON数据)
*/
String rootJson(){
  //应该是在建立静态空间存储建立的JSON文件
  StaticJsonDocument<256> doc;

  //这是第一个对象
  JsonObject info = doc.createNestedObject("info");//对象名
  info["name"] = "lingsou";   //对象中的键值对
  info["url"] = "www.bilibili.com";
  info["email"] = "haoze20212021@outlook.com";

  //这是第二个对象
  JsonObject digital_pin = doc.createNestedObject("digital_pin");
  digital_pin["digitPin"] = String(digitPinValue);  //键值对,但是值是通过读取引脚数据得来的
  digital_pin["BOOT"] = String(bootValue);

  //这是第三个对象
  JsonObject analog_pin = doc.createNestedObject("analog_pin");
  analog_pin["analogPin"] = String(analogPinValue);
  analog_pin["capPin"] = String(capPinValue);

  String output;

  //output是输出的JSON格式的字符串
  serializeJson(doc, output);

  //串口输出JSON数据 
  Serial.print("root JSON :\n");
  Serial.print(output);
  Serial.print("\n\n");

  //返回字符串
  return output;
}


/*
# brief 处理"/info"页面请求,会返回一个字符串(部分JSON数据),以JSON数据为格式存储(其中的代码是在 https://arduinojson.org/ 网站生成的)
# param 无
# retval 返回一个字符串(JSON数据)
*/
String infoJson(){
  //应该是在建立静态空间存储建立的JSON文件
  StaticJsonDocument<128> doc;

  //这是所有数据中的第一个对象
  JsonObject info = doc.createNestedObject("info");//对象名
  info["name"] = "lingsou";   //对象中的键值对
  info["url"] = "www.bilibili.com";
  info["email"] = "haoze20212021@outlook.com";

  String output;

  //output是输出的JSON格式的字符串
  serializeJson(doc, output);

  //串口输出JSON数据 
  Serial.print("infoJson is  :\n");
  Serial.print(output);
  Serial.print("\n\n");

  //返回字符串
  return output;

}



/*
# brief 处理"/digital_pin"页面请求,会返回一个字符串(部分数据),以JSON数据为格式存储(其中的代码是在 https://arduinojson.org/ 网站生成的)
# param 无
# retval 返回一个字符串(JSON数据)
*/
String digitalPinJson(){
  //应该是在建立静态空间存储建立的JSON文件
  StaticJsonDocument<128> doc;

  //这是所有数据中的第二个对象
  JsonObject digital_pin = doc.createNestedObject("digital_pin");
  digital_pin["digitPin"] = String(digitPinValue);  //键值对,但是值是通过读取引脚数据得来的
  digital_pin["BOOT"] = String(bootValue);

  String output;

  //output是输出的JSON格式的字符串
  serializeJson(doc, output);

  //串口输出JSON数据 
  Serial.print("digitalPinJson is  :\n");
  Serial.print(output);
  Serial.print("\n\n");

  //返回字符串
  return output;
}



/*
# brief 处理"/analog_pin"页面请求,会返回一个字符串(部分数据),以JSON数据为格式存储(其中的代码是在 https://arduinojson.org/ 网站生成的)
# param 无
# retval 返回一个字符串(JSON数据)
*/
String analogPinJson(){
  //应该是在建立静态空间存储建立的JSON文件
  StaticJsonDocument<128> doc;

  //这是第三个对象
  JsonObject analog_pin = doc.createNestedObject("analog_pin");
  analog_pin["analogPin"] = String(analogPinValue);
  analog_pin["capPin"] = String(capPinValue);

  String output;

  //output是输出的JSON格式的字符串
  serializeJson(doc, output);

  //串口输出JSON数据 
  Serial.print("analongPinJson is  :\n");
  Serial.print(output);
  Serial.print("\n\n");

  //返回字符串
  return output;
}



/*
# brief   处理服务器端的"/"页面的请求操作,根据开发板数据返回JSON信息
# param    无
# retval  无
*/
void handleRoot(){
  esp32s3_webServe.send(200,"application/Json",rootJson());
}



/*
# brief   处理服务器端的"/info"页面的请求操作,根据开发板数据返回JSON信息
# param    无
# retval  无
*/
void handleInfo(){
  esp32s3_webServe.send(200,"application/Json",infoJson());
}



/*
# brief   处理服务器端的"/digital_pin"页面的请求操作,根据开发板数据返回JSON信息
# param    无
# retval  无
*/
void handleDigitalPin(){
  esp32s3_webServe.send(200,"application/Json",digitalPinJson());
}



/*
# brief   处理服务器端的"/analog_pin"页面的请求操作,根据开发板数据返回JSON信息
# param    无
# retval  无
*/
void handleAnalogPin(){
  esp32s3_webServe.send(200,"application/Json",analogPinJson());
}



/*
# brief  网络服务器的初始化,建立四个页面并给出相应的处理函数,"/","/info","/digital_pin","/analog_pin";
# param 无
# retval 无
*/
void esp32s3_webServe_init(){
  esp32s3_webServe.on("/",handleRoot);
  esp32s3_webServe.on("/info",handleInfo);
  esp32s3_webServe.on("/digital_pin",handleDigitalPin);
  esp32s3_webServe.on("/analog_pin",handleAnalogPin);
  esp32s3_webServe.begin();
  Serial.print("HTTP Serve begin successfully!!!\n");
}



void setup() {
  // 启动串口通讯
  Serial.begin(9600);          
  Serial.println("serial is OK !!!\n");
  
  //设置引脚模式
  pinMode(BOOT,INPUT_PULLUP);
  pinMode(analogPin,INPUT_PULLUP);
  pinMode(capPin,INPUT_PULLUP);
  pinMode(capPin,INPUT_PULLUP);

  //设置开发板的网络环境
  if(!WiFi.config(local_IP,gateway,subnet)){
    Serial.print("Failed to config the esp32s3 ip!!!\n");
  }

  //WiFi连接设置
  wifi_multi_init();//储存多个WiFi
  wifi_multi_con();//自动连接WiFi

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("connect wifi:  ");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("IP address:  ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //处理服务器更新函数
  esp32s3_webServe_init();
}



void loop() {
  //检查BOOT按键的状态,并及时处理
  bootValue = digitalRead(BOOT); 
  analogPinValue = analogRead(analogPin);
  capPinValue = analogRead(capPin);
  digitPinValue = digitalRead(digitPin);

  //保证服务器一直在工作能接收到来自客户端的信息
  esp32s3_webServe.handleClient();    //保证服务器一直在工作能接收到来自客户端的信息
}