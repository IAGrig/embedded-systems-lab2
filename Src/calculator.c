#include "calculator.h"
#include "oled.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OLED_LINE_0_Y 0
#define OLED_LINE_1_Y 16
#define OLED_LINE_2_Y 32
#define OLED_LINE_3_Y 48

#define KEYB_ASTERISK_CODE 10
#define KEYB_ZERO_CODE 11
#define KEYB_HASH_CODE 12

#define INPUT_BUFFER_SIZE 9
#define OUTPUT_BUFFER_SIZE 18


static CalculatorState state = CALC_STATE_INPUT_FIRST_OPERAND;
static int32_t first_number = 0;
static int32_t second_number = 0;
static int32_t result = 0;
static CalculatorOperation operation = CALC_OPERATION_ADDITION;
static char input_buffer[INPUT_BUFFER_SIZE+1] = "";
static uint8_t input_index = 0;

CalculatorState calculator_get_state(void)
{
    return state;
}

void calculator_update_display(void);
void calculator_init(void)
{
	calculator_update_display();
    calculator_reset();
}

void calculator_clear_input_buffer(void)
{
	memset(input_buffer, '\0', sizeof(input_buffer));
}

static void calculator_update_leds(void)
{
    if (state == CALC_STATE_ERROR) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // RED ON
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // GREEN OFF
    } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET); // GREEN ON
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // RED OFF
    }
}


void calculator_write_info_message(void)
{
	switch(state) {
	        case CALC_STATE_INPUT_FIRST_OPERAND:
	        	oled_WriteString("Enter 1st number", Font_7x10, White);
	            break;
	        case CALC_STATE_INPUT_OPERATION:
	        	oled_WriteString("Select operation", Font_7x10, White);
	            break;
	        case CALC_STATE_INPUT_SECOND_OPERAND:
	        	oled_WriteString("Enter 2nd number", Font_7x10, White);
	            break;
	        case CALC_STATE_SHOW_RESULT:
	        case CALC_STATE_ERROR:
	        	oled_WriteString("Press SB1 to clear", Font_7x10, White);
	            break;
	        default:
	        	oled_WriteString("unknown state", Font_7x10, White);
	            break;
	    }
}


void calculator_update_display(void)
{
    if (state == CALC_STATE_ERROR) {
    	oled_Fill(Black);
        oled_SetCursor(0, OLED_LINE_1_Y);
        oled_WriteString("OVERFLOW ERROR", Font_7x10, White);
        oled_SetCursor(0, OLED_LINE_3_Y);
        calculator_write_info_message();
        oled_UpdateScreen();
        return;
    }

    char line1[OUTPUT_BUFFER_SIZE], line2[OUTPUT_BUFFER_SIZE], line3[OUTPUT_BUFFER_SIZE];

    // first operand and operation
    oled_SetCursor(0, OLED_LINE_0_Y);
    if (state == CALC_STATE_INPUT_OPERATION || state == CALC_STATE_INPUT_SECOND_OPERAND || state == CALC_STATE_SHOW_RESULT) {
        snprintf(line1, sizeof(line1), "%ld%c", first_number, CalculatorOperationSymbols[operation]);
    } else {
        if (input_index == 0) {
            snprintf(line1, sizeof(line1), "_");
        } else {
            snprintf(line1, sizeof(line1), "%s_", input_buffer);
        }
    }
    oled_WriteString(line1, Font_7x10, White);

    // second operand and =
    oled_SetCursor(0, OLED_LINE_1_Y);
    if (state == CALC_STATE_INPUT_SECOND_OPERAND || state == CALC_STATE_SHOW_RESULT) {
        if (input_index == 0 && state == CALC_STATE_INPUT_SECOND_OPERAND) {
            snprintf(line2, sizeof(line2), "0=");
        } else {
            snprintf(line2, sizeof(line2), "%s=", input_buffer);
        }
    } else {
        snprintf(line2, sizeof(line2), " ");
    }
    oled_WriteString(line2, Font_7x10, White);

    // result
    oled_SetCursor(0, OLED_LINE_2_Y);
    if (state == CALC_STATE_SHOW_RESULT) {
        snprintf(line3, sizeof(line3), "%ld", result);
    } else {
        snprintf(line3, sizeof(line3), " ");
    }
    oled_WriteString(line3, Font_7x10, White);

    oled_SetCursor(0, OLED_LINE_3_Y);
    calculator_write_info_message();

    oled_UpdateScreen();
}

