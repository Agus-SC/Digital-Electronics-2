
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

/*Masks*/
/*#define MASK_MASK(n) (n & 0xFE)*/

typedef struct{
    uint8_t SLAVE_ADDR;
    uint8_t *Tx_BUFFER;
    int Tx_SIZE; 
}MasterTransfer_TMsg;

typedef struct{
    uint8_t *Rx_BUFFER;
    int Rx_SIZE; 
}MasterTransfer_RMsg;

bool I2C_Master_Transfer(){

}