# USB Interrupt 
## Out EP Interrupt
OTG device OUT endpoint x interrupt register (OTG_DOEPINTx)
(x = 0..5[FS] /8[HS], where x = Endpoint number)

USBD_StdDevReq

USBD_LL_SetupStage


HAL_PCD_SetupStageCallback


PCD_EP_OutSetupPacket_int


HAL_PCD_IRQHandler

 if ((epint & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP)
          {
            CLEAR_OUT_EP_INTR(epnum, USB_OTG_DOEPINT_STUP);
            /* Class B setup phase done for previous decoded setup */
            (void)PCD_EP_OutSetupPacket_int(hpcd, epnum);
          }


USB_OTG_GINTSTS_OEPINT interrupt

## IN EP Interrupt
   if (__HAL_PCD_GET_FLAG(hpcd, USB_OTG_GINTSTS_IEPINT))