static void calculator_compute_result(void)
{
    switch(operation) {
        case CALC_OPERATION_ADDITION:
            if (first_number > 0 && second_number > INT32_MAX - first_number) {
                state = CALC_STATE_ERROR;
                return;
            }
            if (first_number < 0 && second_number < INT32_MIN - first_number) {
            	state = CALC_STATE_ERROR;
            	return;
            }
            result = first_number + second_number;
            state = CALC_STATE_SHOW_RESULT;
            break;

        case CALC_OPERATION_SUBTRACTION:
            if (second_number > 0 && first_number < INT32_MIN + second_number) {
            	state = CALC_STATE_ERROR;
            	return;
            }
            if (second_number < 0 && first_number > INT32_MAX + second_number) {
            	state = CALC_STATE_ERROR;
            	return;
            }
            result = first_number - second_number;
            state = CALC_STATE_SHOW_RESULT;
            break;

        case CALC_OPERATION_MULTIPLICATION:
            if (first_number > 0 && second_number > 0) {
                if (first_number > INT32_MAX / second_number) {
                	state = CALC_STATE_ERROR;
                	return;
                }
            }
            else if (first_number < 0 && second_number < 0) {
                if (first_number < INT32_MAX / second_number) {
                	state = CALC_STATE_ERROR;
                	return;
                }
            }
            else if (first_number > 0 && second_number < 0) {
                if (second_number < INT32_MIN / first_number) {
                	state = CALC_STATE_ERROR;
                	return;
                }
            }
            else if (first_number < 0 && second_number > 0) {
                if (first_number < INT32_MIN / second_number) {
                	state = CALC_STATE_ERROR;
                	return;
                }
            }
            result = first_number * second_number;
            state = CALC_STATE_SHOW_RESULT;
            break;

        default:
            result = 0;
            break;
    }
}

void calculator_process_key(uint8_t key)
{
    if ((input_index >= sizeof(input_buffer) - 1) && (key != KEYB_HASH_CODE && key != KEYB_ASTERISK_CODE) ) {
//        state = CALC_STATE_ERROR;
    }

    // 1. enter first number digit by digit
    // 2. press #
    // 3. clicking # select operation
    // 4. enter second number just by pressing first digit
    // 5. press # to calculate result
    // -. pressing * in any state clears the state
    switch(key) {
        case 1: case 2: case 3: case 4: case 5:
        case 6: case 7: case 8: case 9:
            if (input_index < sizeof(input_buffer) - 1) {
                input_buffer[input_index++] = '0' + key;
//                input_buffer[input_index] = '\0';

//                if (state == CALC_STATE_INPUT_FIRST_OPERAND && input_index == 1) {
//                    oled_Fill(Black);
//                }
            }
            break;

        case KEYB_ZERO_CODE:
            if (0 < input_index && input_index < sizeof(input_buffer) - 1) {
                input_buffer[input_index++] = '0';
//                input_buffer[input_index] = '\0';
            }
            break;

        case KEYB_ASTERISK_CODE:
        	calculator_reset();
            break;

        case KEYB_HASH_CODE:
        	if (state == CALC_STATE_INPUT_FIRST_OPERAND && input_index > 0) {
				first_number = atoi(input_buffer);
				input_index = 0;
				calculator_clear_input_buffer();

				state = CALC_STATE_INPUT_OPERATION;
//				oled_Fill(Black);
			}
			else if (state == CALC_STATE_INPUT_OPERATION) {
				operation = (operation + 1) % CALC_OPERATIONS_COUNT;
			}
			else if (state == CALC_STATE_INPUT_SECOND_OPERAND && input_index > 0) {
                second_number = atoi(input_buffer);

                calculator_compute_result();
            }
            break;
    }

    // after selecting operation user enters second number just by typing digits
    if (state == CALC_STATE_INPUT_OPERATION && input_index > 0) {
        state = CALC_STATE_INPUT_SECOND_OPERAND;
    }

    calculator_update_leds();
    calculator_update_display();
}



void calculator_reset(void)
{
    first_number = 0;
    second_number = 0;
    result = 0;
    operation = CALC_OPERATION_ADDITION;
    state = CALC_STATE_INPUT_FIRST_OPERAND;
    input_index = 0;
    calculator_clear_input_buffer();
    oled_Fill(Black);
}

