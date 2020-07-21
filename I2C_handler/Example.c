#include <stdlib.h>
#include <string.h>
#include "board.h"

/* si DEBUG_ENABLE esta definido entonces defini DEFAULT_I2C sino defini DEFAULT_I2C */
#ifdef DEBUG_ENABLE
#define DEFAULT_I2C          I2C0
#else
#define DEFAULT_I2C          I2C1
#endif

/* Le asigno a los buses los canales de I2C*/
#define I2C_EEPROM_BUS       DEFAULT_I2C
#define I2C_IOX_BUS          DEFAULT_I2C

/*Defino Velocidades*/
#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000

/* Si esta definida DEBUG_ENABLE crea el menu */
#ifdef DEBUG_ENABLE
static const char menu[] =
	"**************** I2C Demo Menu ****************\r\n"
	"\t0: Exit Demo\r\n"
	"\t1: Select I2C peripheral [\033[1;32mI2C%d\033[0;37m]\r\n"
	"\t2: Toggle mode POLLING/INTERRUPT [\033[1;32m%s\033[0;37m]\r\n"
	"\t3: Probe for Slave devices\r\n"
	"\t4: Read slave data\r\n"
	"\t5: Write slave data\r\n"
	"\t6: Write/Read slave data\r\n";
#endif
/***********************************************************************/
/*Del archivo i2c_18xx_43xx.h*/
typedef enum I2C_ID {
	I2C0,				/**< ID I2C0 */
	I2C1,				/**< ID I2C1 */
	I2C_NUM_INTERFACE	/**< Number of I2C interfaces in the chip */
} I2C_ID_T;
/***********************************************************************/

/* Define una variable i2cDev del tipo I2C_ID_T y le asigna el canal a usar*/
static I2C_ID_T i2cDev = DEFAULT_I2C;	/* Currently active I2C device */

/* Define el modo en que se va operar, en este caso es por polling pero podria ser 
a traves de interrupcion*/
static int mode_poll;	/* Poll/Interrupt mode flag */

/* EEPROM SLAVE data */
#define I2C_SLAVE_EEPROM_SIZE       64
#define I2C_SLAVE_EEPROM_ADDR       0x5A
#define I2C_SLAVE_IOX_ADDR          0x5B

/***********************************************************************/
/*Del archivo i2c_18xx_43xx.h*/
typedef enum {
	I2C_STATUS_DONE,	/**< Transfer done successfully */
	I2C_STATUS_NAK,		/**< NAK received during transfer */
	I2C_STATUS_ARBLOST,	/**< Aribitration lost during transfer */
	I2C_STATUS_BUSERR,	/**< Bus error in I2C transfer */
	I2C_STATUS_BUSY,	/**< I2C is busy doing transfer */
	I2C_STATUS_SLAVENAK,/**< NAK received after SLA+W or SLA+R */
} I2C_STATUS_T;

typedef struct {
	uint8_t slaveAddr;		/**< 7-bit I2C Slave address */
	const uint8_t *txBuff;	/**< Pointer to array of bytes to be transmitted */
	int     txSz;			/**< Number of bytes in transmit array,
							   if 0 only receive transfer will be carried on */
	uint8_t *rxBuff;		/**< Pointer memory where bytes received from I2C be stored */
	int     rxSz;			/**< Number of bytes to received,
							   if 0 only transmission we be carried on */
	I2C_STATUS_T status;	/**< Status of the current I2C transfer */
} I2C_XFER_T;
/***********************************************************************/

/*Define las estructuras de los slaves. */
/* Xfer structure for slave operations */
static I2C_XFER_T seep_xfer;
static I2C_XFER_T iox_xfer;

/* Define los espacios de memoria donde va a trabajar el slave*/
/* Data area for slave operations */
static uint8_t seep_data[I2C_SLAVE_EEPROM_SIZE + 1]; /* Porque +1? */
static uint8_t buffer[2][256]; /*2 row 256 columns*/
static uint8_t iox_data[2];	/* PORT0 input port, PORT1 Output port */


