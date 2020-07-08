/****************************** Configuracion de Interrupciones ******************************/
/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

/* Direccion SCU Base */
#define SCU_BASE        0x40086000

/* Direccion GPIO Base */
#define GPIO_PORT_BASE      0x400F4000

/* Direccion GPIO PIN INTERRUPT BASE */
#define GPIO_PINT_REGISTERS      ((GPIO_PINT_T *) 0x40087000)

/* Direccion del NVIC Base*/
#define NVIC_BASE       0xE000E000

/* GPIO pin interrupts */
typedef struct{
	uint32_t ISEL;
	uint32_t IENR;
	uint32_t SIENR;
	uint32_t CIENR;
	uint32_t IENF;
	uint32_t SIENF;
	uint32_t CIENF;
	uint32_t RISE;
	uint32_t FALL;
	uint32_t IST;

}GPIO_PINT_T;

/* Offsets de los registros ISER en el NVIC */
/*
18 I2C0 
19 I2C1

32 PIN_INT0 GPIO pin interrupt 0
33 PIN_INT1 GPIO pin interrupt 1
34 PIN_INT2 GPIO pin interrupt 2
35 PIN_INT3 GPIO pin interrupt 3
*/
#define ISER0_OFFSET       0x100
#define ISER1_OFFSET       0x104

/* Offsets de los registros PINTSEL en el SCU */
#define PINTSEL0_OFFSET     0xE00
#define PINTSEL1_OFFSET     0xE04

/* Definicion de los punteros a los registros del NVIC ISER */
uint32_t *ISER0 = (uint32_t *)(SCU_BASE + ISER0_OFFSET) ; 
uint32_t *ISER1 = (uint32_t *)(SCU_BASE + ISER1_OFFSET) ;

/* Definicion de los punteros a los registros del SCU PINTSEL*/
uint32_t *PINTSEL0 = (uint32_t *)(SCU_BASE + PINTSEL0_OFFSET) ; 
uint32_t *PINTSEL1 = (uint32_t *)(SCU_BASE + PINTSEL1_OFFSET) ;

/* Funcion para configurar una determinada tecla como interrupcion */
void TEC_interrupt(uint8_t TEC){
    switch(TEC){
        case 1:
            /* 
            Configuracion interrupcion 0
            Configuro puerto 0 pin 4 para la tecla 1 en PINTSEL
            Configuro fuente de interrupcion 32 en ISER1  
            */
            *PINTSEL0 |= (0x0 << 5) | (0x4 << 0) ; 
            *ISER1 |= (0x1 << 0) ; 
            break ;

        case 2:
            /* 
            Configuracion interrupcion 1
            Configuro puerto 0 pin 8 para la tecla 2
            Configuro fuente de interrupcion 33 en ISER1   
            */
            *PINTSEL0 |= (0x0 << 13) | (0x8 << 8) ; 
            *ISER1 |= (0x1 << 1) ; 
            break ;

        case 3:
            /* 
            Configuracion interrupcion 2
            configuro puerto 0 pin 9 para la tecla 3 
            Configuro fuente de interrupcion 34 en ISER1 
            */
            *PINTSEL0 |= (0x0 << 21) | (0x9 << 16) ;
            *ISER1 |= (0x1 << 2) ;  
            break ;

        case 4:
            /* 
            Configuracion interrupcion 3
            configuro puerto 1 pin 9 para la tecla 3 
            Configuro fuente de interrupcion 35 en ISER1 
            */
            *PINTSEL0 |= (0x1 << 29) | (0x9 << 24) ; 
            *ISER1 |= (0x1 << 3) ; 
            break ;

        default:
            break;

    }
    /* Configuramos las teclas sensibles a flancos */
    GPIO_PINT_REGISTERS -> ISEL = (0x0 << 0) ;
    /* Configuramos las teclas sensibles a flancos descendentes por que trabajamos con pull-up*/
    GPIO_PINT_REGISTERS -> IENR = (0x0 << 0) ;
    GPIO_PINT_REGISTERS -> SIENR = (0x0 << 0) ;
    GPIO_PINT_REGISTERS -> IENF = (0x1 << 0 ) ; 
    

}

/* Funcion para configurar las interfaces I2C como interrupcion */
void I2C_interrupt(uint8_t interface){

}