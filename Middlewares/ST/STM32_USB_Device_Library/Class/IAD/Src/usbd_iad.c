/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_iad.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_IAD_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_IAD_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_IAD_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_IAD_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_IAD_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_IAD_EP0_RxReady(USBD_HandleTypeDef *pdev);

static uint8_t *USBD_IAD_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_IAD_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_IAD_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_IAD_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_IAD_GetDeviceQualifierDescriptor(uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Variables
  * @{
  */


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_IAD =
{
  USBD_IAD_Init,
  USBD_IAD_DeInit,
  USBD_IAD_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_IAD_EP0_RxReady,
  USBD_IAD_DataIn,
  USBD_IAD_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_IAD_GetHSCfgDesc,
  USBD_IAD_GetFSCfgDesc,
  USBD_IAD_GetOtherSpeedCfgDesc,
  USBD_IAD_GetDeviceQualifierDescriptor,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_IAD_CfgHSDesc[USB_IAD_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_IAD_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x01,                                       /* bNumInterfaces: 1 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
                                              /* == 0, is no configuration descrption */
  0xC0,                                       /* bmAttributes: bus powered */
                                              /* D7 Reserved, set to 1. (USB 1.0 Bus Powered) D6 Self Powered D5 Remote Wakeup 
                                                 D4..0 Reserved, set to 0. */
  0xFA,                                       /* MaxPower 0xFA=250 x 2 = 500 mA */
  /* When the configuration descriptor is read, it returns the entire configuration hierarchy 
  which includes all related interface and endpoint descriptors. The wTotalLength field reflects 
  the number of bytes in the hierarchy.
  */

  /* Interface Descriptors
  The interface descriptor could be seen as a header or grouping of the endpoints into a functional group performing a single 
  feature of the device.
  */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface  */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
                                              /* More like a index definition */
  /* This item distinguishes between interfaces of a configuration. Composite devices, 
  such as a keyboard with an embedded mouse, have more than one active interface. Each interface 
  must be distinguished using this designation item. The firmware template only defines a single 
  interface, so the Interface number can be set to 0x00. */

  0x00,                                       /* bAlternateSetting: Alternate setting */
  /* This item is useful for devices that define multiple interfaces. This firmware template assumes 
  that only one primary interface will be defined for the device, and sets this item to 0x00, 
  which tells the host that the device defines no alternate setting */

  0x01,                                       /* bNumEndpoints: One endpoints used */
  /* This item tells the host how many endpoints, not counting the control endpoint */
  /*  HID specification requires at least one IN interrupt endpoint be defined in every device. */

  0x03,                                       /* bInterfaceClass: HID */
  0x00,                                       /* bInterfaceSubClass   1=BOOT, 0=no boot */
  /* This item further describes the device by defining which subclass of the above-defined class 
  the device falls under. For many HID devices, this item will be set to set to 0x00. 
  See the HID Specification for a list of defined HID subclasses. */

  0x02,                                       /* bInterfaceProtocol: 0=none, 1=keyboard, 2=mouse */
  /* This item can be used to define protocol settings for a USB device. For many HID devices, 
  this item will be set to set to 0x00. See the HID Specification for a list of defined HID protocols. */

  0x00,                                       /* iInterface:  Index of string descriptor */
  /* This item tells the host the string index of the interface string that describes the specifics 
  of the interface and can be displayed on-screen. The firmware template defines no such string 
  and so sets the index to 0x00 */

  /* descriptor of Mouse HID */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,                     /*bNumDescriptors: Number of HID class descriptors to follow*/
  HID_REPORT_DESC,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,

  /******************** Descriptor of Mouse endpoint ********************/
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
  0x00,
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval */

};


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_IAD_CfgFSDesc[USB_IAD_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                       /* bmAttributes: self powered */
  0x32,                                       /* MaxPower 0 mA */
};

__ALIGN_BEGIN static uint8_t USBD_IAD_OtherSpeedCfgDesc[USB_IAD_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,                                       /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
  USB_CDC_CONFIG_DESC_SIZ,
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interfaces */
  0x01,                                       /* bConfigurationValue: */
  0x04,                                       /* iConfiguration: */
  0xC0,                                       /* bmAttributes: */
  0x32,                                       /* MaxPower 100 mA */

};

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_IAD_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_CDC_HandleTypeDef *hcdc;

  hcdc = USBD_malloc(sizeof(USBD_CDC_HandleTypeDef));

  if (hcdc == NULL)
  {
    pdev->pClassData = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassData = (void *)hcdc;

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Open EP IN */
    (void)USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK,
                         CDC_DATA_HS_IN_PACKET_SIZE);

     pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

     /* Open EP OUT */
     (void)USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK,
                          CDC_DATA_HS_OUT_PACKET_SIZE);

      pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

      /* Set bInterval for CDC CMD Endpoint */
      pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = CDC_HS_BINTERVAL;
  }
  else
  {
    /* Open EP IN */
    (void)USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK,
                         CDC_DATA_FS_IN_PACKET_SIZE);

     pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

     /* Open EP OUT */
     (void)USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK,
                          CDC_DATA_FS_OUT_PACKET_SIZE);

      pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

      /* Set bInterval for CMD Endpoint */
      pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;
  }

  /* Open Command IN EP */
  (void)USBD_LL_OpenEP(pdev, CDC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 1U;

  /* Init  physical Interface components */
  ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Init();

  /* Init Xfer states */
  hcdc->TxState = 0U;
  hcdc->RxState = 0U;

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_IAD_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  uint8_t ret = 0U;

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, CDC_IN_EP);
  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, CDC_OUT_EP);
  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;

  /* Close Command IN EP */
  (void)USBD_LL_CloseEP(pdev, CDC_CMD_EP);
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 0U;
  pdev->ep_in[CDC_CMD_EP & 0xFU].bInterval = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL)
  {
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->DeInit();
    (void)USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return ret;
}

