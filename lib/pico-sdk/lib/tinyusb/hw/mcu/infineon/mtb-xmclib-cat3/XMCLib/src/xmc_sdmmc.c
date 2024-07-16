
/**
 * @file xmc_sdmmc.c
 * @date 2019-05-07
 *
 * @cond
 *****************************************************************************
 * XMClib v2.2.0 - XMC Peripheral Driver Library
 *
 * Copyright (c) 2015-2020, Infineon Technologies AG
 * All rights reserved.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * To improve the quality of the software, users are encouraged to share
 * modifications, enhancements or bug fixes with Infineon Technologies AG
 * at XMCSupport@infineon.com.
 *****************************************************************************
 *
 * Change History
 * --------------
 *
 * 2015-02-20:
 *     - Initial <br>
 *     - Removed GetDriverVersion API <br>
 *
 * 2015-06-20:
 *     - Removed definition of GetDriverVersion API <br>
 *
 * 2016-03-14:
 *     - Values are directly assigned to the int status registers <br>
 *
 * 2016-07-11:
 *     - XMC_SDMMC_SetDataTransferMode() shall not invoke SetDateLineTimeout() <br>
 *
 * 2019-05-07:
 *     - Fixed compilation warnings
 *
 * @endcond
 */


/**
 * @addtogroup SDMMC
 * @brief SDMMC driver
 * @{
 */

/*******************************************************************************
 * HEADER FILES
 *******************************************************************************/

#include "xmc_sdmmc.h"

/*
 * The SDMMC peripheral is only available on the
 * XMC4500. The SDMMC definition can be found in
 * the XMC4500.h (device header file).
 */
#if defined (SDMMC)
#include "xmc_scu.h"

/*******************************************************************************
 * MACROS
 *******************************************************************************/

/*
 * Check for valid SDMMC error events <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_ERROR_EVENT(e)\
  ((e == XMC_SDMMC_CMD_TIMEOUT_ERR)     ||\
   (e == XMC_SDMMC_CMD_CRC_ERR)         ||\
   (e == XMC_SDMMC_CMD_END_BIT_ERR)     ||\
   (e == XMC_SDMMC_CMD_IND_ERR)         ||\
   (e == XMC_SDMMC_DATA_TIMEOUT_ERR)    ||\
   (e == XMC_SDMMC_DATA_CRC_ERR)        ||\
   (e == XMC_SDMMC_DATA_END_BIT_ERR)    ||\
   (e == XMC_SDMMC_CURRENT_LIMIT_ERR)   ||\
   (e == XMC_SDMMC_ACMD_ERR)            ||\
   (e == XMC_SDMMC_TARGET_RESP_ERR))

/*
 * Check for valid SDMMC normal events <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_NORMAL_EVENT(e)\
  ((e == XMC_SDMMC_CMD_COMPLETE)        ||\
   (e == XMC_SDMMC_TX_COMPLETE)         ||\
   (e == XMC_SDMMC_BLOCK_GAP_EVENT)     ||\
   (e == XMC_SDMMC_BUFFER_WRITE_READY)  ||\
   (e == XMC_SDMMC_BUFFER_READ_READY)   ||\
   (e == XMC_SDMMC_CARD_INS)            ||\
   (e == XMC_SDMMC_CARD_REMOVAL)        ||\
   (e == XMC_SDMMC_CARD_INT))

/*
 * Check for both normal and error events <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_EVENT(e)\
  ((XMC_SDMMC_CHECK_NORMAL_EVENT(e))    ||\
   (XMC_SDMMC_CHECK_ERROR_EVENT(e)))

/*
 * Check for valid SDMMC wakeup events <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_WAKEUP_EVENT(w)\
  ((w == XMC_SDMMC_WAKEUP_EN_CARD_INT)  ||\
   (w == XMC_SDMMC_WAKEUP_EN_CARD_INS)  ||\
   (w == XMC_SDMMC_WAKEUP_EN_CARD_REM))

/*
 * Check for valid SDMMC software reset modes <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_SW_RESET_MODE(m)\
  ((m == XMC_SDMMC_SW_RESET_ALL)        ||\
   (m == XMC_SDMMC_SW_RST_CMD_LINE)     ||\
   (m == XMC_SDMMC_SW_RST_DAT_LINE))

/*
 * Check for valid SDMMC transfer modes <br>
 *
 * This macro is used in the LLD for assertion checks (XMC_ASSERT).
 */
#define XMC_SDMMC_CHECK_TRANSFER_MODE(m)\
  ((m == XMC_SDMMC_TRANSFER_MODE_TYPE_SINGLE)    ||\
   (m == XMC_SDMMC_TRANSFER_MODE_TYPE_INFINITE)  ||\
   (m == XMC_SDMMC_TRANSFER_MODE_TYPE_MULTIPLE)  ||\
   (m == XMC_SDMMC_TRANSFER_MODE_TYPE_STOP_MULTIPLE))


