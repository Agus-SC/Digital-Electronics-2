/***********************************************************************
 *                                                                     *
 * Ejemplo sobre el uso de la interfaz I2C (Master) con interrupcion.  *
 * Se utiliza como esclavo el RTC DS1307                               *
 *                                                                     *
 * Fecha: 26/04/2016                                                   *
 *                                                                     *
 *                                                                     *
 ***********************************************************************/

#include "board.h"
#include "i2cm_interrupt.h"

#define SPEED_100KHZ		(100000)

#define BOARD_NXP_LPCXPRESSO_4337
#define I2C_ADDR_7BIT		(0x68)
#define I2C_REG_ADDR_7BIT	(0x00)

#define HORA		0x21
#define MINUTOS		0x15
#define SEGUNDOS	0x48

static I2CM_XFER_T  i2cmXferRec;

static char aux[256]; // utilizada para imprimir por pantalla

/* Inicializacion del bus I2C */
static void i2c_app_init(I2C_ID_T id, int speed)
{
	Board_I2C_Init(id);

	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);

	/* Verificacion de la interfaz */
	if (id == I2C0) {
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else {
		NVIC_EnableIRQ(I2C1_IRQn);
	}
}

/* Configuracion y ejecucion de una solicitud de transferencia I2C */
static void SetupXferRecAndExecute(uint8_t devAddr,
								   uint8_t *txBuffPtr,
								   uint16_t txSize,
								   uint8_t *rxBuffPtr,
								   uint16_t rxSize){
								   
	/* Configuracion del registro de transferencia del I2C */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.options = 0;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;
	/*Transfiere los datos*/
	Chip_I2CM_Xfer(LPC_I2C0, &i2cmXferRec);

	/* Espera hasta que se complete la transferencia */
	while (i2cmXferRec.status == I2CM_STATUS_BUSY) {
		__WFI(); 	// Dormir el procesador hasta una interrupcion
					// Esta es una "hint instruction"
	}
	
}

/* Escritura I2C */
static void Establecer_Hora(uint8_t hh, uint8_t mm, uint8_t ss){
	uint8_t tx_buffer[4];

	// Configuracion de la hora
	tx_buffer[0] = 0x00;	// Direccion del primer registro (segundos)
	tx_buffer[1] = ss;		// Valor de segundos
	tx_buffer[2] = mm;		// Valor de minutos
	tx_buffer[3] = hh; 		// Valor de hora
	SetupXferRecAndExecute(I2C_ADDR_7BIT, tx_buffer, 4, NULL, 0);

}

/* Leer I2C */
static void Leer_Hora(){

	uint8_t tx_buffer[1];
	uint8_t rx_buffer[3];

	tx_buffer[0] = I2C_REG_ADDR_7BIT; // Direccion del primer registro (segundos)
	rx_buffer[0] = 0;
	rx_buffer[1] = 0;
	rx_buffer[2] = 0;
	SetupXferRecAndExecute(I2C_ADDR_7BIT, tx_buffer, 1, rx_buffer, 3);

	// Instrucciones para poder imprimir por consola la hora formateada
	sprintf_mio(aux, "Hora: %02X:%02X:%02X\r\n", rx_buffer[2], rx_buffer[1], rx_buffer[0]);
	DEBUGSTR(aux);

}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void UART_Init(void){

   //Initialize peripheral
   Chip_UART_Init(CIAA_BOARD_UART);
	
   // Set Baud rate
   Chip_UART_SetBaud(CIAA_BOARD_UART, SYSTEM_BAUD_RATE);

   //Modify FCR (FIFO Control Register)
   Chip_UART_SetupFIFOS(CIAA_BOARD_UART, UART_FCR_FIFO_EN | UART_FCR_TRG_LEV0);

   // Enable UART Transmission
   Chip_UART_TXEnable(CIAA_BOARD_UART);

   Chip_SCU_PinMux(7, 1, MD_PDN, FUNC6);              /* P7_1: UART2_TXD */
   Chip_SCU_PinMux(7, 2, MD_PLN|MD_EZI|MD_ZI, FUNC6); /* P7_2: UART2_RXD */

   //Enable UART Rx Interrupt
   Chip_UART_IntEnable(CIAA_BOARD_UART,UART_IER_RBRINT );   //Receiver Buffer Register Interrupt
   
   // Enable UART line status interrupt
   //Chip_UART_IntEnable(CIAA_BOARD_UART,UART_IER_RLSINT ); //LPC43xx User manual page 1118
   NVIC_SetPriority(USART2_IRQn, 6);
   
   // Enable Interrupt for UART channel
   NVIC_EnableIRQ(USART2_IRQn);
}

void I2C0_IRQHandler(void)
{
	/* Call I2CM ISR function with the I2C device and transfer rec */
	Chip_I2CM_XferHandler(LPC_I2C0, &i2cmXferRec);
}

int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	UART_Init();
	i2c_app_init(I2C0, SPEED_100KHZ);
	
	sprintf_mio(aux, "\r\n\r\n%s\r\n", "************ Inicio de adquisicion ************");
	DEBUGSTR(aux);

	Establecer_Hora(HORA, MINUTOS, SEGUNDOS);

	/* Loop forever */
	while (1) {
		Leer_Hora();
	}
}
