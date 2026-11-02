#include <Arduino.h>
#include <driver/i2s.h>

// --------------------
// PIN DEFINITIONS
// --------------------
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 26
#define MOTOR_PIN 14

// --------------------
// AUDIO SETTINGS
// --------------------
#define SAMPLE_RATE 22050
#define BUFFER_SIZE 2048   // small chunk (we loop multiple times)

// 32-bit buffer (VERY IMPORTANT FIX)
int32_t i2s_buffer[BUFFER_SIZE];

// --------------------
// I2S SETUP
// --------------------
void setupI2S() {

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

// --------------------
// SETUP
// --------------------
void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
  setupI2S();
}

// --------------------
// LOOP
// --------------------
void loop() {

  size_t bytes_read = 0;

  // Read audio
  i2s_read(I2S_NUM_0,
           &i2s_buffer,
           sizeof(i2s_buffer),
           &bytes_read,
           portMAX_DELAY);

  // Send to PC (convert 32-bit → 16-bit)
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int16_t sample = i2s_buffer[i] >> 14;  // SCALE FIX
    Serial.println(sample);
  }

  // Receive prediction
  if (Serial.available() > 0) {

    char prediction = Serial.read();

    // 0 = car horn
    if (prediction == '0') {
      digitalWrite(MOTOR_PIN, HIGH);
      delay(200);
      digitalWrite(MOTOR_PIN, LOW);
      delay(200);
      digitalWrite(MOTOR_PIN, HIGH);
      delay(200);
      digitalWrite(MOTOR_PIN, LOW);
    }

    // 1 = dog bark
    else if (prediction == '1') {
      digitalWrite(MOTOR_PIN, HIGH);
      delay(400);
      digitalWrite(MOTOR_PIN, LOW);
    }

    // 2 = siren
    else if (prediction == '2') {
      for (int i = 0; i < 3; i++) {
        digitalWrite(MOTOR_PIN, HIGH);
        delay(150);
        digitalWrite(MOTOR_PIN, LOW);
        delay(150);
      }
    }
  }
}