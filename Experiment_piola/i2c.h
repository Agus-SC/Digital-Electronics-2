/* Definicion de los tipos de datos */
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

typedef _Bool bool;
#define	false	0
#define	true	1

/* Definicion de la direccion base del SCU */
#define SCU_BASE        0x40086000

/* La Interfaz 0 de I2C tiene su propio registro SFSI2C0 */
/* Direccion registro SFSI2C0 */
#define SFSI2C0_OFFSET        0xC84

/* La linea SDA de la Interfaz 1 de I2C esta en el pin P2_3 FUNC1 o PE_13 FUNC2 */
/* Offset P2_3 para SDA I2C1 */
#define SFSP2_3_OFFSET     0x10C

/* La linea SCL de la Interfaz 1 de I2C esta en el pin P2_4 FUNC1 o PE_13 FUNC2 */
/* Offset P2_4 para SCL I2C1 */
#define SFSP2_4_OFFSET     0x110

/* Definicion de los punteros a los registros del SCU de los pines asociados a I2C*/
uint32_t *SFSI2C0 = (uint32_t *)(SCU_BASE + SFSI2C0_OFFSET) ;
uint32_t *SFSP2_3 = (uint32_t *)(SCU_BASE + SFSP2_3_OFFSET) ;
uint32_t *SFSP2_4 = (uint32_t *)(SCU_BASE + SFSP2_4_OFFSET) ;

/* Definicion de los identificadores para las interfaces
I2C0 e I2C1 */
typedef enum{
    I2C0_ID,
    I2C1_ID,
} ID_T ;

/* Deficion de los punteros a los registros base de las
interfaces I2C0 e I2C1 del tipo I2C STRUCT */
#define I2C0        ((I2C_T  *)  0x400A1000)
#define I2C1        ((I2C_T  *)  0x400E0000)

typedef struct {
	uint32_t CONSET ;
	uint32_t STAT ;
	uint32_t DAT ;
	uint32_t ADR0 ;
	uint32_t SCLH ;
	uint32_t SCLL ;
	uint32_t CONCLR ;
	uint32_t MMCTRL ;
	uint32_t ADR1 ;
    uint32_t ADR2 ;
    uint32_t ADR3 ;
	uint32_t DATA_BUFFER ;
	uint32_t MASK[4] ;

} I2C_T ;


/* Definicion de los Macros para la configuracion de los bit
del registro CONSET */
#define CONSET_AA           1 << 2
#define CONSET_SI           1 << 3
#define CONSET_STO          1 << 4
#define CONSET_STA          1 << 5
#define CONSET_I2EN         1 << 6

/* Definicion de los Macros para la configuracion de los bit
del registro CONCLR */
#define CONCLR_AA           1 << 2
#define CONCLR_SI           1 << 3
//Cuando un STO es detectado se limpia automaticamente
#define CONCLR_STA          1 << 5
#define CONCLR_I2EN         1 << 6

/* Definicion de la mascara para leer el registro STAT */
#define MASK_STAT           0xF8 // 11111000

/* Definicion de la mascara para leer el bit STO de CONSET */
#define MASK_STO           0x20 // 00100000

/* Definicion de la mascara para leer/escribir el registro DAT */
#define MASK_DAT            0xFF // 11111111

/* Definicion de los macros para leer o escribir */
#define READ                1
#define WRITE               0

/* Clock Source: 12MHz */
#define CLOCK_BASE      12000000

/* Velocidad de transmicion I2C 100kHz */
#define SPEED       100000

/* Definicion de una estructura para la transmicion y recepcion de datos */
typedef struct {
    uint8_t SLA ;           /* Slave address 1 byte */
    uint8_t *DATA_Tx ;      /* Datos de transmicion 1 Byte a la vez */
    uint8_t *RST_DATA_Tx ;  /* Posicion inicial de los datos */
    uint32_t SIZE_Tx ;      /* Tama;o de los datos a transmitir */
    uint32_t RST_SIZE_Tx ;  /* Tama;o inicial de los datos a transmitir */
    uint8_t *DATA_Rx ;      /* Datos de recepion 1 Byte a la vez */
    uint8_t *RST_DATA_Rx ;  /* Posicion inicial de los datos */
    uint32_t SIZE_Rx ;      /* Tama;o de los datos a recibir */
    uint32_t RST_SIZE_Rx ;  /* Tama;o inicial de los datos a recibir */
    uint8_t DIR ;           /* Bit de direccion 0 para escribir 1 para leer */

} TRxFER ;

