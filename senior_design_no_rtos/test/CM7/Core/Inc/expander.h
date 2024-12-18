/*
 * expander.h
 *
 *  Created on: Nov 29, 2024
 *      Author: kohlmanz
 */

#ifndef APPLICATION_USER_CORE_EXPANDER_H_
#define APPLICATION_USER_CORE_EXPANDER_H_
#include "stm32h7xx_hal.h"

// COMMENT THIS VAR OUT IF WANT TO USE POLLING
//#define USING_SPI_INT			1


// Expander 1 CS defs
#define MCP23S17_CS_PIN_1		GPIO_PIN_4		// CHANGE AS NEEDED
#define MCP23S17_CS_PORT_1 		GPIOB			// CHANGE AS NEEDED
#define MCP23S17_CS_HIGH_1() 	HAL_GPIO_WritePin(MCP23S17_CS_PORT_1, MCP23S17_CS_PIN_1, GPIO_PIN_RESET);
#define MCP23S17_CS_LOW_1() 	HAL_GPIO_WritePin(MCP23S17_CS_PORT_1, MCP23S17_CS_PIN_1, GPIO_PIN_SET);


// Expander 2 cs defs
#define MCP23S17_CS_PIN_2		GPIO_PIN_15
#define MCP23S17_CS_PORT_2		GPIOH
#define MCP23S17_CS_HIGH_2() 	HAL_GPIO_WritePin(MCP23S17_CS_PORT_2, MCP23S17_CS_PIN_2, GPIO_PIN_RESET);
#define MCP23S17_CS_LOW_2() 	HAL_GPIO_WritePin(MCP23S17_CS_PORT_2, MCP23S17_CS_PIN_2, GPIO_PIN_SET);




// See SPI Control Byte Format - Figure 3-5.
// Four fixed bits (0100), and three user-defined hardware address bits (A2,A1,A0)
//			7	 6   5   4   3    2    1     0
// Format: | 0 | 1 | 0 | 0 | A2 | A1 | A0 | R/W |							// HW ADDR 000 DISABLES HW ADDRESSABLE MODE. MUST ENABLE IOCON.HAEN FOR HW ADDRESSABLE MODE
#define MCP23S17_HW_ADDR_1      ((uint8_t)((1<<1) | (0<<2) | (0<<3)))		// Hw addr 001. Tie hardware address bits to ground (hw_address 0). Used to address multiple expanders connected to same SPI bus. CHANGE AS NEEDED
#define MCP23S17_HW_ADDR_2      ((uint8_t)((0<<1) | (1<<2) | (0<<3)))		// HW addr 010

#define MCP23S17_ADDRESS    	((uint8_t)(1<<6)) 		// Upper four fixed bits (0100). CHANGE AS NEEDED.


#define MCP23S17_W          	((uint8_t)(0<<0))		// Write command
#define MCP23S17_R          	((uint8_t)(1<<0))		// Read command

// Expander 1
#define READ_CONTROL_BYTE_1		((uint8_t)((MCP23S17_ADDRESS) | ((MCP23S17_HW_ADDR_1) << 1) | (MCP23S17_R)))
#define WRITE_CONTROL_BYTE_1	((uint8_t)((MCP23S17_ADDRESS) | ((MCP23S17_HW_ADDR_1) << 1) | (MCP23S17_W)))

// Expander 2
#define READ_CONTROL_BYTE_2		((uint8_t)((MCP23S17_ADDRESS) | ((MCP23S17_HW_ADDR_2) << 1) | (MCP23S17_R)))
#define WRITE_CONTROL_BYTE_2	((uint8_t)((MCP23S17_ADDRESS) | ((MCP23S17_HW_ADDR_2) << 1) | (MCP23S17_W)))


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Registers associated with GPIO PORTS (BANK = 0) */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Register addresses - GPIOA */
#define MCP23S17_IODIRA     	((uint8_t)0x00)		// I/O direction A. When a bit is set, the corresponding pin becomes an input. When a bit is clear, the corresponding pin becomes an output.
													// [7..0]
														// 1 = Pin is configured as an input
														// 0  =Pin is configured as an output
