#include "ESP_CRSF.h"
#include "driver/ledc.h"
#include "esp_task_wdt.h"
#include "hal/ledc_types.h"
#include "hal/uart_types.h"
#include <esp_err.h>
#include <esp_intr_alloc.h>
#include <hal/wdt_hal.h>
#include <iot_servo.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

// Arbitrary
#define DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define LEDC_SPEED LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0

// Supposted to return a ratio
float remap_channel_precent(int value) {
  if (value == 0) {
    return 0;
  }

  return (value - 922.0f) / 818.0f;
}

void main_thread(void *context) {
  TickType_t xLastWakeTime;
  servo_config_t servo_config = {
      .max_angle = 180,
      .min_width_us = 500,
      .max_width_us = 2500,
      .freq = 50,
      .timer_number = LEDC_TIMER_0,
      .channels =
          {
              .servo_pin =
                  {
                      18,
                  },
              .ch =
                  {
                      LEDC_CHANNEL_0,
                  },
          },
      .channel_number = 1,
  };

  crsf_config_t crsf_config = {
      .uart_num = UART_NUM_2,
      .tx_pin = 17,
      .rx_pin = 16,
  };

  // Initalize CRSF radio
  CRSF_init(&crsf_config);

  ESP_ERROR_CHECK(iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_config));
  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    crsf_channels_t channels = {0};

    CRSF_receive_channels(&channels);

    uint16_t skew_raw = channels.ch3;
    uint16_t skew_control = remap_channel_precent(skew_raw);

    ESP_ERROR_CHECK(iot_servo_write_angle(
        LEDC_LOW_SPEED_MODE, 0, ((115.0f - 75.0f) * skew_control) + 75.0f));

    // ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 75.0f));
    // xTaskDelayUntil(&xLastWakeTime, 10);
    // ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 115.0f));
    esp_task_wdt_reset();
    wdt_hal_feed(context);
    taskYIELD();
  }

  vTaskDelete(NULL);
}

void serial_task(void *serial_data) {}

void app_main() {
  wdt_hal_context_t hal_context;
  TaskHandle_t handle = NULL;

  wdt_hal_init(&hal_context, WDT_MWDT0, 80, true);
  wdt_hal_enable(&hal_context);

  xTaskCreate(&main_thread, "servo-process", 1024, &hal_context,
              tskIDLE_PRIORITY + 1, &handle);

  if (handle != NULL) {
    ESP_ERROR_CHECK(esp_task_wdt_add(handle));
  }
}