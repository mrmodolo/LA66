#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_uart.h"

#define LED_PORT     GPIOB
#define LED_RED_PIN  GPIO_PIN_13
#define LED_GRN_PIN  GPIO_PIN_14

void SysTick_Handler(void) {}

static void busy_wait(volatile uint32_t n)
{
    while (n--) {}
}

static void uart_puts(const char *s)
{
    /* bail out if TX FIFO stays full for too long */
    while (*s) {
        volatile uint32_t timeout = 100000;
        while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_FULL) == SET) {
            if (--timeout == 0) return;
        }
        uart_send_data(UART0, (uint8_t)*s++);
    }
}

static void uart_init_debug(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1); // UART0_TX: GP17

    uart_config_t cfg;
    uart_config_init(&cfg);
    cfg.baudrate  = UART_BAUDRATE_115200;
    cfg.mode      = UART_MODE_TX;
    cfg.fifo_mode = ENABLE;
    uart_init(UART0, &cfg);
    uart_cmd(UART0, ENABLE);
}

int main(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);

    gpio_init(LED_PORT, LED_RED_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(LED_PORT, LED_GRN_PIN, GPIO_MODE_OUTPUT_PP_LOW);

    uart_init_debug();
    uart_puts("TEST-BLINK: boot\r\n");

    uint32_t tick = 0;
    while (1) {
        gpio_write(LED_PORT, LED_RED_PIN, 1);
        gpio_write(LED_PORT, LED_GRN_PIN, 0);
        uart_puts("TEST-BLINK: tick RED\r\n");
        busy_wait(500000);

        gpio_write(LED_PORT, LED_RED_PIN, 0);
        gpio_write(LED_PORT, LED_GRN_PIN, 1);
        uart_puts("TEST-BLINK: tick GRN\r\n");
        busy_wait(500000);

        tick++;
    }

    return 0;
}
