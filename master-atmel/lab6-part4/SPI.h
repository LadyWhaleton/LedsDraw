#ifndef __SPI_HANDLER__
#define __SPI_HANDLER__

#define DDR_SPI DDRB
#define DDR_SS  PORTB
#define DD_SS   4
#define DD_MOSI 5
#define DD_MISO 6
#define DD_SCK  7

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK) | (1 << DD_SS);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	SREG |= 0x80;
}

void SPI_MasterTransmit(char cData)
{
	
	DDR_SS &= ~(1 << DD_SS);
	
	/* Start transmission */
	SPDR = cData;

	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	
	DDR_SS |= (1 << DD_SS);
}

void SPI_SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDR_SPI = (1<<DD_MISO);
	/* Enable SPI */
	SPCR = (1<<SPE);
}

char SPI_SlaveReceive(void)
{
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)));
	
	/* Return Data Register */
	return SPDR;
}

/*
ISR(SPI_STC_vect) { // this is enabled in with the SPCR register’s “SPI

}*/

#endif
