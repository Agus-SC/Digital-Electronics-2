/**
 * @brief	Main program body
 * @return	int
 */
int main(void)
{
	int tmp;
	int xflag = 0;
	static I2C_XFER_T xfer;

	SystemCoreClockUpdate();
	Board_Init();
	i2c_app_init(I2C0, SPEED_100KHZ);
	i2c_app_init(I2C1, SPEED_100KHZ);

	/* Simulate an EEPROM slave in I2C0 */
	i2c_eeprom_init(I2C_EEPROM_BUS);

	/* Simuldate an IO Expansion slave in I2C0 */
	i2c_iox_init(I2C_IOX_BUS);

	while (!xflag) {
		switch (i2c_menu()) {
		case 0: /* Finaliza la transferencia*/
			xflag = 1;
			DEBUGOUT("End of I2C Demo! Bye!\r\n");
			break;

		case 1: /*Se fija que interfaz se va utilizar, */
			tmp = con_get_input("Select I2C device [0 or 1] : ");
			/*Le pide al usuario que seleccione que interfaz usar*/
			DEBUGOUT("\r\n");
			/* si es la cero se fija que este disponible y si lo esta setea el modo de operacion
			en polling y se lo asigna a la variable i2cDev*/
			if ((I2C_ID_T) tmp == I2C0) {
				if (i2cDev == I2C0) {
					break;
				}
				i2c_set_mode(I2C0, 0);
				i2cDev = I2C0;
			}
			/*Idem para la interfaz 1*/
			else if ((I2C_ID_T) tmp == I2C1) {
				if (i2cDev == I2C1) {
					break;
				}
				i2c_set_mode(I2C1, 0);
				i2cDev = I2C1;
			}
			else {
				DEBUGOUT("Invalid I2C Device [Must be 0 or 1]\r\n");
			}
			break;

		case 2:
			i2c_set_mode(i2cDev, !(mode_poll & (1 << i2cDev)));
			break;

		case 3:
			i2c_probe_slaves(i2cDev);
			break;

		case 4:
			i2c_rw_input(&xfer, 1);
			tmp = Chip_I2C_MasterRead(i2cDev, xfer.slaveAddr, xfer.rxBuff, xfer.rxSz);
			DEBUGOUT("Read %d bytes of data from slave 0x%02X.\r\n", tmp, xfer.slaveAddr);
			con_print_data(buffer[1], tmp);
			break;

		case 5:
			i2c_rw_input(&xfer, 2);
			if (xfer.txSz == 0) {
				break;
			}
			tmp = Chip_I2C_MasterSend(i2cDev, xfer.slaveAddr, xfer.txBuff, xfer.txSz);
			DEBUGOUT("Written %d bytes of data to slave 0x%02X.\r\n", tmp, xfer.slaveAddr);
			break;

		case 6:
			i2c_rw_input(&xfer, 3);
			tmp = xfer.rxSz;
			if (!tmp && !xfer.txSz) {
				break;
			}
			Chip_I2C_MasterTransfer(i2cDev, &xfer);
			DEBUGOUT("Master transfer : %s\r\n",
					 xfer.status == I2C_STATUS_DONE ? "SUCCESS" : "FAILURE");
			DEBUGOUT("Received %d bytes from slave 0x%02X\r\n", tmp - xfer.rxSz, xfer.slaveAddr);
			con_print_data(buffer[1], tmp - xfer.rxSz);
			break;

		default:
			DEBUGOUT("Input Invalid! Try Again.\r\n");
		}
	}
	Chip_I2C_DeInit(I2C0);
	Chip_I2C_DeInit(I2C1);

	return 0;
}