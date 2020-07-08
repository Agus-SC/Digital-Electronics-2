/****************************** Inicializacion TECLAS ******************************/
/* Tipo de datos */ 
typedef unsigned char uint8_t ;
typedef unsigned short uint16_t ;
typedef unsigned int uint32_t ;

/* Direccion SCU Base */
#define SCU_BASE        0x40086000

/* Direccion GPIO Base */
#define GPIO_PORT_BASE        0x400F4000

/* TEC1 asociado al P1_0 FUNC0 GPIO0[4] */
#define SFSP1_0_OFFSET		0x080	

/* TEC2 asociado al P1_1 FUNC0 GPIO0[8] */
#define SFSP1_1_OFFSET		0x084	

/* TEC3 asociado al P1_2 FUNC0 GPIO0[9] */
#define SFSP1_2_OFFSET	    0x088	

/* TEC4 asociado al P1_6 FUNC0 GPIO1[9] */
#define SFSP1_6_OFFSET		0x098

/* Definicion de los punteros a los registros del SCU de los pines asociados a las teclas*/
uint32_t *SFSP1_0 = (uint32_t *)(SCU_BASE + SFSP1_0_OFFSET) ; 
uint32_t *SFSP1_1 = (uint32_t *)(SCU_BASE + SFSP1_1_OFFSET) ; 
uint32_t *SFSP1_2 = (uint32_t *)(SCU_BASE + SFSP1_2_OFFSET) ; 
uint32_t *SFSP1_6 = (uint32_t *)(SCU_BASE + SFSP1_6_OFFSET) ; 

void TECs_init(void){
    /*
    Configuramos la funcion 0
    Deshabilitamos pull down
    Habilitmos pull up
    Habilitamos el Input buffer
    */
    *SFSP1_0 |= (1 << 6) | (0x0) ;
    *SFSP1_1 |= (1 << 6) | (0x0) ;
    *SFSP1_2 |= (1 << 6) | (0x0) ;
    *SFSP1_6 |= (1 << 6) | (0x0) ;

}
