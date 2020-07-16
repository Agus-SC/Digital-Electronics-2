/*
Master configuration
 */
#include <I2C1.h>
#include <LED.h>
#include <TEC.h>
#include <INT.h>


typedef enum{
	I2C0_ID,
	I2C1_ID,
} interface ;

typedef enum{
	TEC_NULL,
	TEC1,
	TEC2,
	TEC3,
	TEC4,
} TECS ;

typedef enum{
	LEDB,
	LEDG,
	LEDR,
	LED1,
	LED2,
	LED3,
} LEDS ;

static MSG_T	TX_MSG ;
static MSG_T	RX_MSG ;

bool init_I2C = false ;

/* Mascaras para saber que led esta prendido en el slave */
#define MASK_LEDB	(0x1 << 2)
#define MASK_LEDG	(0x1 << 1)
#define MASK_LEDR	(0x1 << 0)
#define MASK_LED1	(0x1 << 14)
#define MASK_LED2	(0x1 << 11)
#define MASK_LED3	(0x1 << 12)

/* Dato que vamos a transmitir y su tamaño */
/* Definimos el address como 0x06 */
#define SLA	0x06
/*  Definimos el tamaño de transmicion */
#define BYTE	1 
/* Definimos el buffer para recibir datos */
uint8_t BUFF ;

uint8_t led_state = 0 ;

/* Con la tecla 1 inicializo la interfaz I2C. Se enciende el led en blanco */
void GPIO0_IRQHandler(){
	if(!init_I2C){
	I2C_CLK_init(I2C0, I2C0_ID) ;
	I2C_init(I2C0_ID) ;
	init_I2C_interrupt(I2C0_ID) ;
	I2C_CLK_init(I2C1, I2C1_ID) ;
	I2C_init(I2C1_ID) ;
	init_I2C_interrupt(I2C1_ID) ;
	LEDs_set(LEDB) ;
	LEDs_set(LEDG) ;
	LEDs_set(LEDR) ;
	}
}

/* Con la tecla 2 habilitamos modo master transmitter y transmitimos el estado del LED 3 */
void GPIO1_IRQHandler(){
	LEDs_clr(LEDB) ;
	LEDs_clr(LEDG) ;
	LEDs_clr(LEDR) ; 
	
	/* Seteamos el bit de modo transmision */
	TX_MSG.Tx = true ;
	RX_MSG.Tx = false ;
	/* Seteamos el slave address */
	TX_MSG.SLAVE_ADDRESS = SLA ;
	RX_MSG.SLAVE_ADDRESS = SLA ;
	/* Seteamos el tamaño a un byte */
	TX_MSG.SIZE = BYTE ;
	RX_MSG.SIZE = BYTE ;
	/* Seteamos el dato a transmitir, el cual es el estado del LED 3*/
	led_state = (*SET1 & MASK_LED3) >> 12 ;
	TX_MSG.DATA = led_state ;
	/* Guardamos la direccion inicial de DATA */
	TX_MSG.RESTART = TX_MSG.DATA ;

	set_SLAVEADDR(I2C1, SLA) ;
	init_SLAVE(I2C1, SLA) ;

    ENA_interface(I2C0) ;
    START_send(I2C0) ;

	switch(Polling_master(I2C0, TX_MSG)){
		case I2C_STATUS_DONE:
		/* La transmision fue exitosa */
		LEDs_set(LEDG) ;
		return ;
		case  I2C_STATUS_SLAVENAK:
		/* NAK recibido despues de enviar SLA+W*/
		LEDs_set(LED2) ;
		return ;
		case I2C_STATUS_NAK:
		/* NAK recibido despues de transmitir DATOS */
		LEDs_set(LED2) ;
		return ;
		case I2C_STATUS_ARBLOST:
		/*Arbitraje perdido */
		return ;
		case I2C_STATUS_BUSERR :
		/* Error en el bus */
		LEDs_set(LED2) ;
		return ;
	}
}

/* Con la tecla 3 habilitamos el modo master receiver y recibimos el estado
del LED3 de la placa slave. En funcion de su valor encendemos o no el Led Blue */
void GPIO2_IRQHandler(){
	LEDs_clr(LEDB) ;
	LEDs_clr(LEDG) ;
	LEDs_clr(LEDR) ; 
	/* Seteamos el bit de modo transmision */
	TX_MSG.Tx = false ;
	/* Seteamos el slave address */
	TX_MSG.SLAVE_ADDRESS = SLA ;
	/* Seteamos el tamaño a un byte */
	TX_MSG.SIZE = BYTE ;
	
	switch(Polling_master(I2C0, TX_MSG)){
		case I2C_STATUS_DONE:
		/* La transmision fue exitosa */
		if (TX_MSG.DATA){
			LEDs_set(LEDB) ;
		}
		else{
			LEDs_clr(LEDB) ;
		}
		return ;
		case  I2C_STATUS_SLAVENAK:
		/* NAK recibido despues de enviar SLA+W*/
		LEDs_set(LED2) ;
		return ;
		case I2C_STATUS_BUSERR :
		/*Error en el BUS */
		LEDs_set(LED2) ;
		return ;
	}
}

/* Con la tecla 4: Toggleo estado del led 3 */
bool aux_led = false ;
void GPIO3_IRQHandler(){
	LEDs_clr(LEDB) ;
	LEDs_clr(LEDG) ;
	LEDs_clr(LEDR) ; 
	if (aux_led){
		LEDs_set(LED3) ;
	}
	else{
		LEDs_clr(LED3) ;
	}
	aux_led = !aux_led ;
}

void I2C0_IRQHandler(void)
{
	
}

void I2C1_IRQHandler(void)
{
	
}

void main(void)
{
	LEDs_init() ;
	TECs_init() ;

	init_TEC_interrupt(TEC1) ;
	init_TEC_interrupt(TEC2) ;
	init_TEC_interrupt(TEC3) ;
	init_TEC_interrupt(TEC4) ;

	while(1){

	}
}