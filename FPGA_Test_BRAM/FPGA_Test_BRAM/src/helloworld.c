#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "xil_io.h"

/* -------------------- CONFIG -------------------- */
#define BRAM_BASE_ADDR   XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR
#define BRAM_MAX_COUNT   256          // number of 32-bit words
#define SAMPLE_DELAY_US  10000        // 10 ms
#define RECORD_TIME_SEC  3

#define LED_CHANNEL      1
#define SWITCH_CHANNEL   2
#define TRIGGER_MASK     0x01         // SW0

/* -------------------- GLOBALS -------------------- */
XGpio gpio;

/* -------------------- FUNCTIONS -------------------- */
void driverInit(void)
{
    int Status;
    Status = XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        xil_printf("GPIO init failed\r\n");
        while (1);
    }
}

void configGpio(void)
{
    XGpio_SetDataDirection(&gpio, LED_CHANNEL, 0x0);  // LEDs output
    XGpio_SetDataDirection(&gpio, SWITCH_CHANNEL, 0xF); // switches input
}

/* -------------------- MAIN -------------------- */
int main()
{
    u32 sw_data;
    u32 bram_index = 0;
    u32 i;

    init_platform();
    driverInit();
    configGpio();

    xil_printf("\r\n=== BRAM Triggered Recorder ===\r\n");
    xil_printf("Press SW0 to record for 3 seconds\r\n");

    while (1)
    {
        sw_data = XGpio_DiscreteRead(&gpio, SWITCH_CHANNEL);

        /* ---------- IDLE: wait for trigger ---------- */
        if (sw_data & TRIGGER_MASK)
        {
            xil_printf("\r\nRecording started...\r\n");
            bram_index = 0;

            /* ---------- RECORD PHASE (3 seconds) ---------- */
            for (i = 0;
                 i < (RECORD_TIME_SEC * 1000000 / SAMPLE_DELAY_US);
                 i++)
            {
                if (bram_index < BRAM_MAX_COUNT)
                {
                    sw_data = XGpio_DiscreteRead(&gpio, SWITCH_CHANNEL);
                    Xil_Out32(BRAM_BASE_ADDR + bram_index * 4, sw_data);
                    bram_index++;
                }
                usleep(SAMPLE_DELAY_US);
            }

            xil_printf("Recording finished. Samples = %lu\r\n",
                       (unsigned long)bram_index);

            /* ---------- READ BACK ---------- */
            xil_printf("Dumping BRAM contents:\r\n");
            for (i = 0; i < bram_index; i++)
            {
                u32 val = Xil_In32(BRAM_BASE_ADDR + i * 4);
                xil_printf("BRAM[%03lu] = 0x%08lx\r\n",
                           (unsigned long)i,
                           (unsigned long)val);
            }

            xil_printf("Done. Release SW0 to re-arm.\r\n");

            /* ---------- WAIT FOR BUTTON RELEASE ---------- */
            while (XGpio_DiscreteRead(&gpio, SWITCH_CHANNEL) & TRIGGER_MASK);
        }

        usleep(100000);   // IDLE polling delay (100 ms)
    }

    cleanup_platform();
    return 0;
}