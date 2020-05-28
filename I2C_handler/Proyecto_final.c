
/* Tipo de datos */ 
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef _Bool bool;

#define	false	0
#define	true	1

/* Registros canal 0 (cero) I2C */

#define I2C0_REGISTERS        ((I2C_T  *)  0x400A1000)
#define I2C1_REGISTERS        ((I2C_T  *)  0x400E0000)

typedef struct {
	uint32_t CONSET;	
	uint32_t STAT;	
	uint32_t DAT;
	uint32_t ADR0;
	uint32_t SCLH;
	uint32_t SCLL;
	uint32_t CONCLR;	
	uint32_t MMCTRL;
	uint32_t ADR1;
    uint32_t ADR2;
    uint32_t ADR3;
	uint32_t DATA_BUFFER;
	uint32_t MASK[4];     

} I2C_T;

/*Macros.*/
/*Escribir todas las configuraciones de los registros*/
/*Flags del control set register*/
#define CONSET_AA           0x04 /*100 en binario*/
#define CONSET_SI           0x08 /*1000 en binario*/
#define CONSET_STO          0x10 /*10000 en binario*/
#define CONSET_STA          0x20 /*100000 en binario*/
#define CONSET_I2EN         0x40 /*1000000 en binario*/

/*Registro STAT. Solo de lectura*/
#define STAT_MASK           0xF8 /*11111000 en binario*/

/*Registro DAT.*/
#define DATA_MASK           0xFF /*11111111 en binario. Para que al comparar me quede el dato que tengo*/

/*Registros ADR*/
#define ADR_GC              (1 << 0)
#define ADR_MASK            0xFF /*11111111*/

/*Registros SCL high y low. Para definir la forma del clk*/
#define SCLH_MASK            0xFFFF
#define SCLL_MASK            0xFFFF

/*Flags del control clear register*/
#define CONSET_AA_C         0x04 /*100 en binario*/
#define CONSET_SI_C         0x08 /*1000 en binario*/
/*#define CONSET_STO_C 0x10 /*10000 en binario*/
#define CONSET_STA_C        0x20 /*100000 en binario*/
#define CONSET_I2EN_C       0x40 /*1000000 en binario*/

/*Flags de monitoreo*/
#define MMCTRL_ENA         (1 << 0)
#define MMCTRL_ENA_SCL     (1 << 1)
#define MMCTRL_MATCH_ALL   (1 << 2)
/*MMCTRL_MASK 0x07*/

/*Buffer*/
#define BUFFER_MASK         0xFF

/*Masks*/
/*#define MASK_MASK(n) (n & 0xFE)*/

// Estructura para Master Mode.
/* uint16_t Porque quizas se necesita usar un buffer ya se para transmitir
o recibir de mas de 2^8 = 256 elementos */
typedef struct{
    uint8_t SLAVE_ADDR; /* Le paso el slave address */
    uint8_t OPTIONS;	/* Option, todavia no sabemos */
	uint8_t STATUS;		/* Estado de transmision */
	uint16_t Tx_SIZE;		/* Tama;o del buffer a transmitir (Numero de bytes), Transmito el Address si recivo */
	uint16_t Rx_SIZE;		/* Tama;o del buffer a recibir (Numero de bytes), 0 si transmito*/
	uint8_t *Tx_BUFFER; /* Puntero al buffer de datos a transmitir, NULL si recibo */
	uint8_t *Rx_BUFFER;
     
}MasterTransfer_T;

void I2CM_Xfer(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
{
	/* set the transfer status as busy */
	xfer->status = I2CM_STATUS_BUSY;
	/* Clear controller state. */
	Chip_I2CM_ResetControl(pI2C);
	/* Enter to Master Transmitter mode */
	Chip_I2CM_SendStart(pI2C);
}

/**
 * @brief	Reset I2C controller state
 * @param	pI2C	: Pointer to selected I2C peripheral
 * @return	Nothing
 * @note	This function clears all control/status flags.
 */
void I2CM_ResetControl(LPC_I2C_T *pI2C)
{
	/* Reset STA, AA and SI. Stop flag should not be cleared as it is a reserved bit */
	pI2C->CONCLR = I2C_CON_SI | I2C_CON_STA | I2C_CON_AA;

}
/**
 * @brief	Transmit START or Repeat-START signal on I2C bus
 * @param	pI2C	: Pointer to selected I2C peripheral
 * @return	Nothing
 * @note	This function sets the controller to transmit START condition when
 *          the bus becomes free.
 */
void I2CM_SendStart(LPC_I2C_T *pI2C)
{
	pI2C->CONSET = I2C_CON_I2EN | I2C_CON_STA;
}

