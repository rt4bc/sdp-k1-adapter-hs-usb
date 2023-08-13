/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : custom.c
 * @brief          : custom init and function body
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "custom.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"

/**
 * @brief  Reset Exteranl USB Phy by GPIO.
 * @param  None
 * @retval None
 */
void USB_Phy_Reset(void) {

  Debug_Print("Pull Down usb phy reset pin\n");
  HAL_GPIO_WritePin(USB_PHY_RST_GPIO_Port, USB_PHY_RST_Pin, GPIO_PIN_RESET);
  for (volatile unsigned int i = 0; i < 360000; i++)
    ;
  Debug_Print("Pull Up usb phy reset pin\n");
  HAL_GPIO_WritePin(USB_PHY_RST_GPIO_Port, USB_PHY_RST_Pin, GPIO_PIN_SET);
  for (volatile unsigned int i = 0; i < 360000; i++)
    ;
}