// #include <i2c_funciona.h>
// #include <test_leds.h>
// #include <test_tecs.h>
// #include <test_int.h>
#include <i2c.h>
#include <LED.h>
#include <TEC.h>
#include <INT.h>

/*
Trabajamos con 3.3V y resistencias de pull-up de 2k2.
Funcionan tanto MasterTX como Master RX.
*/

TRxFER	M_xFER ;
TRxFER	S_xFER ;

/* Definimos el address como 0x06 */
#define SLAVE_ADD	0x06
/*  Definimos la unidad minima de transmicion */
#define BYTE	1
/*  Definimos el buffer donde se guardan los datos */
uint8_t MRx_BUFF = 0 ;
uint8_t SRx_BUFF = 0 ;

/* Definimos el estado del led a transmitir */
uint8_t led_state = 0 ;

typedef enum{
	LEDR,
	LEDG,
	LEDB,
	LED1,
	LED2,
	LED3,
} LEDS ;

/* Mascaras para saber que led esta prendido */
#define MASK_LEDB	(0x1 << 2)
#define MASK_LEDG	(0x1 << 1)
#define MASK_LEDR	(0x1 << 0)
#define MASK_LED1	(0x1 << 14)
#define MASK_LED2	(0x1 << 11)
#define MASK_LED3	(0x1 << 12)

typedef enum{
	TEC_NULL,
	TEC1,
	TEC2,
	TEC3,
	TEC4,
} TECS ;

bool init_I2C = false ;

void GPIO0_IRQHandler(){
	clear_IST(TEC1) ;
}

/* Con la tecla 2 habilitamos modo master transmitter y transmitimos el estado del LED 3 */
void GPIO1_IRQHandler(){

	/* Seteamos el bit de modo transmision */
	M_xFER.DIR = WRITE ;
    S_xFER.DIR = READ ;
	/* Seteamos el slave address */
	M_xFER.SLA = SLAVE_ADD ;
    S_xFER.SLA = SLAVE_ADD ;
	/* Seteamos el tamaño a un byte */
	M_xFER.SIZE_Tx = BYTE ;
    S_xFER.SIZE_Rx = BYTE ;
	/* Seteamos el dato a transmitir, el cual es el estado del LED 3*/
	led_state = (*SET1 & MASK_LED3) >> 12 ;
	M_xFER.DATA_Tx = &led_state ;
    S_xFER.DATA_Rx = &SRx_BUFF ;

	/* Espero a que el bus este disponible */
	init_SLAVE(I2C0, &S_xFER) ;
    set_I2EN(I2C1) ;
    send_START(I2C1) ;
    clear_IST(TEC2) ;
}

/* Con la tecla 3 habilitamos el modo master receiver y recibimos el estado
del LED3. En funcion de su valor encendemos o no el Led Blue */
void GPIO2_IRQHandler(){

	/* Seteamos el bit de modo transmision */
	M_xFER.DIR = READ ;
    S_xFER.DIR = WRITE ;
	/* Seteamos el slave address */
	M_xFER.SLA = SLAVE_ADD ;
    S_xFER.SLA = SLAVE_ADD ;
	/* Seteamos el tamaño a un byte */
	M_xFER.SIZE_Rx = BYTE ;
    S_xFER.SIZE_Tx = BYTE ;
	/* Seteamos el dato a transmitir, el cual es el estado del LED 3*/
	led_state = (*SET1 & MASK_LED3) >> 12 ;
	S_xFER.DATA_Tx = &led_state ;
    M_xFER.DATA_Rx = &MRx_BUFF ;

	/* Espero a que el bus este disponible */
    // while(is_bus_busy(I2C0)){}
	init_SLAVE(I2C0, &S_xFER) ;
    set_I2EN(I2C1) ;
    send_START(I2C1) ;
    clear_IST(TEC3) ;

}

bool aux_led = false ;
void GPIO3_IRQHandler(){

	if (!aux_led){
		LEDs_set(LED3) ;
	}
	else{
		LEDs_clr(LED3) ;
	}
	aux_led = !aux_led ;
	clear_IST(TEC4) ;
}

uint8_t M_status ;
uint8_t S_status ;

void I2C1_IRQHandler(void)
{
	if(M_xFER.DIR == WRITE ){
        M_status = Tx_MASTER(I2C1, &M_xFER) ;
//        switch(M_status){
//            case I2C_STATUS_DONE:
//            /* transmision exitosa */
//            break ;
//            case I2C_STATUS_OK:
//            /* transmision en proceso */
//            break ;
//            case I2C_STATUS_SLAVENAK:
//			LEDs_set(LEDB) ;
//			LEDs_clr(LEDG) ;
//			LEDs_clr(LEDR) ;
//            break ;
//            case I2C_STATUS_NAK:
//			/* Error en la transmision */
//			LEDs_set(LEDG) ;
//			LEDs_clr(LEDR) ;
//			LEDs_clr(LEDB) ;
//			break ;
//            case I2C_STATUS_BUSERR:
//            /* Error en la transmision */
//            LEDs_set(LEDR) ;
//            LEDs_clr(LEDG) ;
//            LEDs_clr(LEDB) ;
//            break ;
//            case I2C_STATUS_ARBLOST:
//            /* Arbitration has been lost */
//            LEDs_set(LEDB) ;
//            LEDs_set(LEDR) ;
//            LEDs_set(LEDG) ;
//            break ;
//        }
    }
    else{
        M_status = Rx_MASTER(I2C1, &M_xFER) ;
        int i = 0;
        for (i=0;i<1000;i++);
//        switch(M_status){
//            case I2C_STATUS_DONE:
//            /* transmision exitosa */
//            break ;
//            case I2C_STATUS_OK:
//            /* transmision en proceso */
//            break ;
//            case I2C_STATUS_SLAVENAK:
//            case I2C_STATUS_NAK:
//            case I2C_STATUS_BUSERR:
//            /* Error en la transmision */
//            LEDs_set(LEDR) ;
//            break ;
//            case I2C_STATUS_ARBLOST:
//            /* Arbitration has been lost */
//            LEDs_set(LEDB) ;
//            break ;
//        }
    }
}

void I2C0_IRQHandler(void)
{
	if(S_xFER.DIR == READ ){
        S_status = Rx_SLAVE(I2C0, &S_xFER) ;
    }
    else{
        S_status = Tx_SLAVE(I2C0, &S_xFER) ;
        int i = 0;
        for (i=0;i<1000;i++);
    }
}

void main(void)
{
	LEDs_init() ;
	TECs_init() ;

	init_TEC_interrupt(TEC1) ;
	init_TEC_interrupt(TEC2) ;
	init_TEC_interrupt(TEC3) ;
	init_TEC_interrupt(TEC4) ;

	I2C_PIN_init(I2C0_ID) ;
	I2C_CLK_init(I2C0) ;
	init_I2C_interrupt(I2C0_ID) ;
	I2C_PIN_init(I2C1_ID) ;
	I2C_CLK_init(I2C1) ;
	init_I2C_interrupt(I2C1_ID) ;

	while(1){
		if (SRx_BUFF){
			LEDs_set(LED1) ;
		}else{
			LEDs_clr(LED1) ;
		}
		if (MRx_BUFF){
			LEDs_set(LED2) ;
		}else{
			LEDs_clr(LED2) ;
		}
	}
}
