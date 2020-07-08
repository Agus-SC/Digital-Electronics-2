/****************************** Inicializacion LEDS ******************************/
/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

/* Direccion SCU Base */
#define SCU_BASE        0x40086000

/* Direccion GPIO Base */
#define GPIO_PORT_BASE        0x400F4000 

/* LED RED asociado al pin P2_0 FUNC4 */
#define SFSP2_0_OFFSET        0x100

/* LED GREEN asociado al pin P2_1 FUNC4 */
#define SFSP2_1_OFFSET        0x104

/* LED BLUE asociado al pin P2_2 FUNC4 */
#define SFSP2_2_OFFSET        0x108

/* LED 1 asociado al pin P2_10 FUNC0 */
#define SFSP2_10_OFFSET        0x128 

/* LED 2 asociado al pin P2_11 FUNC0 */
#define SFSP2_11_OFFSET        0x12C 

/* LED 3 asociado al pin P2_12 FUNC0 */
#define SFSP2_12_OFFSET        0x130 

/* Offset de los registro DIR para puertos 0, 1, 5 */ 
#define DIR0_OFFSET            0x2000      
#define DIR1_OFFSET            0x2004      
#define DIR5_OFFSET            0x2014

/* Offset de los registro SET para puertos 0, 1, 5 */
#define SET0_OFFSET            0x2200      
#define SET1_OFFSET            0x2204
#define SET5_OFFSET            0x2214

/* Offset de los registro CLR para puertos 0, 1, 5 */
#define CLR0_OFFSET            0x2280
#define CLR1_OFFSET            0x2284
#define CLR5_OFFSET            0x2294

/* Definicion de los punteros a los registros del SCU de los pines asociados a los LEDS*/
uint32_t *SFSP2_0 = (uint32_t *)(SCU_BASE + SFSP2_0_OFFSET) ; 
uint32_t *SFSP2_1 = (uint32_t *)(SCU_BASE + SFSP2_1_OFFSET) ; 
uint32_t *SFSP2_2 = (uint32_t *)(SCU_BASE + SFSP2_2_OFFSET) ; 
uint32_t *SFSP2_10 = (uint32_t *)(SCU_BASE + SFSP2_10_OFFSET) ; 
uint32_t *SFSP2_11 = (uint32_t *)(SCU_BASE + SFSP2_11_OFFSET) ; 
uint32_t *SFSP2_12 = (uint32_t *)(SCU_BASE + SFSP2_12_OFFSET) ; 

/* Definicion de los punteros a los registros del GPIO asociados a los LEDS */
uint32_t *DIR0 = ( uint32_t* )( ( GPIO_PORT_BASE ) + ( DIR0_OFFSET ) ) ; 
uint32_t *DIR1 = ( uint32_t* )( ( GPIO_PORT_BASE ) + ( DIR1_OFFSET ) ) ;  
uint32_t *DIR5 = ( uint32_t* )( ( GPIO_PORT_BASE ) + ( DIR5_OFFSET ) ) ; 

uint32_t *SET0 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( SET0_OFFSET ) ) ;
uint32_t *SET1 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( SET1_OFFSET ) ) ;
uint32_t *SET5 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( SET5_OFFSET ) ) ;

uint32_t *CLR0 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( CLR0_OFFSET ) ) ;
uint32_t *CLR1 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( CLR1_OFFSET ) ) ;
uint32_t *CLR5 = ( uint32_t* )(( GPIO_PORT_BASE ) + ( CLR5_OFFSET ) ) ;

void LEDs_init(void){
    /*
    Configuramos la funcion 4
    Deshabilitamos pull down
    Habilitmos pull up
    */
    *SFSP2_0 |= (1 << 3) | (0x4) ;
    *SFSP2_1 |= (1 << 3) | (0x4) ;
    *SFSP2_2 |= (1 << 3) | (0x4) ;
    /*
    Configuramos la funcion 0
    Deshabilitamos pull down
    Habilitmos pull up
    */
    *SFSP2_10 |= (1 << 3) | (0x0) ;
    *SFSP2_11 |= (1 << 3) | (0x0) ;
    *SFSP2_12 |= (1 << 3) | (0x0) ;
    /*
    Configuramos los pines como salida
    */
    *DIR0 |= (0x1 << 14) ; //LED_1 GPIO0[14]
    *DIR1 |= (0x1 << 12) | (0x1 << 11) ; //LED_3 GPIO1[12], LED_2 GPIO1[11] 
    *DIR5 |= (0x1 << 2) | (0x1 << 1) | (0x1 << 0) ; //LED_B GPIO0[2], LED_G GPIO0[1], LED_R GPIO0[0]

    *CLR0 |= (0x1 << 14) ;
    *CLR1 |= (0x1 << 12) | (0x1 << 11)  ;
    *CLR5 |= (0x1 << 2) | (0x1 << 1) | (0x1 << 0) ;

}

void LEDs_set(uint8_t LED){
    switch(LED){

        case 0: 
            /*SET LED RED*/
            *SET5 |= (0x1 << 0) ;
            break ;

        case 1:
            /*SET LED GREEN*/
            *SET5 |= (0x1 << 1) ;
            break ;

        case 2:
            /*SET LED BLUE*/
            *SET5 |= (0x1 << 2) ;
            break ;
         
        case 3:
            /*SET LED 1*/
            *SET0 |= (0x1 << 14) ;
            break ;
        
        case 4:
            /*SET LED 2*/
            *SET1 |= (0x1 << 11) ;
            break ;
        
        case 5:
            /*SET LED 3*/
            *SET1 |= (0x1 << 12) ;
            break ;
        
        default:
            break ;
        
    }

}