/***********************************************************************/
/*Del archivo i2c_18xx_43xx.h*/
typedef enum {
	I2C_EVENT_WAIT = 1,	/**< I2C Wait event */
	I2C_EVENT_DONE,		/**< Done event that wakes up Wait event */
	I2C_EVENT_LOCK,		/**< Re-entrency lock event for I2C transfer */
	I2C_EVENT_UNLOCK,	/**< Re-entrency unlock event for I2C transfer */
	I2C_EVENT_SLAVE_RX,	/**< Slave receive event */
	I2C_EVENT_SLAVE_TX,	/**< Slave transmit event */
} I2C_EVENT_T;

/**
 * @brief	Event handler function type
 */
/* Devuelve un puntero que apunta a una funcion que recibe parametros del tipo
 I2C_ID_T, I2C_EVENT_T */
typedef void (*I2C_EVENTHANDLER_T)(I2C_ID_T, I2C_EVENT_T);


/*Del archivo i2c_18xx_43xx.c*/
#define I2C_CON_FLAGS (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)
#define LPC_I2Cx(id)      ((i2c[id].ip))
#define SLAVE_ACTIVE(iic) (((iic)->flags & 0xFF00) != 0)

/* I2C interfaces */
/* I2C common interface structure */
struct i2c_interface {
	LPC_I2C_T *ip;		/* IP base address of the I2C device */
	CHIP_CCU_CLK_T clk;	/* Clock used by I2C */
	I2C_EVENTHANDLER_T mEvent;	/* Current active Master event handler */
	I2C_EVENTHANDLER_T sEvent;	/* Slave transfer events */
	I2C_XFER_T *mXfer;	/* Current active xfer pointer */
	I2C_XFER_T *sXfer;	/* Pointer to store xfer when bus is busy */
	uint32_t flags;		/* Flags used by I2C master and slave */
};

/* Slave interface structure */
struct i2c_slave_interface {
	I2C_XFER_T *xfer;
	I2C_EVENTHANDLER_T event;
};

/* con esto setea los campos de la estructura i2c_interface dependiendo de I2C_NUM_INTERFACE
esta escrito como si fuera un array de 2 elementos.*/
static struct i2c_interface i2c[I2C_NUM_INTERFACE] = {
	{LPC_I2C0, CLK_APB1_I2C0, Chip_I2C_EventHandler, NULL, NULL, NULL, 0},
	{LPC_I2C1, CLK_APB3_I2C1, Chip_I2C_EventHandler, NULL, NULL, NULL, 0}
};

// i2c[id].mEvent(id, I2C_EVENT_DONE);

/*Se fija si el canal esta disponible basicamente. */
/* Chip event handler interrupt based */
void Chip_I2C_EventHandler(I2C_ID_T id, I2C_EVENT_T event)
{
	struct i2c_interface *iic = &i2c[id];
	volatile I2C_STATUS_T *stat;

    /* Sale unicamente si el canal esta disponible*/
	/* Only WAIT event needs to be handled */
	if (event != I2C_EVENT_WAIT) {
		return;
	}
    /* Se queda loopeando hasta que haya un cambio en el estado */
	stat = &iic->mXfer->status;
	/* Wait for the status to change */
	while (*stat == I2C_STATUS_BUSY) {}
}

