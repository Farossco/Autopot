#include "mbed.h"
#include "standby.h"

void refresh_RTC_bkp_reg(void)
{
	write_RTC_bkp_reg(BKP0, read_RTC_bkp_reg(BKP0));
    write_RTC_bkp_reg(BKP1, read_RTC_bkp_reg(BKP1));
    write_RTC_bkp_reg(BKP2, read_RTC_bkp_reg(BKP2));
    write_RTC_bkp_reg(BKP3, read_RTC_bkp_reg(BKP3));
    write_RTC_bkp_reg(BKP4, read_RTC_bkp_reg(BKP4));
}

int config_RTC(const uint16_t time)
{	
    enable_bkp_access();
	// RTC Reset
    _SET_BIT(RCC->CSR, RCC_CSR_RTCRST);
    _CLEAR_BIT(RCC->CSR, RCC_CSR_RTCRST);
    
    _write_RTC_bkp_reg(); // update the value of the backup registers
	
	// Reset and Clock Control
    // Power interface clock enabled
    _SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    // Select the RTC clock source: LSE oscillator (32768Hz)
    _SET_BIT(RCC->CSR, RCC_CSR_RTCSEL_LSE);
	// Enable the external low-speed LSE oscillator
	_SET_BIT(RCC->CSR, RCC_CSR_LSEON);

    // Enable write access for RTC registers
	enable_RTC_reg_access();
	
	// RTC configuration
    // Disable wake up timer to modify it
    _CLEAR_BIT(RTC->CR, RTC_CR_WUTE);
    while((RTC->ISR & RTC_ISR_WUTWF) != RTC_ISR_WUTWF) { 
    	// Wait until it is allowed to modify wake up reload value
    }
    
    RTC->WUTR = time-1; // Wake up value reload counter [s]
    RTC->PRER = 0x7F00FF; // ck_spre = 1Hz PREDIV_A = 0x7F(127) PREDIV_S = 0xFF(255) using LSE (32768Hz). ck = (32768)/(PREDIV_A * PREDIV_S)
    // OSEL (output selection) = 0x3 -> RTC_ALARM output = Wake up timer
    _SET_BIT(RTC->CR, RTC_CR_OSEL);
    // WUCKSEL = 0x4 -> RTC Timer [1s - 18h]
    _SET_BIT(RTC->CR, RTC_CR_WUCKSEL_2);
    // Enable wake up counter/interrupt
    _SET_BIT(RTC->CR, RTC_CR_WUTE | RTC_CR_WUTIE);
	
	/* Disable write access for RTC registers */
	disable_RTC_reg_access();
		
	// Re-enable the RTC clock
	_SET_BIT(RCC->CSR, RCC_CSR_RTCEN);
	return 1; // Return TRUE
}

void config_Standby(void)
{
	// Power configuration 
    // Clear the WUF flag
    _SET_BIT(PWR->CR, PWR_CR_CWUF);
	// clear PWR Standby flag
	_SET_BIT(PWR->CR, PWR_CR_CSBF);
	// Enable Fast wake-up
    // V_{REFINT} (internal voltage reference for analog peripherals) is off in low-power mode
    _SET_BIT(PWR->CR, PWR_CR_ULP);
    // Enter Standby mode when the CPU enters deepsleep
    _SET_BIT(PWR->CR, PWR_CR_PDDS);
}

void enter_Standby(void)
{
		// System Control Block
		// Low power mode -> Deep Sleep
		_SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
		// Reenter low-power mode after ISR
		_SET_BIT(SCB->SCR, SCB_SCR_SLEEPONEXIT_Msk);
}

void standby_mode(const uint16_t time)
{
	config_Standby();
	config_RTC(time);
	enter_Standby();
	__WFI(); // Waiting for Interruption -> Enter low-power mode
}

