
/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

typedef bool _bool ;
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

/* Read and Write */
#define READ                0X01
#define WRITE               0X00

/* Mascara para leer el estado del registro status solo bit 3:7 */
#define MASK_STAT           0xF8 // 11111000

/* Mascara para leer el dato del registro DAT 0:7 */
#define MASK_DAT            0xFF // 11111111

/* Mascara para guardar el slave address en el campo de la estructura slave_add */
#define MASK_ADD            0xFE // 11111111

// /*Masks*/
// #define MASK_MASK(n) (n & 0xFE)

// Estructura para Master/Slave Mode.
/* uint16_t Porque quizas se necesita usar un buffer ya se para transmitir
o recibir de mas de 2^8 = 256 elementos */
typedef struct{
    uint8_t SLAVE_ADDR; /* Le paso el slave address */
    uint8_t OPTIONS;	/* Option, todavia no sabemos */
	uint8_t STATUS;		/* Estado de transmision */
	uint16_t Tx_SIZE;	/* Tama;o del buffer a transmitir (Numero de bytes), Transmito el Address si recivo */
	uint16_t Rx_SIZE;	/* Tama;o del buffer a recibir (Numero de bytes), 0 si transmito*/
	uint8_t *Tx_BUFFER; /* Puntero al buffer de datos a transmitir, NULL si recibo */
	uint32_t *Rx_BUFFER;
     
}MS_T;

// Estados 
typedef enum {
	I2C_STATUS_DONE,	/**< Transfer done successfully */
	I2C_STATUS_NAK,		/**< NAK received during transfer */
	I2C_STATUS_ARBLOST,	/**< Aribitration lost during transfer */
	I2C_STATUS_BUSERR,	/**< Bus error in I2C transfer */
	I2C_STATUS_BUSY,	/**< I2C is busy doing transfer */
	I2C_STATUS_SLAVENAK,/**< NAK received after SLA+W or SLA+R */

} I2C_STATUS_T;

// Funcion que inicializa el registro CONSET
void init_CONSET(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_I2EN ;
}

/* Funcion que envia el START condition*/
void send_START(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_I2EN | CONSET_STA ;
}

/* Funcion que lee el registro STATUS */
uint32_t get_STATUS(I2C_T *pI2C){
    return (uint32_t) pI2C -> STAT & MASK_STAT ;
} 