typedef struct {				/* I2C0 Structure         */
	__IO uint32_t CONSET;		/*!< I2C Control Set Register. When a one is written to a bit of this register, the corresponding bit in the I2C control register is set. Writing a zero has no effect on the corresponding bit in the I2C control register. */
	__I  uint32_t STAT;			/*!< I2C Status Register. During I2C operation, this register provides detailed status codes that allow software to determine the next action needed. */
	__IO uint32_t DAT;			/*!< I2C Data Register. During master or slave transmit mode, data to be transmitted is written to this register. During master or slave receive mode, data that has been received may be read from this register. */
	__IO uint32_t ADR0;			/*!< I2C Slave Address Register 0. Contains the 7-bit slave address for operation of the I2C interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave responds to the General Call address. */
	__IO uint32_t SCLH;			/*!< SCH Duty Cycle Register High Half Word. Determines the high time of the I2C clock. */
	__IO uint32_t SCLL;			/*!< SCL Duty Cycle Register Low Half Word. Determines the low time of the I2C clock. SCLL and SCLH together determine the clock frequency generated by an I2C master and certain times used in slave mode. */
	__O  uint32_t CONCLR;		/*!< I2C Control Clear Register. When a one is written to a bit of this register, the corresponding bit in the I2C control register is cleared. Writing a zero has no effect on the corresponding bit in the I2C control register. */
	__IO uint32_t MMCTRL;		/*!< Monitor mode control register. */
	__IO uint32_t ADR1;			/*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave responds to the General Call address. */
	__IO uint32_t ADR2;			/*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave responds to the General Call address. */
	__IO uint32_t ADR3;			/*!< I2C Slave Address Register. Contains the 7-bit slave address for operation of the I2C interface in slave mode, and is not used in master mode. The least significant bit determines whether a slave responds to the General Call address. */
	__I  uint32_t DATA_BUFFER;	/*!< Data buffer register. The contents of the 8 MSBs of the DAT shift register will be transferred to the DATA_BUFFER automatically after every nine bits (8 bits of data plus ACK or NACK) has been received on the bus. */
	__IO uint32_t MASK[4];		/*!< I2C Slave address mask register */
} LPC_I2C_T;

/* Master transfer state change handler handler */
int handleMasterXferState(LPC_I2C_T *pI2C, I2C_XFER_T  *xfer)
{
	uint32_t cclr = I2C_CON_FLAGS;

	switch (getCurState(pI2C)) {
	case 0x08:		/* Start condition on bus */
	case 0x10:		/* Repeated start condition */
		pI2C->DAT = (xfer->slaveAddr << 1) | (xfer->txSz == 0);
		break;

	/* Tx handling */
	case 0x18:		/* SLA+W sent and ACK received */
	case 0x28:		/* DATA sent and ACK received */
		if (!xfer->txSz) {
			cclr &= ~(xfer->rxSz ? I2C_CON_STA : I2C_CON_STO);
			/* si xfer -> rxSz es true entonces entonces hace lo que 
			esta antes de los dos puntos, si es false lo que esta despues.
			si rxSz es >0 setea un cero en STA entonces setea en STA en CONSET
			y cuando rxSz == 0 le pone un 0 en STO.
			
			
			*/
		}
		else {
			pI2C->DAT = *xfer->txBuff++;
			xfer->txSz--;
		}
		break;

	/* Rx handling */
	case 0x58:		/* Data Received and NACK sent */
		cclr &= ~I2C_CON_STO;

	case 0x50:		/* Data Received and ACK sent */
		*xfer->rxBuff++ = pI2C->DAT;
		xfer->rxSz--;

	case 0x40:		/* SLA+R sent and ACK received */
		if (xfer->rxSz > 1) {
			cclr &= ~I2C_CON_AA;
		}
		break;

	/* NAK Handling */
	case 0x20:		/* SLA+W sent NAK received */
	case 0x48:		/* SLA+R sent NAK received */
		xfer->status = I2C_STATUS_SLAVENAK;
		cclr &= ~I2C_CON_STO;
		break;

	case 0x30:		/* DATA sent NAK received */
		xfer->status = I2C_STATUS_NAK;
		cclr &= ~I2C_CON_STO;
		break;

	case 0x38:		/* Arbitration lost */
		xfer->status = I2C_STATUS_ARBLOST;
		break;

	/* Bus Error */
	case 0x00:
		xfer->status = I2C_STATUS_BUSERR;
		cclr &= ~I2C_CON_STO;
	}

	/* Set clear control flags */
	pI2C->CONSET = cclr ^ I2C_CON_FLAGS;
	pI2C->CONCLR = cclr & ~I2C_CON_STO; 

	/* If stopped return 0 */
    /*Primero se fija que haya una condicion de stop, si se da entonces
    se fija si el estado sigue ocuapo y lo pone en ready para que otro lo use
    para ambos devuelve 0*/
	if (!(cclr & I2C_CON_STO) || (xfer->status == I2C_STATUS_ARBLOST)) {
		if (xfer->status == I2C_STATUS_BUSY) {
			xfer->status = I2C_STATUS_DONE;
		}
		return 0;
	}
	return 1;
}