void standby_verification(void)
{
	// check standby flag
    if((PWR->CSR)&(PWR_CSR_SBF)) {
        // clear PWR Wake Up flag
        _SET_BIT(PWR->CR, PWR_CR_CWUF);
        // clear PWR Standby flag
        _SET_BIT(PWR->CR, PWR_CR_CSBF);
        
        printf("\nLa carte se reveille du mode standby\n");
    }
    else {
        printf("\nLa carte se reveille du power cycle\n");
    }
}

void enable_bkp_access(void)
{
	// Disable backup write protection. this bit must be set in order to enable RTC registers write access
    _SET_BIT(PWR->CR, PWR_CR_DBP);
}

void enable_RTC_reg_access(void)
{
	// Enable write access for RTC registers
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53; 
}

void disable_RTC_reg_access(void)
{
	// Disable write access for RTC registers
    RTC->WPR = 0xFE;  
    RTC->WPR = 0x64;  
}

uint32_t read_RTC_bkp_reg(uint32_t bkp_reg)
{
	// RTC_BKPxR, x = 0 - 4
	
	uint32_t bkp_val = 0;
	
	// Reset and Clock Control
    // Power interface clock enabled
    _SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    // Select the RTC clock source: LSE oscillator (32768Hz)
    _SET_BIT(RCC->CSR, RCC_CSR_RTCSEL_LSE);
	// Enable the external low-speed LSE oscillator
	_SET_BIT(RCC->CSR, RCC_CSR_LSEON);
	
	enable_RTC_reg_access();
	
	switch(bkp_reg)
	{
		case 1:
			bkp_val = RTC->BKP0R;
		case 2:
			bkp_val = RTC->BKP1R;
		case 3:
			bkp_val = RTC->BKP2R;
		case 4:
			bkp_val = RTC->BKP3R;
		case 5:
			bkp_val = RTC->BKP4R;
		default:
			bkp_val = RTC->BKP0R;
	}
	
	disable_RTC_reg_access();
	return bkp_val;
}

void write_RTC_bkp_reg(uint32_t bkp_reg, uint32_t value)
{
	RTC_bkp_value[bkp_reg-1] = value;
}

void reset_RTC_bkp_reg(void)
{
	int i;
	for(i = 0; i < 5; i++)
	{
		RTC_bkp_value[i] = 0;
	}
}

void _write_RTC_bkp_reg(void)
{
	// Reset and Clock Control
    // Power interface clock enabled - Reference Manual 7.3.15 (pg 208)
    _SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    enable_bkp_access();
    // Select the RTC clock source: LSE oscillator (32768Hz) - Reference Manual 7.3.21 (pg 220)
    _SET_BIT(RCC->CSR, RCC_CSR_RTCSEL_LSE);
	// Enable the external low-speed LSE oscillator - Reference Manual 7.3.21 (pg 220)
	_SET_BIT(RCC->CSR, RCC_CSR_LSEON);
	
	enable_RTC_reg_access();

	RTC->BKP0R = RTC_bkp_value[0];
	RTC->BKP1R = RTC_bkp_value[1];
	RTC->BKP2R = RTC_bkp_value[2];
	RTC->BKP3R = RTC_bkp_value[3];
	RTC->BKP4R = RTC_bkp_value[4];

	disable_RTC_reg_access();
}

void config_GPIO(void)
{
	// port A (reset state)
	GPIOA->MODER = 0xFCFFU; // Select Analog mode for all GPIOs
	GPIOA->PUPDR = 0x0000U; // All GPIOs set to no pull-up, pull-down
	_CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN); // Disable the peripherical clock for GPIOA

	// port B (reset state)
	GPIOB->MODER = 0xFFFFU; // Select Analog mode for all GPIOs
	GPIOB->PUPDR = 0x0000U; // All GPIOs set to no pull-up, pull-down
	_CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN); // Disable the peripherical clock for GPIOA

	// port C (reset state)
	GPIOC->MODER = 0xFFFFU; // Select Analog mode for all GPIOs
	GPIOC->PUPDR = 0x0000U; // All GPIOs set to no pull-up, pull-down
	_CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN); // Disable the peripherical clock for GPIOA
}