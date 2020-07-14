/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

typedef _Bool bool;
#define	false	0
#define	true	1

#define limit_time 1000 

/* Registros canal 0 (cero) I2C */

#define I2C0        ((I2C_T  *)  0x400A1000)
#define I2C1        ((I2C_T  *)  0x400E0000)

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
	uint32_t SIZE ;
    uint8_t *DATA ;
    uint8_t *RESTART ; /* Para no perder la direccion de data. */
    bool Tx ; /* True transmitir Flase recibir */

    
} MSG_T ;

typedef enum {
	I2C_STATUS_DONE,	/**< Transfer done successfully */
	I2C_STATUS_NAK,		/**< NAK received during transfer */
	I2C_STATUS_ARBLOST,	/**< Aribitration lost during transfer */
	I2C_STATUS_BUSERR,	/**< Bus error in I2C transfer */
	I2C_STATUS_BUSY,	/**< I2C is busy doing transfer */
	I2C_STATUS_SLAVENAK,/**< NAK received after SLA+W or SLA+R */
    I2C_STATUS_OK,  

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

//funcion que devuelve 0 para interfaz I2C0 o 1 para interfaz I2C1
bool get_interface(uint8_t interface){
    if (interface == 0){
        return false ;
    }
    return true ;
} 

// Funcion que inicializa el registro CONSET
void ENA_interface(I2C_T *pI2C){
    pI2C -> CONCLR = CONCLR_I2EN | CONCLR_STA | CONCLR_SI | CONCLR_AA ;
    pI2C -> CONSET = CONSET_I2EN ;
}

/* Funcion que envia el START condition*/
void START_send(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_STA ;
}

/* Funcion que envia el STOP condition*/
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

void set_SLAVEADDR(I2C_T *pI2C, uint8_t SLAVE_ADDRESS){
    int i ; 
    pI2C -> ADR0 = SLAVE_ADDRESS << 1 ;
    /* duplicamos */ 
    pI2C -> ADR1 = SLAVE_ADDRESS << 1 ;
    pI2C -> ADR2 = SLAVE_ADDRESS << 1 ;
    pI2C -> ADR3 = SLAVE_ADDRESS << 1 ;
    /* Seteamos la mascara */
    pI2C -> MASK[0] = SLAVE_ADDRESS ;
    for(i = 1 ; i < 4 ; i++){
        pI2C -> MASK[i] = 0x00 ;
    } 
}

void init_SLAVE(I2C_T *pI2C, uint8_t SLAVE_ADDRESS){
    set_SLAVEADDR(pI2C, SLAVE_ADDRESS) ;
    pI2C ->CONCLR = CONCLR_STA | CONCLR_SI ;
    pI2C -> CONSET = CONSET_I2EN | CONSET_AA ;
}

/* Funcion que transmite n byte en master mode */
uint32_t Tx_master(I2C_T *pI2C, MSG_T MSG){
    
    while(!(pI2C -> CONSET & CONSET_SI)){}
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
        pI2C -> DAT = ( *MSG.DATA++) ;
        MSG.SIZE-- ;  
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
    return I2C_STATUS_OK ;
} 

/* Funcion que recibe n byte en master mode */
uint32_t Rx_master(I2C_T *pI2C, MSG_T MSG){
    while(!(pI2C -> CONSET & CONSET_SI)){}
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
        *MSG.DATA++ = ( pI2C -> DAT ) & MASK_DAT ;
        if (MSG.SIZE == 1){
            pI2C -> CONCLR = CONCLR_AA  ; // clear AA
            SI_clear(pI2C) ; // clear SI
            break ;
        }
        MSG.SIZE-- ;
        pI2C -> CONSET = CONSET_AA ; // set the AA bit
        SI_clear(pI2C) ; // clear the SI flag
        break ;

        case 0x58:
        /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
        A STOP condition will be transmitted. */
        *MSG.DATA = ( pI2C -> DAT ) & MASK_DAT ;
        MSG.SIZE-- ;
        pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set the AA bit
        SI_clear(pI2C) ; // clear the SI flag
        break ;    
    }
    return I2C_STATUS_DONE ;
}

uint32_t Polling_master(I2C_T *pI2C, MSG_T MSG){
    uint32_t status ;
    ENA_interface(pI2C) ;
    START_send(pI2C) ;
    while(MSG.SIZE > 0){
        if(MSG.Tx){
            status = Tx_master(pI2C, MSG) ;
        }
        else{
            status = Rx_master(pI2C, MSG) ;
        }
        if ( status != I2C_STATUS_OK){
            MSG.DATA = MSG.RESTART ; 
            return status ;
        }   
    }
    pI2C -> CONSET = CONSET_STO | CONSET_AA ; // set STO and AA
    SI_clear(pI2C) ; // clear SI
    MSG.DATA = MSG.RESTART ; 
    return I2C_STATUS_DONE ;
}

bool waitforSI(I2C_T *pI2C){
    int timeout = 0 ; 
    while(!(pI2C -> CONSET & CONSET_SI)){
        timeout++ ;
		if (timeout > limit_time){
        return false ; 
        }
    }
    return true ;
}

