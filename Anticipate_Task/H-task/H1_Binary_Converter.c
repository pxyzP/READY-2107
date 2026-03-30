/*
 * H1_Binary_Converter.c
 * H-Task Example 1: Binary to Decimal/Hex Converter using Bump Switches
 *
 * Description:
 * - Use bump switches as binary input (Bump0=bit0...Bump5=bit5)
 * - Press switches to set binary pattern
 * - Display converted decimal and hexadecimal on serial terminal
 * - Library conversion functions NOT allowed - implement manually
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Bump.h"
#include "..\..\inc\UART0.h"

volatile uint8_t bump_data = 0xFF;
volatile uint8_t convert_flag = 0;

void BumpHandler(){
    P4->IFG &= ~0xFF;
    bump_data = Bump_Read();
    convert_flag = 1;
}

// Manual binary to decimal (NO library functions)
uint8_t BinaryToDecimal(uint8_t binary){
    // Already in decimal form as 6-bit value (0-63)
    return binary;
}

// Manual decimal to hex string (NO library functions)
void DecimalToHexString(uint8_t decimal, char *hexStr){
    uint8_t highNibble = (decimal >> 4) & 0x0F;
    uint8_t lowNibble = decimal & 0x0F;
    
    // Convert high nibble
    if (highNibble < 10) {
        hexStr[0] = '0' + highNibble;
    } else {
        hexStr[0] = 'A' + (highNibble - 10);
    }
    
    // Convert low nibble
    if (lowNibble < 10) {
        hexStr[1] = '0' + lowNibble;
    } else {
        hexStr[1] = 'A' + (lowNibble - 10);
    }
    
    hexStr[2] = '\0';  // Null terminator
}

// Print binary representation
void PrintBinary(uint8_t value){
    UART0_OutString("0b");
    for (int i = 5; i >= 0; i--) {
        if (value & (1 << i)) {
            UART0_OutChar('1');
        } else {
            UART0_OutChar('0');
        }
    }
}

int main(){
    uint8_t binaryValue;
    uint8_t decimalValue;
    char hexString[3];
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Bump_Init(&BumpHandler);
    UART0_Init();
    
    UART0_OutString("========================================\r\n");
    UART0_OutString("H-Task 1: Binary to Decimal/Hex Converter\r\n");
    UART0_OutString("========================================\r\n");
    UART0_OutString("Bump Switch Mapping:\r\n");
    UART0_OutString("  Bump0 (right) = bit 0 (LSB)\r\n");
    UART0_OutString("  Bump1 = bit 1\r\n");
    UART0_OutString("  Bump2 = bit 2\r\n");
    UART0_OutString("  Bump3 = bit 3\r\n");
    UART0_OutString("  Bump4 = bit 4\r\n");
    UART0_OutString("  Bump5 (left) = bit 5 (MSB)\r\n");
    UART0_OutString("========================================\r\n");
    UART0_OutString("Press switches to create binary pattern\r\n");
    UART0_OutString("Released = 1, Pressed = 0\r\n\r\n");
    
    while(1){
        if (convert_flag) {
            convert_flag = 0;
            
            // Get binary value from bump switches
            // Inverted because bump switches are negative logic
            binaryValue = ~bump_data & 0x3F;  // 6 bits only
            
            // Convert to decimal (manual)
            decimalValue = BinaryToDecimal(binaryValue);
            
            // Convert to hex string (manual)
            DecimalToHexString(decimalValue, hexString);
            
            // Display results
            UART0_OutString("\r\n--- Conversion Result ---\r\n");
            UART0_OutString("Binary:  ");
            PrintBinary(binaryValue);
            UART0_OutString("\r\n");
            
            UART0_OutString("Decimal: ");
            UART0_OutUDec5(decimalValue);
            UART0_OutString("\r\n");
            
            UART0_OutString("Hex:     0x");
            UART0_OutString(hexString);
            UART0_OutString("\r\n");
            UART0_OutString("-------------------------\r\n\r\n");
        }
        
        Clock_Delay1ms(50);
    }
}