/*******************************************************************************
 * API IMPLEMENTATION
 *******************************************************************************/

/* Get power status of the SDMMC peripheral */
bool XMC_SDMMC_GetPowerStatus(XMC_SDMMC_t *const sdmmc)
{
  XMC_ASSERT("XMC_SDMMC_GetPowerStatus: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  return (bool)(sdmmc->POWER_CTRL & SDMMC_POWER_CTRL_SD_BUS_POWER_Msk);
}

/*
 * De-assert the peripheral reset. The SDMMC peripheral
 * needs to be initialized
 */
void XMC_SDMMC_Enable(XMC_SDMMC_t *const sdmmc)
{
  XMC_ASSERT("XMC_SDMMC_Enable: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_UNUSED_ARG(sdmmc);

#if defined(CLOCK_GATING_SUPPORTED)
  XMC_SCU_CLOCK_UngatePeripheralClock(XMC_SCU_PERIPHERAL_CLOCK_SDMMC);
#endif
#if defined(PERIPHERAL_RESET_SUPPORTED)
  XMC_SCU_RESET_DeassertPeripheralReset(XMC_SCU_PERIPHERAL_RESET_SDMMC);
#endif
}

/* Assert the peripheral reset */
void XMC_SDMMC_Disable(XMC_SDMMC_t *const sdmmc)
{
  XMC_ASSERT("XMC_SDMMC_Disable: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_UNUSED_ARG(sdmmc);

#if defined(PERIPHERAL_RESET_SUPPORTED)
  XMC_SCU_RESET_AssertPeripheralReset(XMC_SCU_PERIPHERAL_RESET_SDMMC);
#endif
#if defined(CLOCK_GATING_SUPPORTED)
  XMC_SCU_CLOCK_GatePeripheralClock(XMC_SCU_PERIPHERAL_CLOCK_SDMMC);
#endif
}

/* Initialize SDMMC peripheral */
XMC_SDMMC_STATUS_t XMC_SDMMC_Init(XMC_SDMMC_t *const sdmmc, const XMC_SDMMC_CONFIG_t *config)
{
  XMC_ASSERT("XMC_SDMMC_Init: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_ASSERT("XMC_SDMMC_Init: Invalid clock divider value", XMC_SDMMC_CHECK_SDCLK_FREQ(config->clock_divider));
  XMC_ASSERT("XMC_SDMMC_Init: Invalid bus width", XMC_SDMMC_CHECK_DATA_LINES(config->bus_width));

  /* Enable SDMMC peripheral */
  XMC_SDMMC_Enable(sdmmc);

  /* Write internal clock divider register */
  sdmmc->CLOCK_CTRL |= (uint16_t)((uint32_t)config->clock_divider << SDMMC_CLOCK_CTRL_SDCLK_FREQ_SEL_Pos);

  /* Set bus width */
  sdmmc->HOST_CTRL = (uint8_t)((sdmmc->HOST_CTRL & (uint8_t)~SDMMC_HOST_CTRL_DATA_TX_WIDTH_Msk) |
                               ((uint8_t)config->bus_width << SDMMC_HOST_CTRL_DATA_TX_WIDTH_Pos));

  return XMC_SDMMC_STATUS_SUCCESS;
}

/* Enable event status */
void XMC_SDMMC_EnableEventStatus(XMC_SDMMC_t *const sdmmc, uint32_t event)
{
  XMC_ASSERT("XMC_SDMMC_EnableEventStatus: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  /* Set INT status enable register */
  sdmmc->EN_INT_STATUS_NORM |= (uint16_t)event;
  sdmmc->EN_INT_STATUS_ERR |= (uint16_t)(event >> 16U);
}

/* Disable event status */
void XMC_SDMMC_DisableEventStatus(XMC_SDMMC_t *const sdmmc, uint32_t event)
{
  XMC_ASSERT("XMC_SDMMC_DisableEventStatus: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  /* Clear INT status enable register */
  sdmmc->EN_INT_STATUS_NORM &= (uint16_t)~event;
  sdmmc->EN_INT_STATUS_ERR &= (uint16_t)~(event >> 16U);
}

/* Enable SDMMC event */
void XMC_SDMMC_EnableEvent(XMC_SDMMC_t *const sdmmc, uint32_t event)
{
  XMC_ASSERT("XMC_SDMMC_EnableEvent: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  XMC_SDMMC_EnableEventStatus(sdmmc, event);

  sdmmc->EN_INT_SIGNAL_NORM |= (uint16_t)event;
  sdmmc->EN_INT_SIGNAL_ERR |= (uint16_t)(event >> 16U);
}

/* Disable SDMMC event without disabling event status */
void XMC_SDMMC_DisableEvent(XMC_SDMMC_t *const sdmmc, uint32_t event)
{
  XMC_ASSERT("XMC_SDMMC_DisableEvent: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  /* Clear INT signal enable register */
  sdmmc->EN_INT_SIGNAL_NORM &= (uint16_t)~event;
  sdmmc->EN_INT_SIGNAL_ERR &= (uint16_t)~(event >> 16U);
}

/* Clear SDMMC event(s) */
void XMC_SDMMC_ClearEvent(XMC_SDMMC_t *const sdmmc, uint32_t event)
{
  XMC_ASSERT("XMC_SDMMC_ClearEvent: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_ASSERT("XMC_SDMMC_ClearEvent: Invalid bit-field", !(event & XMC_SDMMC_TARGET_RESP_ERR));

  sdmmc->INT_STATUS_NORM = (uint16_t)event;
  sdmmc->INT_STATUS_ERR = (uint16_t)(event >> 16U);
}

/* Get the status of an SDMMC event */
bool XMC_SDMMC_GetEvent(XMC_SDMMC_t *const sdmmc, XMC_SDMMC_EVENT_t event)
{
  bool status;

  XMC_ASSERT("XMC_SDMMC_GetEvent: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_ASSERT("XMC_SDMMC_GetEvent: Invalid SDMMC event", XMC_SDMMC_CHECK_EVENT(event));

  if (event < XMC_SDMMC_CMD_TIMEOUT_ERR)
  {
    status = (bool)(sdmmc->INT_STATUS_NORM & (uint16_t)event);
  }
  else
  {
    status = (bool)(sdmmc->INT_STATUS_ERR & (uint16_t)((uint32_t)event >> 16U));
  }

  return status;
}

/* Read R2 response (CID, CSD register) */
void XMC_SDMMC_GetR2Response(XMC_SDMMC_t *const sdmmc, XMC_SDMMC_RESPONSE_t *const response)
{
  XMC_ASSERT("XMC_SDMMC_GetR2Response: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  response->response_0 = sdmmc->RESPONSE[0];
  response->response_2 = sdmmc->RESPONSE[1];
  response->response_4 = sdmmc->RESPONSE[2];
  response->response_6 = sdmmc->RESPONSE[3];
}

/* Send SDMMC command */
XMC_SDMMC_STATUS_t XMC_SDMMC_SendCommand(XMC_SDMMC_t *const sdmmc, const XMC_SDMMC_COMMAND_t *cmd, uint32_t arg)
{
  XMC_ASSERT("XMC_SDMMC_SendCommand: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));

  sdmmc->ARGUMENT1 = arg;
  sdmmc->COMMAND = (uint16_t)(*(uint16_t *)cmd);

  return XMC_SDMMC_STATUS_SUCCESS;
}

/* Set data transfer mode */
void XMC_SDMMC_SetDataTransferMode(XMC_SDMMC_t *const sdmmc, XMC_SDMMC_TRANSFER_MODE_t *const response)
{
  XMC_ASSERT("XMC_SDMMC_SetDataTransferMode: Invalid module pointer", XMC_SDMMC_CHECK_MODULE_PTR(sdmmc));
  XMC_ASSERT("XMC_SDMMC_SetDataTransferMode: Invalid transfer type", XMC_SDMMC_CHECK_TRANSFER_MODE(response->type));

  /* Block size */
  sdmmc->BLOCK_SIZE = (uint16_t)(response->block_size);

  /* Number of blocks */
  sdmmc->BLOCK_COUNT = (uint16_t)(response->num_blocks);

  /* Type of data transfer: single, infinite, multiple or stop multiple */
  sdmmc->TRANSFER_MODE = (uint16_t)((sdmmc->TRANSFER_MODE & (uint16_t)~SDMMC_TRANSFER_MODE_MULTI_BLOCK_SELECT_Msk) |
                                    ((uint16_t)response->type));

  /*
   * Clear block count enable bit; that's only valid for
   * a multi-block transfer
   */
  if (response->type == XMC_SDMMC_TRANSFER_MODE_TYPE_SINGLE)
  {
    sdmmc->TRANSFER_MODE &= (uint16_t)~SDMMC_TRANSFER_MODE_BLOCK_COUNT_EN_Msk;
  }

  /* Auto CMD configuration */
  sdmmc->TRANSFER_MODE = (uint16_t)((sdmmc->TRANSFER_MODE & (uint16_t)~SDMMC_TRANSFER_MODE_ACMD_EN_Msk) |
                                    ((uint16_t)response->auto_cmd << SDMMC_TRANSFER_MODE_ACMD_EN_Pos));
}

#endif /* #if defined (SDMMC) */

/**
 * @}
 */

