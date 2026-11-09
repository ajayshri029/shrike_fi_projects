// Direct Register Address Definitions for GPIO 00
#define REG_GPIO_OUT        (*(volatile unsigned int *)0x60004004)
#define REG_GPIO_ENABLE     (*(volatile unsigned int *)0x60004020)
#define IO_MUX_GPIO0_REG    (*(volatile unsigned int *)0x60009004)

#define GPIO0_MASK          (1U << 0)
#define MCU_FUN_PHYS_OE     (1U << 0)

// --- Watchdog Timer Register Controls (Bypasses OS timeouts) ---
#define WDT_WKEY_VALUE      0x50D83AA1 // Magic hardware key to unlock watchdog writes
#define REG_TIMG0_WDTWPROTECT (*(volatile unsigned int *)0x60020064)
#define REG_TIMG0_WDTFEED     (*(volatile unsigned int *)0x60020060)

/**
 * Feeds the hardware watchdog timer directly at the register layer.
 * This keeps the ESP32-S3 from resetting or throwing stack trace errors
 * during long bare-metal busy loops.
 */
static void feed_hardware_watchdog(void) {
    REG_TIMG0_WDTWPROTECT = WDT_WKEY_VALUE; // Unlock protection register
    REG_TIMG0_WDTFEED = 1;                  // Write any value to feed the watchdog
    REG_TIMG0_WDTWPROTECT = 0;              // Relock protection register
}

static void raw_delay_cycles(void) {
    volatile unsigned int count = 20000000;
    while (count--) {
        // Feed periodically inside the long delay loop
        if ((count % 100000) == 0) {
            feed_hardware_watchdog();
        }
    }
}

void app_main(void) {
    IO_MUX_GPIO0_REG |= MCU_FUN_PHYS_OE; 
    REG_GPIO_ENABLE  |= GPIO0_MASK;      

    while (1) {
        REG_GPIO_OUT |= GPIO0_MASK;      // LED On
        raw_delay_cycles();
        
        REG_GPIO_OUT &= ~GPIO0_MASK;     // LED Off
        raw_delay_cycles();
        
        feed_hardware_watchdog();         // Final safety feed per toggle
    }
}
