#include "stdint.h"
#include "inc/tm4c123gh6pm.h"

void UART_Init(void);
void PLL_SetFrequency(uint32_t frequency);
void UART_WriteChar(char c);
char UART_ReadChar(void);
uint32_t ConvertToFrequency(char *buffer);

int main(void)
{
    char buffer[10];
    int index = 0;
    char receivedChar;

    UART_Init();

    while (1)
    {
        receivedChar = UART_ReadChar();

        if (receivedChar == '\n')
        {
            buffer[index] = '\0';
            uint32_t frequency = ConvertToFrequency(buffer);
            if (frequency >= 10 && frequency <= 80)
            {
                PLL_SetFrequency(frequency * 1000000);
            }
            index = 0; // Reset buffer
        }
        else
        {
            buffer[index++] = receivedChar;
        }
    }
}

void UART_Init(void)
{
    SYSCTL_RCGCUART_R |= 0x01; // Enable UART0
    SYSCTL_RCGCGPIO_R |= 0x01; // Enable Port A

    UART0_CTL_R &= ~0x01;      // Disable UART
    UART0_IBRD_R = 104;        // 16 MHz / (16 * 9600) = 104.1666
    UART0_FBRD_R = 11;         // Fractional part
    UART0_LCRH_R = 0x70;       // 8-bit, no parity, 1-stop bit
    UART0_CTL_R = 0x301;       // Enable UART

    GPIO_PORTA_AFSEL_R |= 0x03; // Enable PA0, PA1 for UART
    GPIO_PORTA_DEN_R |= 0x03;   // Digital enable
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011;
}

void PLL_SetFrequency(uint32_t frequency)
{
    SYSCTL_RCC2_R |= 0x80000000; // Use RCC2
    SYSCTL_RCC2_R |= 0x00000800; // BYPASS2, bypass PLL

    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x7C0) + 0x540; // Clear XTAL bits and set for 16 MHz

    uint32_t sysDiv = 400000000 / frequency - 1;
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) | (sysDiv << 22);

    SYSCTL_RCC2_R &= ~0x00000800; // Enable PLL
    while (!(SYSCTL_RIS_R & 0x40)); // Wait for PLL to lock
    SYSCTL_RCC2_R &= ~0x800;       // Use PLL
}

void UART_WriteChar(char c)
{
    while ((UART0_FR_R & 0x20) != 0); // Wait until TXFF is 0
    UART0_DR_R = c;
}

char UART_ReadChar(void)
{
    while ((UART0_FR_R & 0x10) != 0); // Wait until RXFE is 0
    return (char)(UART0_DR_R & 0xFF);
}

uint32_t ConvertToFrequency(char *buffer)
{
    uint32_t value = 0;
    while (*buffer)
    {
        value = value * 10 + (*buffer - '0');
        buffer++;
    }
    return value;
}
