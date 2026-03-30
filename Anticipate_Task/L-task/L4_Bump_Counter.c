/*
 * L4_Bump_Counter.c
 * L-Task Example 4: Toggle LED on bump switch press and count
 *
 * Description:
 * - Toggle RED LED each time any bump switch is pressed
 * - Display total press count on serial terminal
 * - Debounced input handling
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

volatile uint32_t press_count = 0;
volatile uint8_t bump_flag = 0;
volatile uint8_t led_state = 0;

void BumpHandler(){
    P4->IFG &= ~0xFF;
    uint8_t bump_data = Bump_Read();
    
    // Check if any switch pressed
    if (bump_data != 0x3F) {  // Not all switches released
        bump_flag = 1;
        press_count++;
        led_state ^= 1;  // Toggle LED state
    }
}

int main(){
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Bump_Init(&BumpHandler);
    UART0_Init();
    
    UART0_OutString("L-Task 4: Bump Counter with LED Toggle\r\n");
    UART0_OutString("Press any bump switch to toggle LED\r\n\r\n");
    
    while(1){
        if (bump_flag) {
            bump_flag = 0;
            
            // Toggle LED
            if (led_state) {
                LaunchPad_LED(1);
            } else {
                LaunchPad_LED(0);
            }
            
            // Display count
            UART0_OutString("Bump pressed! Total count: ");
            UART0_OutUDec5(press_count);
            UART0_OutString(" | LED: ");
            if (led_state) {
                UART0_OutString("ON\r\n");
            } else {
                UART0_OutString("OFF\r\n");
            }
        }
        
        Clock_Delay1ms(50);
    }
}
