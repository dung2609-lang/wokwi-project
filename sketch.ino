#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h> 
// ======== THÔNG TIN KẾT NỐI HIVEMQ========
// Cấu hình Wi-Fi Wokwi 
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
// Cấu hình MQTT từ HiveMQ Cloud
const char* mqtt_server = "1166ccb0936d4acdb272601eeee9919d.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883; 
// TÀI KHOẢN ĐĂNG NHẬP HIVEMQ 
const char* mqtt_username = "DungNguyen"; 
const char* mqtt_password = "Anhdung123"; 
const char* mqtt_topic_pub = "esp32/motion/status"; 
// Định nghĩa Hardware và Biến
#define PIR_PIN 23      
#define LED_PIN 2       
#define BUZZER_PIN 15   
unsigned long lastMotionTime = 0;
const int TIMEOUT = 5000; // 5 giây
bool motionDetected = false;
// Khởi tạo Client
WiFiClientSecure espClient;
PubSubClient client(espClient);
// Khai báo hàm
void connectWiFi();
void reconnectMQTT();
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Thử kết nối với Username và Password
    if (client.connect("ESP32WokwiClient", mqtt_username, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 2 seconds");
      delay(2000); 
    }
  }
}
void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  // 1. Kết nối Wi-Fi
  connectWiFi();
  // 2. Thiết lập MQTT (setInsecure() cho phép dùng cổng 8883 dễ dàng trên Wokwi)
  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("Khởi động mô phỏng PIR...");
}
void loop() {
  // Đảm bảo kết nối MQTT và xử lý các tác vụ định kỳ
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop(); 

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    if (!motionDetected) { 
      motionDetected = true;
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 500);
      // PUBLISH: PHÁT HIỆN CHUYỂN ĐỘNG
      client.publish(mqtt_topic_pub, "MOTION_DETECTED_TRUE"); 
      
      Serial.println("Phát hiện chuyển động!");
      lastMotionTime = millis();
    }
  }
  // Tắt LED + buzzer sau timeout
  if (motionDetected && (millis() - lastMotionTime > TIMEOUT)) {
    motionDetected = false;
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    // PUBLISH: KẾT THÚC CHUYỂN ĐỘNG
    client.publish(mqtt_topic_pub, "MOTION_DETECTED_FALSE"); 
    Serial.println("Không phát hiện chuyển động.");
  }
  delay(100); 
}