#define MCP23S17_IPOLA      	((uint8_t)0x02)		// I/O polarity A. If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
													// [7..0]
														// 1 = GPIO register bit reflects the opposite logic state of the input pin
														// 0 = GPIO register bit reflects the opposite logic state of the input pin
#define MCP23S17_GPINTENA   	((uint8_t)0x04)		// Interrupt enable A. If a bit is set, the corresponding pin is enabled for interrupt-on-change. The DEFVAL and INTCON registers must also be configured if any pins are enabled for interrupt-on-change.
													// [7..0]
														// 1 = Enables GPIO input pin for interrupt-on-change event.
														// 0 = Disables GPIO input pin for interrupt-on-change event.
#define MCP23S17_DEFVALA    	((uint8_t)0x06)		// Register default value A (interrupts). The default comparison value is configured in the DEFVAL register. If enabled (via GPINTEN and INTCON) to compare against the DEFVAL register, an opposite value on the associated pin will cause an interrupt to occur.
													// [7..0]
#define MCP23S17_INTCONA    	((uint8_t)0x08)		// Interrupt control A. If a bit is set, the corresponding I/O pin is compared against the associated bit in the DEFVAL register. If a bit value is clear, the corresponding I/O pin is compared against the previous value.
													// [7..0]
														// 1 = If a bit is set, the corresponding I/O pin is compared against the associated bit in the DEFVAL register. If a bit value is clear, the corresponding I/O pin is compared against the previous value.
														// 0  = If a bit is set, the corresponding I/O pin is compared against the associated bit in the DEFVAL register. If a bit value is clear, the corresponding I/O pin is compared against the previous value.
#define MCP23S17_IOCONA     	((uint8_t)0x0A)		// I/O config (also 0x08). See below for information
													// [7..0]
#define MCP23S17_GPPUA      	((uint8_t)0x0C)		// Port A pull ups.  If a bit is set and the corresponding pin is configured as an input, the corresponding port pin is internally pulled up with a 100 k resistor.
													// [7..0]
														// 1 = Pull-up enable
														// 0  = Pull-up disabled
#define MCP23S17_INTFA      	((uint8_t)0x0E)		// Interrupt flag A (where the interrupt came from).  A set bit indicates that the associated pin caused the interrupt.
													//[7..0]
														// 1 =  Pin caused interrupt
														// 0 = Interrupt not pending
#define MCP23S17_INTCAPA    	((uint8_t)0x10)		// Interrupt capture A (value at interrupt is saved here). The INTCAP register captures the GPIO port value at the time the interrupt occurred. The register is read-only and is updated only when an interrupt occurs. The register remains unchanged until the interrupt is cleared via a read of INTCAP or GPIO.
													// [7..0]
														// 1 = Logic-high
														// 0  = Logic-low
#define MCP23S17_GPIOA      	((uint8_t)0x12)		// Port A. The GPIO register reflects the value on the port. Reading from this register reads the port. Writing to this register modifies the Output Latch (OLAT) register.
													// [7..0]
														// 1 = Logic-high
														// 0  = Logic-low
#define MCP23S17_OLATA      	((uint8_t)0x14)		// Output latch A. The OLAT register provides access to the output latches. A read from this register results in a read of the OLAT and not the port itself. A write to this register modifies the output latches that modifies the pins configured as outputs.
													// [7..0]
														// 1 = Logic-high
														// 0 = Logic-low

