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

} I2C_T ;

typedef struct {
    uint8_t SLAVE_ADDRESS ;
	uint32_t TxMSG_SIZE ;
    uint8_t *TxMSG_DATA ;
    uint32_t RxMSG_SIZE ;
    uint8_t *RxMSG_DATA ;
    
} MSG_T ;



typedef enum {
	I2C_STATUS_DONE,	/**< Transfer done successfully */
	I2C_STATUS_NAK,		/**< NAK received during transfer */
	I2C_STATUS_ARBLOST,	/**< Aribitration lost during transfer */
	I2C_STATUS_BUSERR,	/**< Bus error in I2C transfer */
	I2C_STATUS_BUSY,	/**< I2C is busy doing transfer */
	I2C_STATUS_SLAVENAK,/**< NAK received after SLA+W or SLA+R */

} I2C_STATUS_T;

/*Macros.*/
/*Escribir todas las configuraciones de los registros*/
/*Flags del control set register*/
#define CONSET_AA           1 << 2 
#define CONSET_SI           1 << 3 
#define CONSET_STO          1 << 4 
#define CONSET_STA          1 << 5 
#define CONSET_I2EN         1 << 6 

/*Flags del control clear register*/
#define CONCLR_AA           1 << 2 
#define CONCLR_SI           1 << 3  
#define CONCLR_STA          1 << 5 
#define CONCLR_I2EN         1 << 6

/* Read and Write */
#define READ                1 << 0
#define WRITE               0 << 0

/* Mascara para leer el estado del registro status solo bit 3:7 */
#define MASK_STAT           0xF8 // 11111000

/* Mascara para leer el dato del registro DAT 0:7 */
#define MASK_DAT            0xFF // 11111111

// Funcion que inicializa el registro CONSET
void ENA_interface(I2C_T *pI2C){
    pI2C -> CONCLR = CONCLR_I2EN | CONCLR_STA | CONCLR_SI | CONCLR_AA ;
    pI2C -> CONSET = CONSET_I2EN ;
}

/* Funcion que envia el START condition*/
void START_send(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_STA ;
}

/* Funcion que envia el START condition*/
void STOP_send(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_STO ;
}

/* Funcion que limpia el flag SI */
void SI_clear(I2C_T *pI2C){
    pI2C -> CONCLR = CONCLR_SI ;
}

uint32_t get_STATUS(I2C_T *pI2C){
    return (uint32_t) pI2C -> STAT & MASK_STAT ;
} 

/* Funcion que transmite n byte en master mode */
uint32_t WRITE_nbyte(I2C_T *pI2C, MSG_T MSG){
    ENA_interface(pI2C) ;
    START_send(pI2C) ;
    while(MSG.TxMSG_SIZE > 0){
        switch(get_STATUS(pI2C)){
            case 0x00:
            /* Bus Error. Enter not addressed Slave mode and release bus */
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
            SI_clear(pI2C) ; // clear SI flag
            return I2C_STATUS_BUSERR ;

            case 0x08:
            /* A START condition has been transmitted. The Slave Address
            + R/W bit will be transmitted, an ACK bit will be received. */
            pI2C -> DAT = ( MSG.SLAVE_ADDRESS << 1 ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag 
            // Los buffers los cargamos en previamente 
            break ;

            case 0x10:
            /*A Repeated START condition has been transmitted. The Slave 
            Address + R/W bit will be transmitted, an ACK bit will be received.*/
            pI2C -> DAT = ( MSG.SLAVE_ADDRESS << 1 ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag 
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
            pI2C -> DAT = ( *MSG.TxMSG_DATA++) ;
            MSG.TxMSG_SIZE-- ;  
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag
            break ;
                
            case 0x20:
            /* Slave Address + Write has been transmitted, NOT ACK has been 
            received. A STOP condition will be transmitted. */
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
            SI_clear(pI2C) ; // clear SI flag
            return I2C_STATUS_SLAVENAK ;

            case 0x30:
            /* Data has been transmitted, NOT ACK received. A STOP condition will 
            be transmitted. */
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
            SI_clear(pI2C) ; // clear SI flag
            return I2C_STATUS_NAK ;

            case 0x38:
            /* Arbitration has been lost during Slave Address + Write or data. 
            The bus has been released and not addressed Slave mode is entered. 
            A new START condition will be transmitted when the bus is free again. */
            pI2C -> CONSET = CONSET_STA | CONSET_AA ; // set the STA and AA bits
            SI_clear(pI2C) ; // clear SI flag
            return I2C_STATUS_ARBLOST ;
        }
    }
    pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA
    SI_clear(pI2C) ; // clear SI
    return I2C_STATUS_DONE ;
} 

/* Funcion que recibe n byte en master mode */
uint32_t READ_nbyte(I2C_T *pI2C, MSG_T MSG){
    ENA_interface(pI2C) ;
    START_send(pI2C) ;
    while(MSG.RxMSG_SIZE > 0){
        switch(get_STATUS(pI2C)){
            case 0x00:
            /* Bus Error. Enter not addressed Slave mode and release bus */
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA bits
            SI_clear(pI2C) ; // clear SI flag
            return I2C_STATUS_BUSERR ;

            case 0x08:
            /* A START condition has been transmitted. The Slave Address
            + R/W bit will be transmitted, an ACK bit will be received. */
            pI2C -> DAT = ( MSG.SLAVE_ADDRESS << 1 ) | READ ; // write Slave address with W bit to DAT
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag 
            // Los buffers los cargamos en previamente 
            break ;

            case 0x10:
            /*A Repeated START condition has been transmitted. The Slave 
            Address + R/W bit will be transmitted, an ACK bit will be received.*/
            pI2C -> DAT = ( MSG.SLAVE_ADDRESS << 1 ) | READ ; // write Slave address with W bit to DAT
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag 
            // Los buffers los cargamos en previamente 
            break ;
            
            /* Reciver Mode */
            case 0x40:
            /* Previous state was State 08 or State 10. Slave Address + Read has
            been transmitted, ACK has been received. Data will be received and ACK returned. */
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear the SI flag
            break ;

            case 0x48:
            /* Slave Address + Read has been transmitted, NOT ACK has been received. A STOP
            condition will be transmitted. */
            pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA
            SI_clear(pI2C) ; // clear SI
            return I2C_STATUS_SLAVENAK ;

            case 0x50:
            /* Data has been received, ACK has been returned. Data will be read from DAT. Additional
            data will be received. If this is the last data byte then NOT ACK will be returned, otherwise
            ACK will be returned. */
            // Pointer moves to the next int position (as if it was an array). But returns the old content
            *MSG.RxMSG_DATA++ = ( pI2C -> DAT ) & MASK_DAT ;
            MSG.RxMSG_SIZE-- ;
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear the SI flag
            break ;

            case 0x58:
            /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
            A STOP condition will be transmitted. */
            *Xstruct -> Rx_BUFFER = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONSET = 0x14 ; // set the STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            
        
        }
    }
    pI2C -> CONSET = 0x14 ; // set STO and AA
    pI2C -> CONCLR = 0x08 ; // clear SI
    return I2C_STATUS_DONE ;
}
