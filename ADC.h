// put prototypes for public functions, explain what it does
// put your names here, LUKE Y COLE L NOV 28

#include <stdint.h>

void ADC0_Init(void);

// Converts ADC data to linear data based on linera regression
// Minimum value is 0, Maximum value is 2000
unsigned long Convert(uint32_t input);

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC0_In(void);