uint32_t Rx_slave(I2C_T *pI2C, MSG_T MSG){
    while(!(pI2C -> CONSET & CONSET_SI)){}
    switch(get_STATUS(pI2C)){
        /* Reciver states */
        case 0x60:
            /* Own Slave Address + Write has been received, ACK has been returned. Data will be
            received and ACK returned. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
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
            SI_clear(pI2C) ; // clear SI flag 
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;

        case 0x70:
            /*General call has been received, ACK has been returned. Data will be received and ACK
            returned.*/
            pI2C -> CONSET = CONSET_AA ; // set the AA bit
            SI_clear(pI2C) ; // clear SI flag  
            // 3. Set up Slave Receive mode data buffer.
            // 4. Initialize Slave data counter.
            break ;
            
        case 0x80:
            /* Previously addressed with own Slave Address. Data has been received and ACK has
            been returned. Additional data will be read. */
            *MSG.DATA++ = ( pI2C -> DAT ) & MASK_DAT ;
            if (MSG.SIZE == 1){
                MSG.SIZE-- ;
                pI2C -> CONCLR = CONCLR_AA  ; // clear AA
                SI_clear(pI2C) ; // clear SI flag
                break ;
            }
            MSG.SIZE-- ;
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
            break ;

        case 0x88:
            /* Previously addressed with own Slave Address. Data has been received and NOT ACK
            has been returned. Received data will not be saved. Not addressed Slave mode is
            entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
            break ;

        case 0x90:
            /* Previously addressed with General Call. Data has been received, ACK has been returned.
            Received data will be saved. Only the first data byte will be received with ACK. Additional
            data will be received with NOT ACK. */
            *MSG.DATA = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONCLR = CONCLR_AA  ; // clear AA
            SI_clear(pI2C) ; // clear SI flag
            break ;

        case 0x98:
            /* Previously addressed with General Call. Data has been received, NOT ACK has been
            returned. Received data will not be saved. Not addressed Slave mode is entered. */
        case 0xA0:
            /* A STOP condition or Repeated START has been received, while still addressed as a
            Slave. Data will not be saved. Not addressed Slave mode is entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
            break ;
    }
   return I2C_STATUS_OK ;    
}        
         
uint32_t Tx_slave(I2C_T *pI2C, MSG_T MSG){
    while(!(pI2C -> CONSET & CONSET_SI)){}  
    switch(get_STATUS(pI2C)){        
        /* Transmitter states */
        case 0xA8:
            /* Own Slave Address + Read has been received, ACK has been returned. Data will be
            transmitted, ACK bit will be received. */
        case 0xB8:
            /* Data has been transmitted, ACK has been received. Data will be transmitted, ACK bit will
            be received. */
            pI2C -> DAT = ( *MSG.DATA++ ) ;
            MSG.SIZE-- ;  
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
            break ;

        case 0xB0:
            /* Arbitration lost in Slave Address and R/W bit as bus Master. Own Slave Address + Read
            has been received, ACK has been returned. Data will be transmitted, ACK bit will be
            received. STA is set to restart Master mode after the bus is free again. */
            pI2C -> DAT = ( *MSG.DATA++ ) ;
            MSG.SIZE-- ;  
            pI2C -> CONSET = CONSET_STA | CONSET_AA ;  // set STA and the AA bit
            SI_clear(pI2C) ; // clear SI flag 
            break ;

        case 0xC0:
            /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
            A STOP condition will be transmitted. */
        case 0xC8:
            /* The last data byte has been transmitted, ACK has been received. Not addressed Slave
            mode is entered. */
            pI2C -> CONSET = CONSET_AA ; // set AA bits
            SI_clear(pI2C) ; // clear SI flag
            break ;
    }
    return I2C_STATUS_OK ;  
}

uint32_t Polling_slave(I2C_T *pI2C, MSG_T MSG){
    uint32_t status ;
    init_SLAVE(pI2C, MSG.SLAVE_ADDRESS) ;
    while(MSG.SIZE > 0){
        if(MSG.Tx){
            status = Tx_slave(pI2C, MSG) ;
        }
        else{
            status = Rx_slave(pI2C, MSG) ;
        }
        if ( status != I2C_STATUS_OK){
            MSG.DATA = MSG.RESTART ; 
            return status ;
        }   
    }
    MSG.DATA = MSG.RESTART ; 
    return I2C_STATUS_DONE ;
}

/*
Master
define el address
define el tamaño de los datos a recibir
inicializa I2C comunication as master 
**** master -> slave ****
transmite el slave address
escribe un dato
termina la transmision 
**** slave -> master ****
usa una funcion requestFrom(add, tamaño de los datos a recibir)
lee los datos que le envia el slave y los guarda en una variable

Slave
define el address
define el tamaño de los datos a transmitir
define el dato a transmitir 
inicializa I2C comunication as slave
usa una funcion onReceive(requestEvent) son eventos/funciones
que se disparan cuando el master solicita datos. En este caso define
los datos a transmitir y los escribe.
usa una funcion onReceive(receiveEvent) son eventos/funciones
que se disparan cuando el master transmite datos. En estaa funcion
lee los datos y los almacena en una variable








*/