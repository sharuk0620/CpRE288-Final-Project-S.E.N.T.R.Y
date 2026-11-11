//#ifndef ADC_H_
//#define ADC_H_


//Initializes GPIO and ADC registers
void adc_init(void);

//Read a converted value
int adc_read(void);

//Convert an adc dig. value into a distance (cm)
float adc_convert(int adc_dig_val);

//#endif ADC_H_
