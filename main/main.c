#include "ESP_CRSF.h"
#include "esp_task_wdt.h"
#include "driver/ledc.h"
#include <esp_err.h>
#include <math.h>
#include <stdlib.h>
#include <esp_intr_alloc.h>
#include <iot_servo.h>
#include <hal/wdt_hal.h>

// Arbitrary
#define DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define LEDC_SPEED LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0

void main_thread(void *context)
{
  TickType_t xLastWakeTime;
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
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 75.0f));
    xTaskDelayUntil(&xLastWakeTime, 10);
    ESP_ERROR_CHECK(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, 115.0f));
    esp_task_wdt_reset();
    wdt_hal_feed(context);
    taskYIELD();
  }

  vTaskDelete(NULL);
}

void serial_task(void *serial_data)
{
}

void app_main()
{
  wdt_hal_context_t hal_context;
  TaskHandle_t handle = NULL;

  wdt_hal_init(&hal_context, WDT_MWDT0, 80, true);
  wdt_hal_enable(&hal_context);

  xTaskCreate(&main_thread, "servo-process", 1024, &hal_context, tskIDLE_PRIORITY + 1, &handle);

  if (handle != NULL)
  {
    ESP_ERROR_CHECK(esp_task_wdt_add(handle));
  }
}