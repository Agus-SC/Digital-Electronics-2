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
#define SLAVE_ADD	0x06
/* Transmitimos un byte 0x01 para encender el led del slave */
#define DATA_8bit	0x01
#define SIZE	1 

void setup_msg(){
	TX_MSG.SLAVE_ADDRESS = SLAVE_ADD ;
	TX_MSG.DATA = DATA_8bit ;
	TX_MSG.RESTART = DATA_8bit ;
	TX_MSG.Tx = true ;
}

/* Con la tecla 1 inicializo la interfaz I2C. Se enciende el led en blanco */
void GPIO0_IRQHandler(){
	if(!init_I2C){
	I2C_CLK_init(I2C0, I2C0_ID) ;
	I2C_init(I2C0_ID) ;
	LEDs_set(LEDB) ;
	LEDs_set(LEDG) ;
	LEDs_set(LEDR) ;
	}
}

/* Con la tecla 2 transmitimos datos master -> slave, encendemos el LED2 */
void GPIO1_IRQHandler(){

}

/* Con la tecla 3 recibimos datos slava -> master, recibimos el estado
del LED3 el cual lo seteamos con la TEC3 pertenecientes a la placa slave */
void GPIO2_IRQHandler(){
	
}

/* Con la tecla 4  */
void GPIO3_IRQHandler(){
	
}



int main(void)
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