/* Definicion de estados de control */
typedef enum {
	I2C_STATUS_BUSERR,	  /* Error en el bus */
    I2C_STATUS_SLAVENAK,  /* NAK recibido despues de SLA+W o SLA+R */
    I2C_STATUS_NAK,		  /* NAK recibido durante la transferencia */
    I2C_STATUS_ARBLOST,	  /* Aribitration lost during transfer */
    I2C_STATUS_DONE ,	  /* Transferencia de excitosa */
	I2C_STATUS_OK,        /* Transferencia en transito */

} I2C_STATUS_T;

/* Funcion para la configuracion electrica y modos de funcionamiento
de los pines pertenecientes a las interfaces I2C0 e I2C1
Input: numero de interfaz 0 o 1.
Output: configura los registros del SCU para operar en modo standard en
ambos casos
*/
void I2C_PIN_init(uint8_t ITF){
    if (!ITF){
        /* configuracion interfaz 0,
        Habilitamos input receiver SCL
        Habilitamos input glitch filter del pin SCL (trabajamos con STANDARD MODE)
        Habilitamos input receiver SDA
        Habilitamos input glitch filter del pin SDA (trabajamos con STANDARD MODE) */
        //          SDA_EZI  |   SCL_EZI
        *SFSI2C0 = (0x1 << 11) | (0x1 << 8) | (0x1 << 3) | (0x1 << 0) ;
    }
    else{
        /*configuracion interfaz 1
        Configuramos la funcion 1
        Habilitamos pull up: 0 en bit EPUN
        Habilitamos el input buffer
        Habilitamos el input filter glitch
        */
        *SFSP2_3 = (0x1 << 7) | (0x1 << 6) | (0x1 << 0) ;
        *SFSP2_4 = (0x1 << 7) | (0x1 << 6) | (0x1 << 0) ;
    }
}

/* Funcion que configura el duty cycle al 50% lo que corresponde a cargar
un valor en los registros SCLH Y SCLL de 120.
*/
void I2C_CLK_init(I2C_T *pI2C){
    uint32_t SCLL_SCLH = CLOCK_BASE / SPEED ;
    pI2C -> SCLH = SCLL_SCLH / 2 ;
	pI2C -> SCLL = SCLL_SCLH / 2 ;
	pI2C -> CONCLR = CONCLR_I2EN | CONCLR_STA | CONCLR_SI | CONCLR_AA ;
}

/* Funcion que habilita la interfaz I2C */
void set_I2EN(I2C_T *pI2C){
    pI2C -> CONCLR = CONCLR_I2EN | CONCLR_STA | CONCLR_SI | CONCLR_AA ;
    pI2C -> CONSET = CONSET_I2EN ;
}

/* Funcion que envia un START condition */
void send_START(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_STA ;
}

/* Funcion que envia un STOP condition */
void send_STOP(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_STO ;
}

/* Funcion que limpia el bit SI del registro CONSET */
void clear_SI(I2C_T *pI2C){
	int i ;
    pI2C -> CONCLR = CONCLR_SI ;
}

/* Funcion que retorna el estado de la comunicacion */
uint32_t get_STATUS(I2C_T *pI2C){
    return (uint32_t) pI2C -> STAT & MASK_STAT ;
}

void delay(void){
	int i;
	for(i = 0; i < 1000; i++);
}



/*************** Funciones para modo master ***************/

/* Funcion para transmitir */
uint32_t Tx_MASTER(I2C_T *pI2C, TRxFER *MSG){
    switch(get_STATUS(pI2C)){
        case 0x00:
        /* Bus Error. Enter not addressed Slave mode and release bus */
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
        clear_SI(pI2C) ; // clear SI flag
        return I2C_STATUS_BUSERR ;

        case 0x08:
        /* A START condition has been transmitted. The Slave Address
        + R/W bit will be transmitted, an ACK bit will be received. */
        pI2C -> DAT = ( MSG -> SLA << 1 ) | WRITE ; // write Slave address with W bit to DAT
//        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        pI2C -> CONCLR = CONCLR_AA | CONCLR_STA | CONCLR_SI; // Clear the AA bit
//        clear_SI(pI2C) ; // clear SI flag
        // Los buffers los cargamos en previamente
        break ;

        case 0x10:
        /*A Repeated START condition has been transmitted. The Slave
        Address + R/W bit will be transmitted, an ACK bit will be received.*/
        pI2C -> DAT = ( MSG -> SLA << 1 ) | WRITE ; // write Slave address with W bit to DAT
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        pI2C -> CONCLR = CONCLR_STA ; // Clear the AA bit
        clear_SI(pI2C) ; // clear SI flag

        // Los buffers los cargamos en previamente
        break ;

        /* Transmitter Mode */
        case 0x18:
        /* Previous state was State 8 or State 10, Slave Address + Write
        has been transmitted, ACK has been received. The first data byte
        will be transmitted, an ACK bit will be received. */
        case 0x28:
        /* Data has been transmitted, ACK has been received. If the transmitted data was the last
        data byte then transmit a STOP condition, otherwise transmit the next data byte. */
        if(!(MSG -> SIZE_Tx)){
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
            clear_SI(pI2C) ; // clear SI flag
            return I2C_STATUS_DONE ;
        }
        pI2C -> DAT = ( *MSG -> DATA_Tx++) ;
        MSG -> SIZE_Tx-- ;
        pI2C -> CONCLR = CONCLR_AA ;
        clear_SI(pI2C) ; // clear SI flag
        break ;

        case 0x20:
        /* Slave Address + Write has been transmitted, NOT ACK has been
        received. A STOP condition will be transmitted. */
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
        clear_SI(pI2C) ; // clear SI flag
        return I2C_STATUS_SLAVENAK ;

        case 0x30:
        /* Data has been transmitted, NOT ACK received. A STOP condition will
        be transmitted. */
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
        clear_SI(pI2C) ; // clear SI flag
        return I2C_STATUS_NAK ;

        case 0x38:
        /* Arbitration has been lost during Slave Address + Write or data.
        The bus has been released and not addressed Slave mode is entered.
        A new START condition will be transmitted when the bus is free again. */
        pI2C -> CONSET = CONSET_STA | CONSET_AA ; // set the STA and AA bits
        clear_SI(pI2C) ; // clear SI flag
        return I2C_STATUS_ARBLOST ;
    }
    return I2C_STATUS_OK ;
}

