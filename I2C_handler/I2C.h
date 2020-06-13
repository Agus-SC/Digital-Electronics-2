
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

/* Read and Write */
#define READ                0X01
#define WRITE               0X00

/* Mascara para leer el estado del registro status solo bit 3:7 */
#define MASK_STAT           0xF8 // 11111000

/* Mascara para leer el dato del registro DAT 0:7 */
#define MASK_DAT            0xFF // 11111111
/*Masks*/
/*#define MASK_MASK(n) (n & 0xFE)*/

// Estructura para Master Mode.
/* uint16_t Porque quizas se necesita usar un buffer ya se para transmitir
o recibir de mas de 2^8 = 256 elementos */
typedef struct{
    uint8_t SLAVE_ADDR; /* Le paso el slave address */
    uint8_t OPTIONS;	/* Option, todavia no sabemos */
	uint8_t STATUS;		/* Estado de transmision */
	uint16_t Tx_SIZE;	/* Tama;o del buffer a transmitir (Numero de bytes), Transmito el Address si recivo */
	uint16_t Rx_SIZE;	/* Tama;o del buffer a recibir (Numero de bytes), 0 si transmito*/
	uint8_t *Tx_BUFFER; /* Puntero al buffer de datos a transmitir, NULL si recibo */
	uint8_t *Rx_BUFFER;
     
}Master_T;

// Funcion que inicializa el registro CONSET
void init_CONSET(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_I2EN ;
}

/* Funcion que envia el START condition*/
void send_START(I2C_T *pI2C){
    pI2C -> CONSET = CONSET_I2EN | CONSET_STA ;
}

/* Funcion que lee el registro STATUS */
void get_STATUS(I2C_T *pI2C){
    return (uint32_t) pI2C -> STAT & MASK_STAT ;
} 

/* Maquina de estado para transmit Master Mode */
void TransmitMasterMode(I2C_T *pI2C, Master_T *Mstruct){

    switch(get_STATUS(pI2C)){
        
        case 0x00:
            /* Bus Error. Enter not addressed Slave mode and release bus */
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0x08:
            /* A START condition has been transmitted. The Slave Address
            + R/W bit will be transmitted, an ACK bit will be received. */
            pI2C -> DAT = ( Mstruct -> SLAVE_ADDR ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag 
            // Los buffers los cargamos en previamente 
            break ;

        case 0x10:
            /*A Repeated START condition has been transmitted. The Slave 
            Address + R/W bit will be transmitted, an ACK bit will be received.*/
            pI2C -> DAT = ( Mstruct -> SLAVE_ADDR ) | WRITE ; // write Slave address with W bit to DAT
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear SI flag  
            // Los buffers los cargamos en previamente 
            break ;
            
        /* Transmitter Mode */
        case 0x18:
            /* Previous state was State 8 or State 10, Slave Address + Write 
            has been transmitted, ACK has been received. The first data byte 
            will be transmitted, an ACK bit will be received. */
            if (Mstruct -> Tx_SIZE != 0 ){
                // si no quiero transmitir y quiero recibir manda un START condition
                if (Mstruct -> Rx_SIZE){
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
                pI2C -> DAT = ( *Mstruct -> Tx_BUFFER++ ) ;
                Mstruct -> Tx_SIZE-- ;  
                pI2C -> CONSET = 0x04 ; // set the AA bit
                pI2C -> CONCLR = 0x08 ; // clear SI flag
                break ;
            }
            
        case 0x20:
            /* Slave Address + Write has been transmitted, NOT ACK has been 
            received. A STOP condition will be transmitted. */
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0x28:
            /* Data has been transmitted, ACK has been received. If the transmitted
            data was the last data byte then transmit a STOP condition, otherwise 
            transmit the next data byte. */
            if (Mstruct -> Tx_SIZE != 0 ){
                // si no quiero transmitir y quiero recibir manda un START condition
                if (Mstruct -> Rx_SIZE){
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
                pI2C -> DAT = ( *Mstruct -> Tx_BUFFER++ ) ;
                Mstruct -> Tx_SIZE-- ;  
                pI2C -> CONSET = 0x04 ; // set the AA bit
                pI2C -> CONCLR = 0x08 ; // clear SI flag
                break ;
            }   

        case 0x30:
            /* Data has been transmitted, NOT ACK received. A STOP condition will 
            be transmitted. */
            pI2C -> CONSET = 0x14 ; // set STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0x38:
            /* Arbitration has been lost during Slave Address + Write or data. 
            The bus has been released and not addressed Slave mode is entered. 
            A new START condition will be transmitted when the bus is free again. */
            pI2C -> CONSET = 0x24 ; // set the STA and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;
        
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
            pI2C -> CONSET = 0x14 ; // set the STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

        case 0x50:
            /* Data has been received, ACK has been returned. Data will be read from DAT. Additional
            data will be received. If this is the last data byte then NOT ACK will be returned, otherwise
            ACK will be returned. */
            *Mstruct -> Rx_BUFFER++ = ( pI2C -> DAT ) & MASK_DAT ;
            Mstruct -> Rx_SIZE-- ;
            pI2C -> CONCLR = 0x0C ;
            pI2C -> CONSET = 0x04 ; // set the AA bit
            pI2C -> CONCLR = 0x08 ; // clear the SI flag
            break ;

        case 0x58:
            /* Data has been received, NOT ACK has been returned. Data will be read from DAT.
            A STOP condition will be transmitted. */
            Mstruct -> Rx_BUFFER = ( pI2C -> DAT ) & MASK_DAT ;
            pI2C -> CONSET = 0x14 ; // set the STO and AA bits
            pI2C -> CONCLR = 0x08 ; // clear SI flag
            break ;

    }

}


// void I2CM_Xfer(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
// {
// 	/* set the transfer status as busy */
// 	xfer->status = I2CM_STATUS_BUSY;
// 	/* Clear controller state. */
// 	Chip_I2CM_ResetControl(pI2C);
// 	/* Enter to Master Transmitter mode */
// 	Chip_I2CM_SendStart(pI2C);
// }

// /**
//  * @brief	Reset I2C controller state
//  * @param	pI2C	: Pointer to selected I2C peripheral
//  * @return	Nothing
//  * @note	This function clears all control/status flags.
//  */
// void I2CM_ResetControl(LPC_I2C_T *pI2C)
// {
// 	/* Reset STA, AA and SI. Stop flag should not be cleared as it is a reserved bit */
// 	pI2C->CONCLR = I2C_CON_SI | I2C_CON_STA | I2C_CON_AA;

// }
// /**
//  * @brief	Transmit START or Repeat-START signal on I2C bus
//  * @param	pI2C	: Pointer to selected I2C peripheral
//  * @return	Nothing
//  * @note	This function sets the controller to transmit START condition when
//  *          the bus becomes free.
//  */
// void I2CM_SendStart(LPC_I2C_T *pI2C)
// {
// 	pI2C->CONSET = I2C_CON_I2EN | I2C_CON_STA;
// }

