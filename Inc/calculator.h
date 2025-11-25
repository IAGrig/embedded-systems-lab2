#ifndef INC_CALCULATOR_H_
#define INC_CALCULATOR_H_

#include <stdint.h>

typedef enum {
    CALC_STATE_INPUT_FIRST_OPERAND = 0,
    CALC_STATE_INPUT_OPERATION,
    CALC_STATE_INPUT_SECOND_OPERAND,
    CALC_STATE_SHOW_RESULT,
    CALC_STATE_ERROR,
	CALC_STATES_COUNT
} CalculatorState;

typedef enum {
    CALC_OPERATION_ADDITION = 0,
    CALC_OPERATION_SUBTRACTION,
	CALC_OPERATION_MULTIPLICATION,
	CALC_OPERATIONS_COUNT
} CalculatorOperation;

static const char CalculatorOperationSymbols[CALC_OPERATIONS_COUNT] = {
    [CALC_OPERATION_ADDITION] = '+',
    [CALC_OPERATION_SUBTRACTION] = '-',
    [CALC_OPERATION_MULTIPLICATION] = '*'
};

void calculator_init(void);
void calculator_process_key(uint8_t key);
void calculator_reset(void);


#endif /* INC_CALCULATOR_H_ */
