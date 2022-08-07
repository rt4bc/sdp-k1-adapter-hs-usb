/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : custom.h
  * @brief          : Header for custom.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CUSTOM_H
#define __CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

void USB_Phy_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __CUSTOM_H */