/* Register addresses - GPIOB - See IOCON.BANK = 0 Register Addressing */
#define MCP23S17_IODIRB     	((uint8_t)0x01)		// I/O direction B
#define MCP23S17_IPOLB      	((uint8_t)0x03)		// I/O polarity B
#define MCP23S17_GPINTENB   	((uint8_t)0x05)		// Interrupt enable B
#define MCP23S17_DEFVALB    	((uint8_t)0x07)		// Register default value B (interrupts)
#define MCP23S17_INTCONB    	((uint8_t)0x09)		// Interrupt control B
#define MCP23S17_IOCONB			((uint8_t)0x0B)		// IO control B
#define MCP23S17_GPPUB      	((uint8_t)0x0D)		// Port B pull ups
#define MCP23S17_INTFB      	((uint8_t)0x0F)		// Interrupt flag B (where the interrupt came from)
#define MCP23S17_INTCAPB    	((uint8_t)0x11)		// Interrupt capture B (value at interrupt is saved here)
#define MCP23S17_GPIOB      	((uint8_t)0x13)		// Port B
#define MCP23S17_OLATB      	((uint8_t)0x15)		// Output latch B
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* I/O configuration register bits (IOCON) */
// 		7		6		5		4		3		2		1		0
// | BANKL | MIRROR | SEQOP | DISSLW | HAEN | ODER | INTERPOL | unimplemented = 0

#define IOCON_BANK_BIT 			((uint8_t)7)	// Controls how the registers are addressed - should be 0 for our application
														// (1 = The registers associated with each port are separated into different banks.)
														// 2 = The registers are in the same bank (addresses are sequential).
#define IOCON_INT_MIRROR_BIT 	((uint8_t)6)  	// Interrupt mirror (INTa|INTb)
														// 1 = The INT pins are internally connected
														// 0 = The INT pins are not connected. INTA is associated with PORTA and INTB is associated with PORTB
#define IOCON_SEQOP_BIT 		((uint8_t)5)  	// Sequential Operation mode bit - Should be = 0
														// 1 = Sequential operation disabled, address pointer does not increment.
														// 0 = Sequential operation enabled, address pointer increments.
#define IOCON_DISSLW_BIT 		((uint8_t)4)  	// Slew Rate control bit for SDA output - Not in SPI
														// 1 = Slew Rate disabled
														// 0 = Slew Rate enabled
#define IOCON_HAEN_BIT 			((uint8_t)3)  	// Hardware Address Enable bit - Should be = 1 for our application
														// 1 = Enables the MCP23S17 address pins
														// 0 = Enables the MCP23S17 address pins
#define IOCON_ODR_BIT 			((uint8_t)2)  	//  Configures the INT pin as an open-drain output
														// 1 = Open-drain output (overrides the INTPOL bit.)
														// 0 = Active driver output (INTPOL bit sets the polarity.)
#define IOCON_INTPOL_BIT 		((uint8_t)1)  	// Sets the polarity of the INT output pin
														// 1 = Active-high
typedef enum
{
	IDLE,
	WRITING,
	READING
}ExpanderState;

#define DEFAULT_STATE IDLE

// Expander 1
#define EXPANDER1_NUM 1
#define EXPANDER2_NUM 2

#define MAX_WRITE 1000
#define MAX_READ 1000
typedef struct {
    SPI_HandleTypeDef* hspi;
    uint8_t num;	// Expander num
    uint8_t write_reg;
    uint8_t write_data;
    uint8_t read_reg;
    uint8_t read_data;
    ExpanderState state;
} Expander;
HAL_StatusTypeDef Expander_Init(Expander* expander, uint8_t num, SPI_HandleTypeDef* hspi, uint8_t port);
HAL_StatusTypeDef Expander_ConfigureGPIO(Expander* expander, uint8_t port, uint8_t direction);
HAL_StatusTypeDef Expander_ConfigurePullUps(Expander* expander, uint8_t port, uint8_t pullup_config);
HAL_StatusTypeDef Expander_WriteRegister(Expander* expander, uint8_t reg, uint8_t data);
HAL_StatusTypeDef Expander_ReadRegister(Expander* expander, uint8_t reg);
HAL_StatusTypeDef Expander_ReadGPIO(Expander* expander, uint8_t port, uint8_t* value);
HAL_StatusTypeDef Expander_WriteGPIO(Expander* expander, uint8_t port, uint8_t value);
#endif /* APPLICATION_USER_CORE_EXPANDER_H_ */
