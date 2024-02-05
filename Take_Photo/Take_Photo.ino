#include "Arduino.h"
#include "esp_camera.h"
#include "base64.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "camera_pins.h"

const char* ssid = "Glide-US-Resident";
const char* password = "ReplyPrismTaunt";
const char* api_url = "https://api.openai.com/v1/chat/completions";
const char* api_key = "Put your API key";

void setup() {
  Serial.begin(115200);
  initCamera();
  connectToWiFi();
}

void loop() {
  checkForCommands();
}

void initCamera() {
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
// Configuration of captured images
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 5; // Set the image quality, the higher the value, the higher the quality
  config.fb_count = 1; // Setting the number of buffer frames

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("------Camera initialization failed------", err);
    return;
  }else{
    Serial.printf("------Camera initialization succeeded------");
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("------WiFi connection succeeded------");
}

void checkForCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); 
    // Determine whether it is a camera instruction
    if (command == "TAKE") {
      takePhotoAndAnalyze();
    } 
      Serial.println("------Service has been over------");
  }
}

void takePhotoAndAnalyze() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  // Photo function
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // // Observe the address and length of fb
  // Serial.print("Captured frame buffer length: ");
  // Serial.println(fb->len);
  // Serial.print("Frame buffer address: ");
  // Serial.println((uint32_t)fb->buf, HEX);
  // Convert image data to base64 encoding
  String base64Image = base64::encode(fb->buf, fb->len);  
  esp_camera_fb_return(fb); // Release the memory occupied by fb
  //Serial.println(base64Image); // Copy the Base64 encoding to this website to view the image: https://www.lddgo.net/convert/base64-to-image

  HTTPClient http; // HTTP client initialization

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

  // Send request header file
  http.begin(api_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ")+api_key);
  // Send request body file
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
}