/* Funcion para recibir */
uint32_t Rx_MASTER(I2C_T *pI2C, TRxFER *MSG){
	int i = 0;
    switch(get_STATUS(pI2C)){
        case 0x00:
        /* Bus Error. Enter not addressed Slave mode and release bus */
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
        clear_SI(pI2C) ; // clear SI flag
        return I2C_STATUS_BUSERR ;

        case 0x08:
        /* A START condition has been transmitted. The Slave Address
        + R/W bit will be transmitted, an ACK bit will be received. */
        pI2C -> DAT = ( MSG -> SLA << 1 ) | READ ; // write Slave address with W bit to DAT
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        pI2C -> CONCLR = CONCLR_STA  ; // clear STA
        clear_SI(pI2C) ; // clear SI flag
        // Los buffers los cargamos en previamente
        break ;

        case 0x10:
        /*A Repeated START condition has been transmitted. The Slave
        Address + R/W bit will be transmitted, an ACK bit will be received.*/
        pI2C -> DAT = ( MSG -> SLA << 1 ) | READ ; // write Slave address with W bit to DAT
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        clear_SI(pI2C) ; // clear SI flag
        // Los buffers los cargamos en previamente
        break ;

        /* Reciver Mode */
        case 0x40:
        /* Previous state was State 08 or State 10. Slave Address + Read has
        been transmitted, ACK has been received. Data will be received and ACK returned. */
		if (MSG -> SIZE_Rx == 1){
			pI2C -> CONCLR = CONCLR_AA  ; // clear AA
			clear_SI(pI2C) ; // clear SI
			break ;
		}
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        clear_SI(pI2C) ; // clear the SI flag
        break ;

        case 0x48:
        /* Slave Address + Read has been transmitted, NOT ACK has been received. A STOP
        condition will be transmitted. */
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA
        clear_SI(pI2C) ; // clear SI
        return I2C_STATUS_SLAVENAK ;

        case 0x50:
        /* Data has been received, ACK has been returned. Data will be read from DAT. Additional
        data will be received. If this is the last data byte then NOT ACK will be returned, otherwise
        ACK will be returned. */
        // Pointer moves to the next int position (as if it was an array). But returns the old content
        *MSG -> DATA_Rx = ( pI2C -> DAT ) & MASK_DAT ;
        if (MSG -> SIZE_Rx == 1){
            pI2C -> CONCLR = CONCLR_AA  ; // clear AA
            clear_SI(pI2C) ; // clear SI
            break ;
        }
        MSG -> DATA_Rx++ ;
        MSG -> SIZE_Rx-- ;
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        clear_SI(pI2C) ; // clear the SI flag
        break ;

        case 0x58:
        /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
        A STOP condition will be transmitted. */
        *MSG -> DATA_Rx = ( pI2C -> DAT ) & MASK_DAT ;
        MSG -> SIZE_Rx-- ;
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set the AA bit
        clear_SI(pI2C) ; // clear the SI flag
        return I2C_STATUS_NAK ;
    }
    return I2C_STATUS_OK ;
}

