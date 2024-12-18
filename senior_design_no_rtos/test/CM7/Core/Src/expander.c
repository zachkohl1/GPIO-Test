/*
 * expander.c
 *
 *  Created on: Nov 29, 2024
 *      Author: kohlmanz
 *  Description: Implemntation of MCP23S17 GPIO Expander driver.
 *  This module provides interrupt-driven SPI communication for
 *  configuring and controlling MCP23S17 GPIO expander chips.
 */

#include "expander.h"
/**
 * @brief Initialize an MCP23S17 GPIO Expander
 *
 * @param expander Pointer to the Expander structure to initialize
 * @param num Expander number (used for chip select and identification)
 * @param hspi Pointer to the SPI handle for communication
 *
 * @return void
 */
HAL_StatusTypeDef Expander_Init(Expander* expander, uint8_t num, SPI_HandleTypeDef* hspi, uint8_t port)
{
    expander->hspi = hspi;
    expander->num = num;
    expander->state = DEFAULT_STATE;

    // Set up addressing schemes - hardware and register
    if(port == 0)
    {
    	// Set HAEN for hardware addressing (Bit 3 in IOCON register (4th bit including 0))
    	if(Expander_WriteRegister(expander,MCP23S17_IOCONA, (1 << IOCON_INTPOL_BIT) | (0 << IOCON_ODR_BIT) | (1 << IOCON_HAEN_BIT) | (1 << IOCON_DISSLW_BIT) | (0 << IOCON_SEQOP_BIT) | (0 << IOCON_INT_MIRROR_BIT) | (0 << IOCON_BANK_BIT)))
    		return HAL_ERROR;
    }
    else
    {
  	  if(Expander_WriteRegister(expander,MCP23S17_IOCONB,  IOCON_HAEN_BIT))
  		  return HAL_ERROR;
    }
    // If using interrupts, write to appropriate pins and registers
	#ifdef USING_SPI_INT
    if(port == 0)
    {
		Expander_WriteRegister(&expander1, MCP23S17_DEFVALA, (1 << 0));			// Set default value to 1, for interrupt on change
		Expander_WriteRegister(&expander1, MCP23S17_GPINTENA, 0xFF);			// Enable interrupts on all GPIOA pins
    }
    else
    {
		Expander_WriteRegister(&expander1, MCP23S17_DEFVALB, (1 << 0));			// Set default value to 1, for interrupt on change
		Expander_WriteRegister(&expander1, MCP23S17_GPINTENB, 0xFF);			// Enable interrupts on all GPIOA pins
    }

	#endif
    return HAL_OK;
}

/**
 * @brief Configure GPIO pin direction for each pin on a port on the expander
 *			Writes IO direction
 * @param expander Pointer to the Expander structure
 * @param port Port to configure (0 for GPIOA, 1 for GPIOB)
 * @param direction Bit configuration for EACH pin direction
 *        (0 = output, 1 = input). Expects direction FOR EACH PIN
 *        Ex:	IO7		IO6		IO5		IO4		IO3		IO2		IO1		IO0
 *       		0		0		0		0		0		1		0		1
 *			To set pins GPIO2 and GPIO0 as inputs.
 * @return HAL_StatusTypeDef Status of the configuration operation
 */
HAL_StatusTypeDef Expander_ConfigureGPIO(Expander* expander, uint8_t port, uint8_t direction)
{
    uint8_t reg = (port == 0) ? MCP23S17_IODIRA : MCP23S17_IODIRB;
    return Expander_WriteRegister(expander, reg, direction);
}

/**
 * @brief Configure internal pull-up resistors for a specific port
 *
 * @param expander Pointer to the Expander structure
 * @param port Port to configure (0 for GPIOA, 1 for GPIOB)
 * @param pullup_config Bit configuration for pull-up resistors
 *        (1 = pull-up enabled, 0 = pull-up disabled)
 *
 * @return HAL_StatusTypeDef Status of the pull-up configuration operation
 */
HAL_StatusTypeDef Expander_ConfigurePullUps(Expander* expander, uint8_t port, uint8_t pullup_config)
{
    uint8_t reg = (port == 0) ? MCP23S17_GPPUA : MCP23S17_GPPUB;
    return Expander_WriteRegister(expander, reg, pullup_config);
}

