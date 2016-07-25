/*!
    \file  usbd_pwr.c
    \brief USB device power management driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_pwr.h"

#ifdef USBD_LOWPWR_MODE_ENABLE
static void  lowpower_mode_exit (void);
#endif /* USBD_LOWPWR_MODE_ENABLE */

__IO uint8_t g_ESOF_count = 0;
__IO uint8_t g_suspend_enabled = 1;
__IO uint8_t g_remote_wakeup_on = 0;

/*!
    \brief      USB wakeup first operation is to wakeup mcu
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  resume_mcu (void)
{
    /* clear low_power mode bit in USBD_CTL */
    USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) & (~CTL_LOWM));

#ifdef USBD_LOWPWR_MODE_ENABLE

    /* restore normal operations */
    lowpower_mode_exit();

#endif /* USBD_LOWPWR_MODE_ENABLE */

    /* clear SETSPS bit */
    USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) & (~CTL_SETSPS));
}

#ifdef USBD_LOWPWR_MODE_ENABLE

/*!
    \brief      restore system clocks and power while exiting suspend mode
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void  lowpower_mode_exit (void)
{
    /* restore system clock */

    /* enable HSE */
    rcu_osci_on(RCU_HXTAL);

    /* wait till HSE is ready */
    while(RESET == rcu_flag_get(RCU_FLAG_HXTALSTB));

    /* enable PLL */
    rcu_osci_on(RCU_PLL);

    /* wait till PLL is ready */
    while(RESET == rcu_flag_get(RCU_FLAG_PLLSTB));

    /* select PLL as system clock source */
    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);

    /* wait till PLL is used as system clock source */
    while(0x08 != rcu_system_clock_source_get());

    /* low power sleep on exit disabled */
    system_lowpower_reset(SCB_LPM_DEEPSLEEP);
}

#endif /* USBD_LOWPWR_MODE_ENABLE */

/*!
    \brief      set USB device to suspend mode
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usbd_suspend (void)
{
    /* set usb module to suspend and low-power mode */
    USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) | CTL_SETSPS | CTL_LOWM);

#ifdef USBD_LOWPWR_MODE_ENABLE

    /* check wakeup flag is set */
    if (0 == (USBD_REG_GET(USBD_INTF) & INTF_WKUPIF)) {
        /* enter DEEP_SLEEP mode with LDO in low power mode */
        pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);
    } else {
        /* clear wakeup interrupt flag */
        USBD_REG_SET(USBD_INTF, CLR_WKUPIF);

        /* clear set_suspend flag */
        USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) & ~CTL_SETSPS);
    }

#endif /* USBD_LOWPWR_MODE_ENABLE */
}

/*!
    \brief      start to remote wakeup
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usbd_remote_wakeup_active(void)
{
    resume_mcu();
    g_remote_wakeup_on = 1;

    g_ESOF_count = 15;
    USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) | CTL_RSREQ);
}