/* Check if master state is active */
int Chip_I2C_IsMasterActive(I2C_ID_T id)
{
	return isMasterState(i2c[id].ip);
}
/* State change handler for master transfer */
void Chip_I2C_MasterStateHandler(I2C_ID_T id)
{   /* si un stop fue enviado entonces entra en el if y 
        se fija si el canal este libre*/
	if (!handleMasterXferState(i2c[id].ip, i2c[id].mXfer)) {
		i2c[id].mEvent(id, I2C_EVENT_DONE);
	}
}

/*Para las funciones del slave*/
/**
 * @brief	I2C Slave Identifiers
 */
typedef enum {
	I2C_SLAVE_GENERAL,	/**< Slave ID for general calls */
	I2C_SLAVE_0,		/**< Slave ID fo Slave Address 0 */
	I2C_SLAVE_1,		/**< Slave ID fo Slave Address 1 */
	I2C_SLAVE_2,		/**< Slave ID fo Slave Address 2 */
	I2C_SLAVE_3,		/**< Slave ID fo Slave Address 3 */
	I2C_SLAVE_NUM_INTERFACE	/**< Number of slave interfaces */
} I2C_SLAVE_ID;

/* Find the slave address of SLA+W or SLA+R */
I2C_SLAVE_ID getSlaveIndex(LPC_I2C_T *pI2C)
{
	switch (getCurState(pI2C)) {
	case 0x60:
	case 0x68:
	case 0x70:
	case 0x78:
	case 0xA8:
	case 0xB0:
		return lookupSlaveIndex(pI2C, pI2C->DAT);
	}
    /* If everything is fine code should never come here */
	return I2C_SLAVE_GENERAL;
}

/* Match the slave address */
STATIC int isSlaveAddrMatching(uint8_t addr1, uint8_t addr2, uint8_t mask)
{
	mask |= 1;
	return (addr1 & ~mask) == (addr2 & ~mask);
}
/* Get the index of the active slave */
STATIC I2C_SLAVE_ID lookupSlaveIndex(LPC_I2C_T *pI2C, uint8_t slaveAddr)
{
	if (!(slaveAddr >> 1)) {
		return I2C_SLAVE_GENERAL;					/* General call address */
	}
	if (isSlaveAddrMatching(pI2C->ADR0, slaveAddr, pI2C->MASK[0])) {
		return I2C_SLAVE_0;
	}
	if (isSlaveAddrMatching(pI2C->ADR1, slaveAddr, pI2C->MASK[1])) {
		return I2C_SLAVE_1;
	}
	if (isSlaveAddrMatching(pI2C->ADR2, slaveAddr, pI2C->MASK[2])) {
		return I2C_SLAVE_2;
	}
	if (isSlaveAddrMatching(pI2C->ADR3, slaveAddr, pI2C->MASK[3])) {
		return I2C_SLAVE_3;
	}

	/* If everything is fine the code should never come here */
	return I2C_SLAVE_GENERAL;
}
/* I2C Slave event handler */
void Chip_I2C_SlaveStateHandler(I2C_ID_T id)
{
	int ret;
	struct i2c_interface *iic = &i2c[id];

	/* Get the currently addressed slave */
	if (!iic->sXfer) {
		struct i2c_slave_interface *si2c;

		I2C_SLAVE_ID sid = getSlaveIndex(iic->ip);
		si2c = &i2c_slave[id][sid];
		iic->sXfer = si2c->xfer;
		iic->sEvent = si2c->event;
	}

	iic->sXfer->slaveAddr |= iic->mXfer != 0;
	ret = handleSlaveXferState(iic->ip, iic->sXfer);
	if (ret) {
		if (iic->sXfer->status == I2C_STATUS_DONE) {
			iic->sXfer = 0;
		}
		iic->sEvent(id, (I2C_EVENT_T) ret);
	}
}

/***********************************************************************/
/* State machine handler for I2C0 and I2C1 */
/*Que hace esta funcion? 
-recibe un parametro de tipo I2C_ID_T que indica que interfaz se utiliza
-se fija si el modo master esta activo, de estarlo se fija si el bus esta libre.
-si no esta en modo master esta en modo slave.*/
static void i2c_state_handling(I2C_ID_T id)
{
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	}
	else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

