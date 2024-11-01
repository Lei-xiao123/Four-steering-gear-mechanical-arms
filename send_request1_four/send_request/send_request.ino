#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>

#define LED 2 // LED灯连接到GPIO 2，用LED灯指示设备状态

#define ONENET_TOPIC_PROP_POST "$sys/" product_id "/" device_id "/thing/property/post"             // 设备属性上报请求
#define ONENET_TOPIC_PROP_SET "$sys/" product_id "/" device_id "/thing/property/set"               // 设备属性设置请求
#define ONENET_TOPIC_PROP_POST_REPLY "$sys/" product_id "/" device_id "/thing/property/post/reply" // 设备属性上报响应
#define ONENET_TOPIC_PROP_SET_REPLY "$sys/" product_id "/" device_id "/thing/property/set_reply"   // 设备属性设置响应
#define ONENET_TOPIC_PROP_FORMAT "{\"id\":\"%u\", \"version\":\"1.0\", \"params\":%s}"             // 设备属性格式模板

#define FREQ 50
#define CHANNEL 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define CHANNEL_3 3
#define RESOLUTION 8
#define SERVO 5
#define SERVO_1 2
#define SERVO_2 4
#define SERVO_3 15

const char *ssid = "TP-LINK_G";    // 替换成自己的WiFi名称
const char *password = "88888888"; // 替换成自己的WiFi密码

const char *mqtt_server = "mqtts.heclouds.com"; // MQTT服务器地址
const int mqtt_port = 1883;                     // MQTT服务器端口

#define product_id "4lF0z4M2J1"                                                                                                            // 替换成自己的产品
#define device_id "test"                                                                                                                   // 替换成自己的设备ID
#define token "version=2018-10-31&res=products%2F4lF0z4M2J1%2Fdevices%2Ftest&et=1744387714&method=md5&sign=%2BFLMW9brZHqbg7afprcbUw%3D%3D" // 替换成自己的token

int postMsgId = 0;          // 消息ID初始值为0
float longitude = 113.0031; // 经度
float latitude = 33.0163;   // 纬度
// 这里定义自己要传输的数据
const char *longitude_a = "114.0031";
const char *latitude_a = "33.0163";

WiFiClient espclient;           // 创建一个WiFiClient对象
PubSubClient client(espclient); // 创建一个PubSubClient对象
Ticker ticker;

void LED_Flash(int time);
void WiFiConnect();
void OneNet_Connect();
void OneNet_Prop_Post();
void callback(char *topic, byte *payload, unsigned int length);

int calculatePWM(int degree)
{

  int min_width = 0.5 / 20 * pow(2, RESOLUTION);
  int max_width = 2.5 / 20 * pow(2, RESOLUTION);

  return (max_width - min_width) * degree / 180 + min_width;
}

void setup()
{
  pinMode(LED, OUTPUT); // LED灯设置为输出模式
  Serial.begin(9600);   // 串口初始化，波特率9600，用于输出调试信息
  WiFiConnect();        // 连接WiFi

  // 建立 LEDC 通道0
  ledcSetup(CHANNEL, FREQ, RESOLUTION);
  // 关联 GPIO 口与 LEDC 通道
  ledcAttachPin(SERVO, CHANNEL);
  Serial.println("Servo 1 attached to channel 0");

  // 建立 LEDC 通道1
  ledcSetup(CHANNEL_1, FREQ, RESOLUTION);
  // 关联 GPIO 口与 LEDC 通道
  ledcAttachPin(SERVO_1, CHANNEL_1);
  Serial.println("Servo 2 attached to channel 1");

  // 设置 LEDC 通道2
  ledcSetup(CHANNEL_2, FREQ, RESOLUTION);
  ledcAttachPin(SERVO_2, CHANNEL_2);
  Serial.println("Servo 3 attached to channel 2");

  // 设置 LEDC 通道3
  ledcSetup(CHANNEL_3, FREQ, RESOLUTION);
  ledcAttachPin(SERVO_3, CHANNEL_3);
  Serial.println("Servo 4 attached to channel 3");

  // 设置所有舵机的初始角度
  int initialAngle = 90;
  ledcWrite(CHANNEL, calculatePWM(initialAngle));
  ledcWrite(CHANNEL_1, calculatePWM(initialAngle));
  ledcWrite(CHANNEL_2, calculatePWM(initialAngle));
  ledcWrite(CHANNEL_3, calculatePWM(initialAngle));
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // 如果WiFi连接断开，重新连接WiFi
    WiFiConnect();
  }

  if (!client.connected())
  {
    // 如果MQTT连接断开，重新连接OneNet
    OneNet_Connect();
  }

  // 保持MQTT连接
  client.loop();
}