/**
 * @brief Write to a register using interrupt-driven SPI transmission
 *
 * @param expander Pointer to the Expander structure
 * @param reg Register address to write to
 * @param data Data to write to the register
 *
 * @return HAL_StatusTypeDef Status of the write operation
 * @note Prevents writing if expander is not in IDLE state
 */
HAL_StatusTypeDef Expander_WriteRegister(Expander* expander, uint8_t reg, uint8_t data)
{
	// Make sure no errors in parameters
	if(!expander || !expander->hspi)
		return HAL_ERROR;

	// If we aren't in an idle state, then that means another transaction in progress. Return busy are waiting for an
	// Used for the interrupt version
//    if(expander->state != IDLE)
//        return HAL_BUSY;

    // Store write variables - NOTE: write_data is only set in this function and WILL NOT be set if trying to read which will first write to the expander and wait.
    expander->write_reg = reg;		// Register address to write to
    expander->write_data = data;	// Data to write - register pin states (on/off, 0/1)
    expander->state = WRITING;		// We are now writing data to the expander, switch the state

    // Data to send - Control byte, register to write to, data to write
    const uint8_t tx_buffer[2] = {WRITE_CONTROL_BYTE_1, expander->write_reg};

    // Use appropriate expander CS pins
    if(expander->num == 1)
    {
        MCP23S17_CS_LOW_1();
    }
    else
    {
    	MCP23S17_CS_LOW_2();
    }

    // Use appropriate write
	#ifdef USING_SPI_INT
		return HAL_SPI_Transmit_IT(expander->hspi, tx_buffer, sizeof(tx_buffer));
	#endif

    return HAL_SPI_Transmit(expander->hspi, tx_buffer, sizeof(tx_buffer), 100);
}

/**
 * @brief Read from a register using interrupt-driven SPI transmission
 *
 * @param expander Pointer to the Expander structure
 * @param reg Register address to read from
 *
 * @return HAL_StatusTypeDef Status of the read operation
 * @note Prevents reading if expander is not in IDLE state
 */
HAL_StatusTypeDef Expander_ReadRegister(Expander* expander, uint8_t reg)
{
	// If we aren't in an idle state, then that means we are waiting for another transaction. Return busy
    if(expander->state != IDLE)
        return HAL_BUSY;

    // Set expander read and state variables
    expander->read_reg = reg;
    expander->state = READING;

    // To interface with the expander chip, must first send transaction asking to 1. Read and 2. From which register
    const uint8_t tx_buffer[2] = {READ_CONTROL_BYTE_1, reg};

    // Use appropriate expander CS pins
    if(expander->num == 1)
    {
        MCP23S17_CS_LOW_1();
    }
    else
    {
    	MCP23S17_CS_LOW_2();
    }

    // Send data in tx_buffer

#ifdef USING_SPI_INT
    return HAL_SPI_Transmit_IT(expander->hspi, tx_buffer, sizeof(tx_buffer));
#endif

    return HAL_SPI_Transmit(expander->hspi, tx_buffer, sizeof(tx_buffer), 100);
}

/**
 * @brief Read GPIO pin states for a specific port
 *
 * @param expander Pointer to the Expander structure
 * @param port Port to read (0 for GPIOA, 1 for GPIOB)
 * @param value Pointer to store the read GPIO pin states
 *
 * @return HAL_StatusTypeDef Status of the GPIO read operation
 */
HAL_StatusTypeDef Expander_ReadGPIO(Expander* expander, uint8_t port, uint8_t* value)
{
    uint8_t reg = (port == 0) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    return Expander_ReadRegister(expander, reg);
}

/**
 * @brief Write to GPIO pins for a specific port
 *
 * @param expander Pointer to the Expander structure
 * @param port Port to write to (0 for GPIOA, 1 for GPIOB)
 * @param value Bit values to write to the GPIO pins
 *
 * @return HAL_StatusTypeDef Status of the GPIO write operation
 */
HAL_StatusTypeDef Expander_WriteGPIO(Expander* expander, uint8_t port, uint8_t value)
{
    uint8_t reg = (port == 0) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    return Expander_WriteRegister(expander, reg, value);
}






