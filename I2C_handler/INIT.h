/* Este archivo contiene todas las funciones y declaraciones 
para inicializar los pines a utilizar: I2C, LEDS, TECLAS */

/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;


/******************************* Inicializacion CLOCK I2C ******************************/

/* Direccion CCU1 Base */
#define CCU1_BASE   0x40051000

/* La Interfaz 0 de I2C esta asociado al branch CLK_APB1_I2C0 de la base BASE_APB1_CLK */
/* Offset de el registro de configuracion de CLK_APB1_I2C0 */
#define CLK_APB1_I2C0_OFFSET    0x210

/* La Interfaz 1 de I2C esta asociado al branch CLK_APB3_I2C1 de la base BASE_APB3_CLK */
/* Offset de el registro de configuracion de CLK_APB3_I2C1 */
#define CLK_APB3_I2C1_OFFSET    0x108

/****************************** Inicializacion I2C ******************************/

/* Direccion SCU Base */
#define SCU_BASE        0x40086000

/* Direccion GPIO Base */
#define GPIO_PORT_BASE        0x400F4000       

/* La Interfaz 0 de I2C tiene su propio registro SFSI2C0 */
/* Direccion registro SFSI2C0 */
#define I2C0_OFFSET        0xC84

/* La linea SDA de la Interfaz 1 de I2C esta en el pin P2_3 FUNC1 o PE_13 FUNC2 */
/* Offset P2_3 para SDA I2C1 */
#define I2C1_SDA_OFFSET     0x10C

/* La linea SCL de la Interfaz 1 de I2C esta en el pin P2_4 FUNC1 o PE_13 FUNC2 */
/* Offset P2_4 para SCL I2C1 */
#define I2C1_SCL_OFFSET     0x110


/* Definicion de los punteros a los registros del CCU1 para I2C*/
uint32_t *I2C0_CLK = (uint32_t *)(CCU1_BASE + CLK_APB1_I2C0_OFFSET ) ;
uint32_t *I2C1_CLK = (uint32_t *)(CCU1_BASE + CLK_APB3_I2C1_OFFSET ) ;

/* Definicion de los punteros a los registros del SCU de los pines asociados a I2C*/
uint32_t *I2C0 = (uint32_t *)(SCU_BASE + I2C0_OFFSET) ;
uint32_t *I2C1_SDA = (uint32_t *)(SCU_BASE + I2C1_SDA_OFFSET) ;
uint32_t *I2C1_SCL = (uint32_t *)(SCU_BASE + I2C1_SCL_OFFSET) ;


void I2C_CLK_init(uint8_t interface){
    if (interface == 0){
        /*Habilitamos Clock de la interfaz 0 */
        *I2C0_CLK |= (0x1) ;
    }
    else{
        /*Habilitamos Clock de la interfaz 1 */
        *I2C1_CLK |= (0x1) ;
    }
}

void I2C_init(uint8_t interface){
    if (interface == 0){
        /* configuracion interfaz 0,  
        Habilitamos input receiver SCL
        Deshabilitamos input glitch filter del pin SCL (trabajamos con STANDARD MODE)
        Habilitamos input receiver SDA
        Deshabilitamos input glitch filter del pin SDA (trabajamos con STANDARD MODE) */
        *I2C0 |= (0x1 << 15) | (0x1 << 11) | (0x1 << 7) | (0x1 << 3) ;  
    }
    else{
        /*configuracion interfaz 1
        Configuramos la funcion 1
        habilitamos pull up: 0 en bit EPUN
        habilitamos el input buffer
        deshabilitamos el input filter glitch
        */
        *I2C1_SDA |= (0x1 << 7) | (0x1 << 6) | (0x1 << 0) ;
        *I2C1_SCL |= (0x1 << 7) | (0x1 << 6) | (0x1 << 0) ; 
    }
}






