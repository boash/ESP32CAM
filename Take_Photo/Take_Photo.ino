#include "Arduino.h"
#include "esp_camera.h"
#include "base64.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Glide-US-Resident";
const char* password = "ReplyPrismTaunt";
const char* api_url = "https://api.openai.com/v1/chat/completions";
const char* api_key = "put your key"; 
int pictureNumber = 0;

// ESP32-CAM pins configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// define the number of bytes you want to access
#define EEPROM_SIZE 1

void setup() {
  Serial.begin(115200);
 
  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
 if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 5;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 5;
    config.fb_count = 2;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("------Camera initialization failed------", err);
    return;
  }else{
    Serial.printf("------Camera initialization succeeded------");
  }

  // Connect Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("------WiFi connection succeeded------");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); 
    // Determine whether it is a camera instruction
    if (command == "TAKE") {

      camera_fb_t * fb = NULL;
  
      // Take Picture with Camera
      // fb = esp_camera_fb_get();  
      // delay(1000);//This is key to avoid an issue with the image being very dark and green. If needed adjust total delay time.
      fb = esp_camera_fb_get();  
      
      if(!fb) {
        Serial.println("Camera capture failed");
        return;
      }
      
      // Convert image data to base64 encoding
      String base64Image = base64::encode(fb->buf, fb->len);
      Serial.println(base64Image);

      // HTTP client initialization
      HTTPClient http;

      // DynamicJsonDocument doc(200000); // 440KB
      
      // doc["model"] = "gpt-4-vision-preview";

      // JsonArray messages = doc.createNestedArray("messages");
      // JsonObject message = messages.createNestedObject();
      // message["role"] = "user";
      // JsonArray content = message.createNestedArray("content");

      // JsonObject textContent = content.createNestedObject();
      // textContent["type"] = "text";
      // textContent["text"] = "What is in this image?";

      // JsonObject imageContent = content.createNestedObject();
      // imageContent["type"] = "image_url";
      // JsonObject imageUrl = imageContent.createNestedObject("image_url");
      // String imageUrlValue = "data:image/jpeg;base64," + base64Image;
      // imageUrl["url"] = imageUrlValue;
      // Serial.println(imageUrlValue);
      // String requestBody;
      // doc["max_tokens"] = 200;
      //serializeJson(doc, requestBody);
      
      String requestBody = "{";
      requestBody += "\"model\": \"gpt-4-vision-preview\",";
      requestBody += "\"messages\": [";
      requestBody += "{";
      requestBody += "\"role\": \"user\",";
      requestBody += "\"content\": [";
      requestBody += "{";
      requestBody += "\"type\": \"text\",";
      requestBody += "\"text\": \"What is in this image?\"";
      requestBody += "},";
      requestBody += "{";
      requestBody += "\"type\": \"image_url\",";
      requestBody += "\"image_url\": {";
      requestBody += "\"url\": \"data:image/jpeg;base64," + base64Image + "\"";
      requestBody += "}";
      requestBody += "}";
      requestBody += "]";
      requestBody += "}";
      requestBody += "],";
      requestBody += "\"max_tokens\": 100";
      requestBody += "}";
    

      Serial.println(requestBody);

      http.begin(api_url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", String("Bearer ")+api_key);

      int httpResponseCode = http.POST(requestBody);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end(); // end HTTP connection
      esp_camera_fb_return(fb);
    } 
      

      Serial.println("------Service has been end------");
    
  }
}