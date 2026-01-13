#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"
#include "xil_io.h"

#define DDR_BASE_ADDR  0x00120000
#define DDR_MAX_COUNT  1024
#define SAMPLE_DELAY_US 10000        // 10 ms
#define RECORD_TIME_SEC 3

XGpio gpio;

void driverInit(void)
{
    XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
}

void configGpio(void)
{
    XGpio_SetDataDirection(&gpio, 1, 0x0);   // LED output
    XGpio_SetDataDirection(&gpio, 2, 0xF);   // Button input
}

int main()
{
    int pb_data;
    u32 ddr_index = 0;
    u32 i;

    init_platform();
    driverInit();
    configGpio();

    xil_printf("Press Button[0] to start 3-second recording\r\n");

    while (1)
    {
        pb_data = XGpio_DiscreteRead(&gpio, 2);

        /* -------- IDLE: wait for trigger -------- */
        if (pb_data & 0x01)
        {
            xil_printf("Recording started...\r\n");

            ddr_index = 0;

            /* -------- RECORDING (3 seconds) -------- */
            for (i = 0; i < (RECORD_TIME_SEC * 1000000 / SAMPLE_DELAY_US); i++)
            {
                if (ddr_index < DDR_MAX_COUNT)
                {
                    pb_data = XGpio_DiscreteRead(&gpio, 2);
                    Xil_Out32(DDR_BASE_ADDR + ddr_index * 4, pb_data);
                    ddr_index++;
                }
                usleep(SAMPLE_DELAY_US);
            }

            xil_printf("Recording finished. Samples = %d\r\n", ddr_index);

            /* -------- OUTPUT -------- */
            xil_printf("Dumping DDR contents:\r\n");
            for (i = 0; i < ddr_index; i++)
            {
                u32 val = Xil_In32(DDR_BASE_ADDR + i * 4);
                xil_printf("DDR[%d] = 0x%x\r\n", i, val);
            }

            xil_printf("Done. Back to IDLE.\r\n");

            /* */
            while (XGpio_DiscreteRead(&gpio, 2) & 0x01);
        }

        usleep(100000);  // IDLE polling
    }

    cleanup_platform();
    return 0;
}