/* Maquina de estado para recibir y transmitir en Master Mode */
int ComMasterMode(I2C_T *pI2C, MS_T *Xstruct){

    switch(get_STATUS(pI2C)){
        
        case 0x00:
            /* Bus Error. Enter not addressed Slave mode and release bus */
            Xstruct -> STATUS = I2C_STATUS_BUSERR;
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            if (Xstruct -> STATUS == I2C_STATUS_BUSY ){
                Xstruct -> STATUS = I2C_STATUS_DONE ;
            }
            return 0 ;

        case 0x08:
            /* A START condition has been transmitted. The Slave Address
            + R/W bit will be transmitted, an ACK bit will be received. */
            pI2C -> DAT = ( Xstruct -> SLAVE_ADDR ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag 
            // Los buffers los cargamos en previamente 
            break ;

        case 0x10:
            /*A Repeated START condition has been transmitted. The Slave 
            Address + R/W bit will be transmitted, an ACK bit will be received.*/
            pI2C -> DAT = ( Xstruct -> SLAVE_ADDR ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag  
            // Los buffers los cargamos en previamente 
            break ;
            
        /* Transmitter Mode */
        case 0x18:
            /* Previous state was State 8 or State 10, Slave Address + Write 
            has been transmitted, ACK has been received. The first data byte 
            will be transmitted, an ACK bit will be received. */
            if (Xstruct -> Tx_SIZE != 0 ){
                // si no quiero transmitir y quiero recibir manda un START condition
                if (Xstruct -> Rx_SIZE){
                    // CON PINZAS
                    pI2C -> CONSET = 1 << 5 ; // set STA
                    pI2C -> CONCLR = 1 << 2 | 1 << 3 ; // clear AA and SI 
                    break ;
                }
                // si no quiero transmitir y no quiero recibir mando un STOP condition
                else{
                    pI2C -> CONSET = 0x14 ; // set STO and AA
                    pI2C -> CONCLR = 0x08 ; // clear SI 
                    break ;
                }           
            }
            else{
                pI2C -> DAT = ( *Xstruct -> Tx_BUFFER++ ) ;
                Xstruct -> Tx_SIZE-- ;  
                pI2C -> CONSET = 0x04 ; // set the AA bit
                pI2C -> CONCLR = 0x08 ; // clear SI flag
                break ;
            }
            
        case 0x20:
            /* Slave Address + Write has been transmitted, NOT ACK has been 
            received. A STOP condition will be transmitted. */
            Xstruct -> STATUS = I2C_STATUS_SLAVENAK ;
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            if (Xstruct -> STATUS == I2C_STATUS_BUSY ){
                Xstruct -> STATUS = I2C_STATUS_DONE ;
            }
            return 0 ;

        case 0x28:  
            if (Xstruct -> Tx_SIZE > 0 ){
                pI2C -> DAT = ( *Xstruct -> Tx_BUFFER++ ) ;
                Xstruct -> Tx_SIZE-- ;
                pI2C -> CONSET = 0x04 ; // set the AA bit
                pI2C -> CONCLR = 0x08 ; // clear SI flag
                break ;
            }
            else{
                pI2C -> CONSET = 0x14 ; // set STO and AA
                pI2C -> CONCLR = 0x08 ; // clear SI
                if (Xstruct -> STATUS == I2C_STATUS_BUSY ){
                Xstruct -> STATUS = I2C_STATUS_DONE ;
                }
                return 0 ;
            }
        case 0x30:
            /* Data has been transmitted, NOT ACK received. A STOP condition will 
            be transmitted. */
            Xstruct -> STATUS = I2C_STATUS_NAK ; 
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            if (Xstruct -> STATUS == I2C_STATUS_BUSY ){
                Xstruct -> STATUS = I2C_STATUS_DONE ;
            }
            return 0 ;

        case 0x38:
            /* Arbitration has been lost during Slave Address + Write or data. 
            The bus has been released and not addressed Slave mode is entered. 
            A new START condition will be transmitted when the bus is free again. */
            Xstruct -> STATUS = I2C_STATUS_ARBLOST ;
            pI2C -> CONSET = 0x24 ; // set the STA and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            return 0 ;
        
         /* Reciver Mode */
        case 0x40:
            /* Previous state was State 08 or State 10. Slave Address + Read has
            been transmitted, ACK has been received. Data will be received and ACK returned. */
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear the SI flag
            break ;

        case 0x48:
            /* Slave Address + Read has been transmitted, NOT ACK has been received. A STOP
            condition will be transmitted. */
            Xstruct -> STATUS = I2C_STATUS_SLAVENAK ;
            pI2C -> CONSET = 0x14 ; // set the STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            return 0 ;

        case 0x50:
            /* Data has been received, ACK has been returned. Data will be read from DAT. Additional
            data will be received. If this is the last data byte then NOT ACK will be returned, otherwise
            ACK will be returned. */
            // Pointer moves to the next int position (as if it was an array). But returns the old content
            *Xstruct -> Rx_BUFFER++ = ( pI2C -> DAT ) & MASK_DAT ;
            Xstruct -> Rx_SIZE-- ;
            pI2C -> CONCLR = 0x0C ;
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear the SI flag
            break ;

        case 0x58:
            /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
            A STOP condition will be transmitted. */
            *Xstruct -> Rx_BUFFER = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONSET = 0x14 ; // set the STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            if (Xstruct -> STATUS == I2C_STATUS_BUSY ){
                Xstruct -> STATUS = I2C_STATUS_DONE ;
            }
            return 0 ;
    }
return 1 ;
}

/* Maquina de estado para comunicacion (recibir y transmitir) en slave Mode */
void ComSlaveMode(I2C_T *pI2C, MS_T *Xstruct){

    switch(get_STATUS(pI2C)){
        /* Reciver states */
        case 0x60:
            /* Own Slave Address + Write has been received, ACK has been returned. Data will be
            received and ACK returned. */
            Xstruct -> SLAVE_ADDR = pI2C -> DAT & MASK_ADD ; // save slave address in SALVE_ADDR
            pI2C -> CONSET = 0x04 ; // set AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;

        case 0x68:
            /* Arbitration has been lost in Slave Address and R/W bit as bus Master. Own Slave Address
            + Write has been received, ACK has been returned. Data will be received and ACK will be
            returned. STA is set to restart Master mode after the bus is free again. */
            Xstruct -> SLAVE_ADDR = pI2C -> DAT & MASK_ADD ; // save slave address in SALVE_ADDR
            pI2C -> CONSET = 0x24 ; // set STA and the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag 
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;

        case 0x70:
            /*General call has been received, ACK has been returned. Data will be received and ACK
            returned.*/
            Xstruct -> SLAVE_ADDR = pI2C -> DAT & MASK_ADD ; // save slave address in SALVE_ADDR
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag  
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;
            
        case 0x78:
            /* Arbitration has been lost in Slave Address + R/W bit as bus Master. General call has been
            received and ACK has been returned. Data will be received and ACK returned. STA is set
            to restart Master mode after the bus is free again. */
            Xstruct -> SLAVE_ADDR = pI2C -> DAT & MASK_ADD ; // save slave address in SALVE_ADDR
            pI2C -> CONSET = 0x24 ; // set STA and the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag 
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;
            
        case 0x80:
            /* Previously addressed with own Slave Address. Data has been received and ACK has
            been returned. Additional data will be read. */
            *Xstruct -> Rx_BUFFER++ = ( pI2C -> DAT ) & MASK_DAT ;
            Xstruct -> Rx_SIZE-- ;
            if (Xstruct -> Rx_SIZE > 1){
                pI2C -> CONSET = 0x04 ; // set AA bits
                pI2C -> CONCLR = 0x08 ; // clear SI flag
            }
            else{
                pI2C -> CONCLR = 0x0C ; // clear the SI flag and the AA bit
            }
            break ;

        case 0x88:
            /* Previously addressed with own Slave Address. Data has been received and NOT ACK
            has been returned. Received data will not be saved. Not addressed Slave mode is
            entered. */
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag 
            break ;

        case 0x90:
            /* Previously addressed with General Call. Data has been received, ACK has been returned.
            Received data will be saved. Only the first data byte will be received with ACK. Additional
            data will be received with NOT ACK. */
            *Xstruct -> Rx_BUFFER++ = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONCLR = 0x0C ; // clear the SI flag and the AA bit
            break ;

        case 0x98:
            /* Previously addressed with General Call. Data has been received, NOT ACK has been
            returned. Received data will not be saved. Not addressed Slave mode is entered. */
            pI2C -> CONSET = 0x04 ; // set AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0xA0:
            /* A STOP condition or Repeated START has been received, while still addressed as a
            Slave. Data will not be saved. Not addressed Slave mode is entered. */
            pI2C -> CONSET = 0x04 ; // set AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;
        
         /* Transmitter states */
        case 0xA8:
            /* Own Slave Address + Read has been received, ACK has been returned. Data will be
            transmitted, ACK bit will be received. */
            pI2C -> DAT = ( *Xstruct -> Tx_BUFFER++ ) ;
            Xstruct -> Tx_SIZE-- ;  
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0xB0:
            /* Arbitration lost in Slave Address and R/W bit as bus Master. Own Slave Address + Read
            has been received, ACK has been returned. Data will be transmitted, ACK bit will be
            received. STA is set to restart Master mode after the bus is free again. */
            pI2C -> DAT = ( *Xstruct -> Tx_BUFFER++ ) ;
            Xstruct -> Tx_SIZE-- ;  
            pI2C -> CONSET = 0x24 ; // set the STA and AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0xB8:
            /* Data has been transmitted, ACK has been received. Data will be transmitted, ACK bit will
            be received. */
            pI2C -> DAT = ( *Xstruct -> Tx_BUFFER++ ) ;
            Xstruct -> Tx_SIZE-- ;  
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0xC0:
            /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
            A STOP condition will be transmitted. */
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0xC8:
            /* The last data byte has been transmitted, ACK has been received. Not addressed Slave
            mode is entered. */
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

    }

}

/*

-MASTER SLAVE FUNCTIONS 

*/

/*

1. Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
como afecta esto al resto? porque hay que hacerlo? no la usan

2.
void I2C0_IRQHandler(void)
{
	Call I2CM ISR function with the I2C device and transfer rec 
	Chip_I2CM_XferHandler(LPC_I2C0, &i2cmXferRec);
    esta funcion es la maquina de estados.
}

*/