/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_IAD_Setup(USBD_HandleTypeDef *pdev,
                              USBD_SetupReqTypedef *req)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS:
    if (req->wLength != 0U)
    {
      if ((req->bmRequest & 0x80U) != 0U)
      {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)hcdc->data,
                                                          req->wLength);

          (void)USBD_CtlSendData(pdev, (uint8_t *)hcdc->data, req->wLength);
      }
      else
      {
        hcdc->CmdOpCode = req->bRequest;
        hcdc->CmdLength = (uint8_t)req->wLength;

        (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hcdc->data, req->wLength);
      }
    }
    else
    {
      ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                        (uint8_t *)req, 0U);
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
      {
        (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
      }
      else
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_GET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED)
      {
        (void)USBD_CtlSendData(pdev, &ifalt, 1U);
      }
      else
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_SET_INTERFACE:
      if (pdev->dev_state != USBD_STATE_CONFIGURED)
      {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
      }
      break;

    case USB_REQ_CLEAR_FEATURE:
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    }
    break;

  default:
    USBD_CtlError(pdev, req);
    ret = USBD_FAIL;
    break;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_IAD_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc;
  PCD_HandleTypeDef *hpcd = pdev->pData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if ((pdev->ep_in[epnum].total_length > 0U) &&
      ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
  {
    /* Update the packet total length */
    pdev->ep_in[epnum].total_length = 0U;

    /* Send ZLP */
    (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
  }
  else
  {
    hcdc->TxState = 0U;
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->TransmitCplt(hcdc->TxBuffer, &hcdc->TxLength, epnum);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_IAD_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get the received data length */
  hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application Xfer */

  ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Receive(hcdc->RxBuffer, &hcdc->RxLength);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDC_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_IAD_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if ((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFFU))
  {
    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(hcdc->CmdOpCode,
                                                      (uint8_t *)hcdc->data,
                                                      (uint16_t)hcdc->CmdLength);
    hcdc->CmdOpCode = 0xFFU;

  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_IAD_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_IAD_GetHSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_IAD_CfgHSDesc);

  return USBD_IAD_CfgHSDesc;
}

/**
  * @brief  USBD_IAD_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_IAD_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_IAD_CfgFSDesc);

  return USBD_IAD_CfgFSDesc;
}

/**
  * @brief  USBD_IAD_GetCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_IAD_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_IAD_OtherSpeedCfgDesc);

  return USBD_IAD_OtherSpeedCfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_IAD_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_CDC_DeviceQualifierDesc);

  return USBD_CDC_DeviceQualifierDesc;
}

/**
* @brief  USBD_CDC_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_IAD_RegisterInterface(USBD_HandleTypeDef *pdev,
                                   USBD_CDC_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t USBD_IAD_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_CDC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t USBD_IAD_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  hcdc->RxBuffer = pbuff;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CDC_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_IAD_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  USBD_StatusTypeDef ret = USBD_BUSY;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hcdc->TxState == 0U)
  {
    /* Tx Transfer in progress */
    hcdc->TxState = 1U;

    /* Update the packet total length */
    pdev->ep_in[CDC_IN_EP & 0xFU].total_length = hcdc->TxLength;

    /* Transmit next packet */
    (void)USBD_LL_Transmit(pdev, CDC_IN_EP, hcdc->TxBuffer, hcdc->TxLength);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}


/**
  * @brief  USBD_CDC_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_IAD_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer,
                                 CDC_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
