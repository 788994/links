#include "../lib/WifiConnect.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_now.h>

// ESPNOW接收端的MAC地址
uint8_t broadcastAddress[] = {0x24,0xD7,0xEB,0x15,0x07,0x64};
// 发送结构体类型
struct ESP_message
{
  String temp;
} ESP_message;

const char *AP_NAME = "PocketCard"; // wifi名字
// 暂时存储wifi账号密码
char sta_ssid[32] = {0};
char sta_password[64] = {0};
// 配网页面代码
const char *page_html = "\
<!DOCTYPE html>\r\n\
<html lang='en'>\r\n\
<head>\r\n\
  <meta charset='UTF-8'>\r\n\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n\
  <title>Document</title>\r\n\
</head>\r\n\
<body>\r\n\
  <form name='input' action='/' method='POST'>\r\n\
        wifi名称: <br>\r\n\
        <input type='text' name='ssid'><br>\r\n\
        wifi密码:<br>\r\n\
        <input type='text' name='password'><br>\r\n\
        <input type='submit' value='保存'>\r\n\
    </form>\r\n\
</body>\r\n\
</html>\r\n\
";

const byte DNS_PORT = 53;        // DNS端口号
IPAddress apIP(192, 168, 18, 1); // esp8266-AP-IP地址
DNSServer dnsServer;             // 创建dnsServer实例
WebServer server(80);            // 创建WebServer

void handleRoot()
{ // 访问主页回调函数
  server.send(200, "text/html", page_html);
}

void handleRootPost()
{ // Post回调函数
  Serial.println("handleRootPost");
  if (server.hasArg("ssid"))
  { // 判断是否有账号参数
    Serial.print("got ssid:");
    strcpy(sta_ssid, server.arg("ssid").c_str()); // 将账号参数拷贝到sta_ssid中
    Serial.println(sta_ssid);
  }
  else
  { // 没有参数
    Serial.println("error, not found ssid");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid"); // 返回错误页面
    return;
  }
  // 密码与账号同理
  if (server.hasArg("password"))
  {
    Serial.print("got password:");
    strcpy(sta_password, server.arg("password").c_str());
    Serial.println(sta_password);
  }
  else
  {
    Serial.println("error, not found password");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }

  server.send(200, "text/html", "<meta charset='UTF-8'>保存成功"); // 返回保存成功页面
  delay(50);
  // 连接wifi
  connectNewWifi();
}

void initBasic()
{ // 初始化基础
  //    Serial.begin(115200);
  WiFi.hostname("PocketCard"); // 设置设备名
}

void initSoftAP()
{ // 初始化AP模式
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(AP_NAME))
  {
    Serial.println("ESP8266 SoftAP is right");
  }
}

void initWebServer()
{ // 初始化WebServer
  // server.on("/",handleRoot);
  // 上面那行必须以下面这种格式去写否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);      // 设置主页回调函数
  server.onNotFound(handleRoot);             // 设置无法响应的http请求的回调函数
  server.on("/", HTTP_POST, handleRootPost); // 设置Post请求回调函数
  server.begin();                            // 启动WebServer
  Serial.println("WebServer started!");
}

void initDNS()
{ // 初始化DNS服务器
  if (dnsServer.start(DNS_PORT, "*", apIP))
  { // 判断将所有地址映射到esp8266的ip上是否成功
    Serial.println("start dnsserver success.");
  }
  else
    Serial.println("start dnsserver failed.");
}

void connectNewWifi()
{
  WiFi.mode(WIFI_STA);
  // Serial.println("");
  WiFi.setAutoConnect(true);          // 设置自动连接
  WiFi.begin(sta_ssid, sta_password); // 连接上一次连接成功的wifi
  // Serial.println("");
  // Serial.print("oCnnect to wifi");

  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500 / portTICK_PERIOD_MS);
    count++;
    if (count > 30)
    { // 如果5秒内没有连上，就开启Web配网 可适当调整这个时间
      initSoftAP();
      initWebServer();
      initDNS();
      break; // 跳出 防止无限初始化
    }
    Serial.print(".");
  }
  // Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  { // 如果连接上 就输出IP信息 防止未连接上break后会误输出
    // Serial.println("WIFI Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); // 打印esp8266的IP地址
    server.stop();
  }
}

// 进入配网模式需要循环运行的函数
void WifiConnect()
{
  server.handleClient();
  dnsServer.processNextRequest();
}

// 退出配网模式需运行的函数
bool WifiDisConnect()
{
  server.stop();
  WiFi.mode(WIFI_OFF);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (WiFi.status() != WL_CONNECTED)
  {
    return true;
  }
  return false;
}

// 传递wifi名称
char *TransportSSID()
{
  return sta_ssid;
}
// 传递wifi密码
char *TransportKeyWord()
{
  return sta_password;
}

// 返回是否成功连上网络（用于判断wifi账号密码，或者信号是否稳定是使用）
bool IsConnectOK()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // 返回 wifi账号密码错误/wifi信号弱
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED)
  {
    // 返回 连接成功
    return true;
  }
  return false;
}

// 简单联网函数
bool WifiEasyConnect()
{
  WiFi.mode(WIFI_STA);
  uint16_t timer = 0;
  // IPAddress dns(8, 8, 8, 8);
  WiFi.begin(sta_ssid, sta_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    timer++;
    if (timer >= 20) // 防止死等
    {
      return false; // 返回连接失败提示
    }
  }
  Serial.println(WiFi.localIP());
  // WiFi.dnsIP(dns);
  return true; // 返回连接成功提示
}

// 简单的获取时间函数
void SyncTime()
{
}

// 获取SSID和Keywords
void GetWIFISSIDandKEY(char SSID[32], char KEY[64])
{
  strcpy(sta_ssid, SSID);
  strcpy(sta_password, KEY);
}

// ESP_NOW 获取ESP32 MAC唯一地址
String GetWIFIMACAddr()
{
  String MACadd = WiFi.macAddress();
  return MACadd;
}

// ESP_NOW 回调函数，可以知晓消息是否发送成功
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// ESP_NOW 开始发送消息前的准备
void ESPNOWSTART()
{
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // ESP_NOW 注册回调函数
  esp_now_register_send_cb(OnDataSent);
  // 注册通信频道
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.ifidx = WIFI_IF_STA;
  peerInfo.channel = 0;  //通道
  peerInfo.encrypt = false;//是否加密为False
         
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

//ESP_NOW 发送消息
void ESPNOW_SEND(){
  
  //发送信息到指定ESP32上
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESP_message, sizeof(ESP_message));
   
 //判断是否发送成功
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}

//ESP_NOW 获取待发送消息
void GET_ESPNOW_MSG(String temp){
  //Serial.println(temp);
  ESP_message.temp = temp;
  Serial.println(ESP_message.temp);
}