void LED_Flash(int time)
{
  digitalWrite(LED, HIGH); // 点亮LED
  delay(time);             // 延时time
  digitalWrite(LED, LOW);  // 熄灭LED
  delay(time);             // 延时time
}

void WiFiConnect()
{
  WiFi.begin(ssid, password); // 连接WiFi
  while (WiFi.status() != WL_CONNECTED)
  {                 // 等待WiFi连接，WiFi.status()返回当前WiFi连接状态，WL_CONNECTED为连接成功状态
    LED_Flash(500); // LED闪烁，循环等待
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network"); // WiFi连接成功
  Serial.println(WiFi.localIP());                  // 输出设备IP地址
  digitalWrite(LED, HIGH);                         // 点亮LED
}

void OneNet_Connect()
{
  client.setServer(mqtt_server, mqtt_port); // 设置MQTT服务器地址和端口
  client.setCallback(callback);             // 设置回调函数
  if (client.connect(device_id, product_id, token))
  { // 连接到OneNet
    LED_Flash(500);
    Serial.println("Connected to OneNet!");
  }
  else
  {
    Serial.println("Failed to connect to OneNet!");
  }
  client.subscribe(ONENET_TOPIC_PROP_SET);        // 订阅设备属性设置请求
  client.subscribe(ONENET_TOPIC_PROP_POST_REPLY); // 订阅设备属性上报响应
}

void OneNet_Prop_Post()
{
  if (client.connected())
  {
    char parmas[256];
    char jsonBuf[256];
    sprintf(parmas, "{\"longitude\":{\"value\":%.4f},\"latitude\":{\"value\":%.4f}}", longitude, latitude);
    // 数据请求格式，详情看阿里云的官方文档https://open.iot.10086.cn/doc/v5/fuse/detail/922
    Serial.println(parmas);

    sprintf(jsonBuf, ONENET_TOPIC_PROP_FORMAT, postMsgId++, parmas);
    Serial.println(jsonBuf);

    if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
    {
      LED_Flash(500);
      Serial.println("Post property success!");
    }
    else
    {
      Serial.println("Post property failed!");
    }
  }
}
void callback(char *topic, byte *payload, unsigned int length)
{
  // 将payload转换为字符串
  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  // 解析JSON格式的消息
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);

  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // 获取命令
  if (doc["params"].containsKey("angle_1"))
  {
    int angle_1 = doc["params"]["angle_1"];
    if (angle_1 >= 0 && angle_1 <= 180)
    {
      ledcWrite(CHANNEL, calculatePWM(angle_1));
      Serial.print("Rotating servo 1 to ");
      Serial.print(angle_1);
      Serial.println(" degrees");
    }
    else
    {
      Serial.println("Invalid angle for servo 1 received");
    }
  }

  if (doc["params"].containsKey("angle_2"))
  {
    int angle_2 = doc["params"]["angle_2"];
    if (angle_2 >= 0 && angle_2 <= 180)
    {
      ledcWrite(CHANNEL_1, calculatePWM(angle_2)); // 使用正确的通道 CHANNEL_1
      Serial.print("Rotating servo 2 to ");
      Serial.print(angle_2);
      Serial.println(" degrees");
    }
    else
    {
      Serial.println("Invalid angle for servo 2 received");
    }
  }

  if (doc["params"].containsKey("angle_3"))
  {
    int angle_3 = doc["params"]["angle_3"];
    if (angle_3 >= 0 && angle_3 <= 180)
    {
      ledcWrite(CHANNEL_2, calculatePWM(angle_3));
      Serial.print("Rotating servo 3 to ");
      Serial.print(angle_3);
      Serial.println(" degrees");
    }
    else
    {
      Serial.println("Invalid angle for servo 3 received");
    }
  }

  if (doc["params"].containsKey("angle_4"))
  {
    int angle_4 = doc["params"]["angle_4"];
    if (angle_4 >= 0 && angle_4 <= 180)
    {
      ledcWrite(CHANNEL_3, calculatePWM(angle_4));
      Serial.print("Rotating servo 4 to ");
      Serial.print(angle_4);
      Serial.println(" degrees");
    }
    else
    {
      Serial.println("Invalid angle for servo 4 received");
    }
  }
}