/*************** Funciones para modo slave ***************/
void init_SLAVE(I2C_T *pI2C, TRxFER *MSG){
    int i ;
    pI2C -> ADR0 = MSG -> SLA << 1 ;
    /* duplicamos */
    pI2C -> ADR1 = MSG -> SLA << 1 ;
    pI2C -> ADR2 = MSG -> SLA << 1 ;
    pI2C -> ADR3 = MSG -> SLA << 1 ;
    /* Seteamos la mascara */
    pI2C -> MASK[0] = (MSG -> SLA << 1) & 0x00 ;
    for(i = 1 ; i < 4 ; i++){
        pI2C -> MASK[i] = 0x00 & 0x00 ;
    }
    /* Habilitamos la interfaz y el AA para slave mode */
    pI2C -> CONCLR = CONCLR_STA | CONCLR_SI ;
    pI2C -> CONSET = CONSET_I2EN | CONSET_AA ;
}

/* Funcion para recibir */
uint32_t Rx_SLAVE(I2C_T *pI2C, TRxFER *MSG){
    switch(get_STATUS(pI2C)){
        /* Reciver states */
    	case 0x00:
    		clear_SI(pI2C) ;
    		break ;
        case 0x60:
            /* Own Slave Address + Write has been received, ACK has been returned. Data will be
            received and ACK returned. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0x68:
            /* Arbitration has been lost in Slave Address and R/W bit as bus Master. Own Slave Address
            + Write has been received, ACK has been returned. Data will be received and ACK will be
            returned. STA is set to restart Master mode after the bus is free again. */
        case 0x78:
            /* Arbitration has been lost in Slave Address + R/W bit as bus Master. General call has been
            received and ACK has been returned. Data will be received and ACK returned. STA is set
            to restart Master mode after the bus is free again. */
            pI2C -> CONSET = CONSET_STA | CONSET_AA ;  // set STA and the AA bit
            clear_SI(pI2C) ; // clear SI flag
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;

        case 0x70:
            /*General call has been received, ACK has been returned. Data will be received and ACK
            returned.*/
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            clear_SI(pI2C) ; // clear SI flag
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;

        case 0x80:
            /* Previously addressed with own Slave Address. Data has been received and ACK has
            been returned. Additional data will be read. */
            *MSG -> DATA_Rx++ = ( pI2C -> DAT ) & MASK_DAT ;
            if (MSG -> SIZE_Rx == 1){
                MSG -> SIZE_Rx-- ;
                pI2C -> CONCLR = CONCLR_AA  ; // clear AA
                clear_SI(pI2C) ; // clear SI flag
                return I2C_STATUS_DONE ;
            }
            MSG -> SIZE_Rx-- ;
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0x88:
            /* Previously addressed with own Slave Address. Data has been received and NOT ACK
            has been returned. Received data will not be saved. Not addressed Slave mode is
            entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0x90:
            /* Previously addressed with General Call. Data has been received, ACK has been returned.
            Received data will be saved. Only the first data byte will be received with ACK. Additional
            data will be received with NOT ACK. */
            *MSG -> DATA_Rx = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONCLR = CONCLR_AA  ; // clear AA
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0x98:
            /* Previously addressed with General Call. Data has been received, NOT ACK has been
            returned. Received data will not be saved. Not addressed Slave mode is entered. */
        case 0xA0:
            /* A STOP condition or Repeated START has been received, while still addressed as a
            Slave. Data will not be saved. Not addressed Slave mode is entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;
    }
   return I2C_STATUS_OK ;
}

/* Funcion para transmitir */
uint32_t Tx_SLAVE(I2C_T *pI2C, TRxFER *MSG){
    switch(get_STATUS(pI2C)){
        /* Transmitter states */
    	case 0x00:
    		clear_SI(pI2C) ; // clear SI flag
    		break ;
        case 0xA8:
            /* Own Slave Address + Read has been received, ACK has been returned. Data will be
            transmitted, ACK bit will be received. */
        case 0xB8:
            /* Data has been transmitted, ACK has been received. Data will be transmitted, ACK bit will
            be received. */
            pI2C -> DAT = ( *MSG -> DATA_Tx++ ) ;
            MSG -> SIZE_Tx-- ;
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0xB0:
            /* Arbitration lost in Slave Address and R/W bit as bus Master. Own Slave Address + Read
            has been received, ACK has been returned. Data will be transmitted, ACK bit will be
            received. STA is set to restart Master mode after the bus is free again. */
            pI2C -> DAT = ( *MSG -> DATA_Tx++ ) ;
            MSG -> SIZE_Tx-- ;
            pI2C -> CONSET = CONSET_STA | CONSET_AA ;  // set STA and the AA bit
            clear_SI(pI2C) ; // clear SI flag
            break ;

        case 0xC0:
            /*Data has been transmitted, NOT ACK has been received. Not addressed Slave mode is
            entered.*/
        case 0xC8:
            /* The last data byte has been transmitted, ACK has been received. Not addressed Slave
            mode is entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            clear_SI(pI2C) ; // clear SI flag
            break ;
    }
    return I2C_STATUS_OK ;
}
