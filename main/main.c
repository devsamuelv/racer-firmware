#include "ESP_CRSF.h"
#include "esp_task_wdt.h"
#include "driver/ledc.h"
#include <esp_err.h>
#include <math.h>
#include <stdlib.h>
#include <esp_intr_alloc.h>
#include <iot_servo.h>

// Arbitrary
#define DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define LEDC_SPEED LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0

void main_thread(void *pvParameters)
{
  servo_config_t servo_config = {
      .max_angle = 180,
      .min_width_us = 500,
      .max_width_us = 2500,
      .freq = 50,
      .timer_number = LEDC_TIMER_0,
      .channels = {
          .servo_pin = {
              18,
          },
          .ch = {
              LEDC_CHANNEL_0,
          },
      },
      .channel_number = 1,
  };

  ESP_ERROR_CHECK(iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_config));

  for (;;)
  {
    ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 100.0f));
    esp_task_wdt_reset();
    taskYIELD();
  }

  vTaskDelete(NULL);
}

void task_two(void *pvParameters)
{
  for (;;)
  {
    printf("hello!");
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(100 / portTICK_PERIOD_MS));
  }
}

void app_main()
{
  TaskHandle_t handle = NULL;

  xTaskCreatePinnedToCore(&main_thread, "servo-process", 8192, NULL, tskIDLE_PRIORITY + 1, &handle, APP_CPU_NUM);

  if (handle != NULL)
  {
    ESP_ERROR_CHECK(esp_task_wdt_add(handle));
    printf("Task added to watchdog!");
  }
}