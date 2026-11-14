
/* USER CODE BEGIN Header */
/**
  * @file    main.c
  * @brief   STM32F401RE + BME280 + ESP8266 MQTT Publisher
  *
  * BME280:
  *   SCL -> PB8
  *   SDA -> PB9
  *   VCC -> 3.3V
  *   GND -> GND
  *
  * ESP8266:
  *   TX  -> PA10 (USART1_RX)
  *   RX  -> PA9  (USART1_TX)
  *   GND -> GND
  */
/* USER CODE END Header */

#include "main.h"
#include "bme280.h"
#include "ESPLib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Peripheral handles */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/* BME280 variables */
static struct bme280_dev bme;
static struct bme280_data sensor_data;

static uint8_t bme280_i2c_address =
    BME280_I2C_ADDR_PRIM;

static float temperature_c;
static float pressure_hpa;
static float humidity_pct;

static char mqtt_payload[160];

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);

void Error_Handler(void);

static int8_t BME280_I2C_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
);

static int8_t BME280_I2C_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
);

static void BME280_Delay_us(
    uint32_t period,
    void *intf_ptr
);

static void BME280_InitSensor(void);
static void ESP_MQTT_Init(void);


/* MAIN FUNCTION */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    /* Initialize BME280 */
    BME280_InitSensor();

    /* Initialize ESP8266 and MQTT */
    ESP_MQTT_Init();

    while (1)
    {
        /* Read BME280 sensor data */

        int8_t result = bme280_get_sensor_data(
            BME280_ALL,
            &sensor_data,
            &bme
        );

        if (result == BME280_OK)
        {
            /* Store sensor readings */

            temperature_c =
                (float)sensor_data.temperature;

            pressure_hpa =
                (float)(sensor_data.pressure / 100.0);

            humidity_pct =
                (float)sensor_data.humidity;


            /* Convert readings into JSON */

            (void)snprintf(
                mqtt_payload,
                sizeof(mqtt_payload),

                "{\"temperature\":%.2f,"
                "\"pressure_hpa\":%.2f,"
                "\"humidity\":%.2f}",

                (double)temperature_c,
                (double)pressure_hpa,
                (double)humidity_pct
            );


            /* Publish data through MQTT */

            MQTT_publishTopic(
                "stm32/nigamanth-project/data",
                mqtt_payload,
                QoS0
            );
        }

        /* Wait 2 seconds */

        HAL_Delay(2000);
    }
}


/* BME280 INITIALIZATION */

static void BME280_InitSensor(void)
{
    struct bme280_settings settings;

    int8_t result;

    memset(&bme, 0, sizeof(bme));

    /* Configure I2C interface */

    bme.intf_ptr = &bme280_i2c_address;

    bme.intf = BME280_I2C_INTF;

    bme.read = BME280_I2C_Read;

    bme.write = BME280_I2C_Write;

    bme.delay_us = BME280_Delay_us;


    /* Initialize BME280 */

    result = bme280_init(&bme);

    if (result != BME280_OK)
    {
        Error_Handler();
    }


    /* Configure oversampling */

    memset(&settings, 0, sizeof(settings));

    settings.osr_h = BME280_OVERSAMPLING_1X;

    settings.osr_p = BME280_OVERSAMPLING_1X;

    settings.osr_t = BME280_OVERSAMPLING_1X;

    settings.filter = BME280_FILTER_COEFF_OFF;


    /* Apply sensor settings */

    result = bme280_set_sensor_settings(

        BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP |
        BME280_SEL_OSR_HUM |
        BME280_SEL_FILTER,

        &settings,
        &bme
    );

    if (result != BME280_OK)
    {
        Error_Handler();
    }


    /* Set normal operating mode */

    result = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL,
        &bme
    );

    if (result != BME280_OK)
    {
        Error_Handler();
    }

    HAL_Delay(10);
}


/* ESP8266 AND MQTT INITIALIZATION */

static void ESP_MQTT_Init(void)
{
    /* Initialize ESP8266 using USART1 */

    if (ESP8266_Init(StationMode, huart1) != funcOk)
    {
        Error_Handler();
    }


    /* Connect to WiFi */

    if (ESP8266_wifiConnect(
            "SNUC",
            "snu12345"
        ) != funcOk)
    {
        Error_Handler();
    }


    /* Connect to MQTT broker */

    if (ESP8266_portConnect(
            "TCP",
            "broker.emqx.io",
            "1883"
        ) != funcOk)
    {
        Error_Handler();
    }


    /* Configure MQTT client */

    MQTT_InitTypeDef_t mqttSetting = {0};

    mqttSetting.keepAlive = 500;

    mqttSetting.connectFlag = CleanSession;

    mqttSetting.clientID = "STM32_BME280_Publisher";


    /* Connect to MQTT broker */

    MQTT_connectBroker(&mqttSetting);

    HAL_Delay(500);
}


/* BME280 I2C READ FUNCTION */

static int8_t BME280_I2C_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(

        &hi2c1,

        (uint16_t)(address << 1),

        reg_addr,

        I2C_MEMADD_SIZE_8BIT,

        reg_data,

        (uint16_t)len,

        1000
    );

    return (status == HAL_OK)
           ? BME280_OK
           : BME280_E_COMM_FAIL;
}


/* BME280 I2C WRITE FUNCTION */

static int8_t BME280_I2C_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t len,
    void *intf_ptr
)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(

        &hi2c1,

        (uint16_t)(address << 1),

        reg_addr,

        I2C_MEMADD_SIZE_8BIT,

        (uint8_t *)reg_data,

        (uint16_t)len,

        1000
    );

    return (status == HAL_OK)
           ? BME280_OK
           : BME280_E_COMM_FAIL;
}


/* BME280 DELAY FUNCTION */

static void BME280_Delay_us(
    uint32_t period,
    void *intf_ptr
)
{
    (void)intf_ptr;

    HAL_Delay((period + 999U) / 1000U);
}


/* I2C1 INITIALIZATION */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    hi2c1.Init.ClockSpeed = 100000;

    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;

    hi2c1.Init.OwnAddress1 = 0;

    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c1.Init.OwnAddress2 = 0;

    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;


    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* USART1 INITIALIZATION */

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate = 115200;

    huart1.Init.WordLength =
        UART_WORDLENGTH_8B;

    huart1.Init.StopBits =
        UART_STOPBITS_1;

    huart1.Init.Parity =
        UART_PARITY_NONE;

    huart1.Init.Mode =
        UART_MODE_TX_RX;

    huart1.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;

    huart1.Init.OverSampling =
        UART_OVERSAMPLING_16;


    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* GPIO INITIALIZATION */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
}


/* SYSTEM CLOCK CONFIGURATION */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE2
    );


    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState = RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSI;

    RCC_OscInitStruct.PLL.PLLM = 16;

    RCC_OscInitStruct.PLL.PLLN = 336;

    RCC_OscInitStruct.PLL.PLLP =
        RCC_PLLP_DIV4;

    RCC_OscInitStruct.PLL.PLLQ = 7;


    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitTypeDef RCC_ClkInitStruct2 = {0};

    RCC_ClkInitStruct2.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct2.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct2.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct2.APB1CLKDivider =
        RCC_HCLK_DIV2;

    RCC_ClkInitStruct2.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct2,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ERROR HANDLER */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}

#endif
