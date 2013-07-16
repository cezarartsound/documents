/*
 * SerialPort.c
 *
 *  Created on: 22 de Dez de 2011
 *      Author: fabio32883
 */

#include "SerialPort.h"
#include <stdbool.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool serial_log = false;

Serialport * signal_serialport;

Serialport* Serialport_Create(){

	Serialport * serialport = (Serialport*)malloc(sizeof(Serialport));

	serialport->mFileDescriptor = 0;
	serialport->mOldPortSettings = (termios*)malloc(sizeof(termios));
	serialport->isOpen = false;
	serialport->mInputBuffer = (char*)malloc(INPUTBUFFER_SIZE);
	serialport->mInputBuffer_put = serialport->mInputBuffer_get = serialport->mInputBuffer;

	serialport->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(serialport->mutex, NULL);

	return serialport;
}

void Serialport_Destroy(Serialport* serialport){
	pthread_mutex_lock(serialport->mutex);
	free(serialport->mOldPortSettings);
	free(serialport->mInputBuffer);
	free(serialport->mutex);
	free(serialport);
}

void Serialport_Open_Default(Serialport * serialport, const char * 	name){
	/*
	 * Throw an exception if the port is already open.
	 */
	pthread_mutex_lock(serialport->mutex);
	if ( serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port already open.");
		return;
	}
	/*
	 * Try to open the serial port and throw an exception if we are
	 * not able to open it.
	 *
	 * :FIXME: Exception thrown by this method after opening the
	 * serial port might leave the port open even though mIsOpen
	 * is false. We need to close the port before throwing an
	 * exception or close it next time this method is called before
	 * calling open() again.
	 */
	serialport->mFileDescriptor = open( name, O_RDWR | O_NOCTTY | O_NONBLOCK ) ;
	if (serialport->mFileDescriptor < 0 ){
		if(serial_log) printf("\nERROR: Cannot open serial port.");
		return;
	}

	signal_serialport = serialport;
	signal( SIGIO, Serialport_Signal);

	/*
	 * Direct all SIGIO and SIGURG signals for the port to the current
	 * process.
	 */
	if ( fcntl( serialport->mFileDescriptor, F_SETOWN,getpid() ) < 0 ) {
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}

	/*
	 * Enable asynchronous I/O with the serial port.
	 */
	if ( fcntl( serialport->mFileDescriptor,F_SETFL, FASYNC ) < 0 ) {
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}

	/*
	 * Save the current settings of the serial port so they can be
	 * restored when the serial port is closed.
	 */
	if ( tcgetattr( serialport->mFileDescriptor,serialport->mOldPortSettings ) < 0 ) {
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}

	//
	// Start assembling the new port settings.
	//

	termios port_settings ;
	bzero( &port_settings, sizeof( port_settings ) ) ;

	//
	// Enable the receiver (CREAD) and ignore modem control lines
	// (CLOCAL).
	//
	port_settings.c_cflag |= CREAD | CLOCAL ;

	//
	// Set the VMIN and VTIME parameters to zero by default. VMIN is
	// the minimum number of characters for non-canonical read and
	// VTIME is the timeout in deciseconds for non-canonical
	// read. Setting both of these parameters to zero implies that a
	// read will return immediately only giving the currently
	// available characters.
	//
	port_settings.c_cc[ VMIN  ] = 0 ;
	port_settings.c_cc[ VTIME ] = 0 ;
	/*
	 * Flush the input buffer associated with the port.
	 */
	if ( tcflush( serialport->mFileDescriptor, TCIFLUSH ) < 0 ) {
		if(serial_log) printf("\nERROR: Cannot flush serial port.");
		return;
	}

	/*
	 * Write the new settings to the port.
	 */
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 )  {
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}

	/*
	 * The serial port isif(serial_log) printfat this point.
	 */
	serialport->isOpen = true ;
	pthread_mutex_unlock(serialport->mutex);
	return ;
}

void Serialport_Open(Serialport * serialport, const char * 	name,
		const BaudRate      baudRate, const CharacterSize charSize,
		const Parity        parityType, const StopBits      stopBits,
		const FlowControl   flowControl){

	Serialport_Open_Default(serialport,name);
	Serialport_SetBaudRate(serialport,baudRate);
	Serialport_SetCharSize(serialport,charSize);
	Serialport_SetParity(serialport,parityType);
	Serialport_SetNumOfStopBits(serialport,stopBits);
	Serialport_SetFlowControl(serialport,flowControl);
}

bool Serialport_IsOpen(Serialport * serialport){
	return serialport->isOpen;
}

void Serialport_Close(Serialport * serialport){
	//
	// Throw an exception if the serial port is not open.
	//
	pthread_mutex_lock(serialport->mutex);
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		pthread_mutex_unlock(serialport->mutex);
		return;
	}

	//TODO signal( SIGIO, SerialPort_Signal) ;

	//
	// Restore the old settings of the port.
	//
	tcsetattr( serialport->mFileDescriptor,TCSANOW, serialport->mOldPortSettings ) ;
	//
	// Close the serial port file descriptor.
	//
	close(serialport->mFileDescriptor) ;
	//
	// The port is not open anymore.
	//
	serialport->isOpen = false ;
	//
	pthread_mutex_unlock(serialport->mutex);
	return ;
}

void Serialport_SetBaudRate(Serialport * serialport, BaudRate baudRate ){

	//
	// Throw an exception if the serial port is not open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Get the current settings of the serial port.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}

	pthread_mutex_lock(serialport->mutex);
	//
	// Set the baud rate for both input and output.
	//
	if ( ( cfsetispeed( &port_settings,baudRate ) < 0 ) ||
		 ( cfsetospeed( &port_settings,baudRate ) < 0 ) ){
		//
		// If any of the settings fail, we abandon this method.
		//
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		pthread_mutex_unlock(serialport->mutex);
		return;
	}

	//
	// Set the new attributes of the serial port.
	//
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 )	{
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		pthread_mutex_unlock(serialport->mutex);
		return;
	}
	pthread_mutex_unlock(serialport->mutex);
	return ;
}

BaudRate Serialport_GetBaudRate(Serialport * serialport){
	 //
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}
	//
	// Read the current serial port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return -1;
	}
	//
	// Obtain the input baud rate from the current settings.
	//
	return cfgetispeed( &port_settings ); //TODO ver se é igual
}

void Serialport_SetCharSize( Serialport * serialport,CharacterSize charSize ){

	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Get the current settings of the serial port.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	pthread_mutex_lock(serialport->mutex);
	//
	// Set the character size.
	//
	port_settings.c_cflag &= ~CSIZE ;
	port_settings.c_cflag |= charSize ;
	//
	// Apply the modified settings.
	//
	pthread_mutex_unlock(serialport->mutex);
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	return ;
}

CharacterSize Serialport_GetCharSize(Serialport * serialport){
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return -1;
	}
	//
	// Read the character size from the setttings.
	//
	return port_settings.c_cflag & CSIZE; //TODO ver se é igual
}

void Serialport_SetParity( Serialport * serialport,Parity parityType ){

	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 )	{
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	pthread_mutex_lock(serialport->mutex);
	//
	// Set the parity type depending on the specified parameter.
	//
	switch( parityType )
	{
	case PARITY_EVEN:
		port_settings.c_cflag |= PARENB ;
		port_settings.c_cflag &= ~PARODD ;
		port_settings.c_iflag |= INPCK ;
		break ;
	case PARITY_ODD:
		port_settings.c_cflag |= ( PARENB | PARODD ) ;
		port_settings.c_iflag |= INPCK ;
		break ;
	case PARITY_NONE:
		port_settings.c_cflag &= ~(PARENB) ;
		port_settings.c_iflag |= IGNPAR ;
		break ;
	default:
		if(serial_log) printf("ERROR: Invalid parity.\n");
		pthread_mutex_unlock(serialport->mutex);
		return;
		break ;
	}
	//
	// Apply the modified port settings.
	//
	pthread_mutex_unlock(serialport->mutex);
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	return ;
}

Parity Serialport_GetParity(Serialport * serialport){
	 //
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return -1;
	}
	//
	// Get the parity type from the current settings.
	//
	if ( port_settings.c_cflag & PARENB )
	{
		//
		// Parity is enabled. Lets check if it is odd or even.
		//
		if ( port_settings.c_cflag & PARODD )
		{
			return PARITY_ODD ;
		}
		else
		{
			return PARITY_EVEN ;
		}
	}
	//
	// Parity is disabled.
	//
	return PARITY_NONE ;
}

void Serialport_SetNumOfStopBits( Serialport * serialport,StopBits numOfStopBits ){

	 //
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	pthread_mutex_lock(serialport->mutex);
	//
	// Set the number of stop bits.
	//
	switch( numOfStopBits )
	{
	case STOP_BITS_1:
		port_settings.c_cflag &= ~(CSTOPB) ;
		break ;
	case STOP_BITS_2:
		port_settings.c_cflag |= CSTOPB ;
		break ;
	default:
		if(serial_log) printf("\nERROR: Invalid number of stop bits.");
		pthread_mutex_unlock(serialport->mutex);
		return;
		break ;
	}
	//
	// Apply the modified settings.
	//
	pthread_mutex_unlock(serialport->mutex);
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 )	{
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	return ;
}

StopBits Serialport_GetNumOfStopBits(Serialport * serialport){
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return -1;
	}
	//
	// If CSTOPB is set then we are using two stop bits, otherwise we
	// are using 1 stop bit.
	//
	if ( port_settings.c_cflag & CSTOPB )
		return STOP_BITS_2 ;

	return STOP_BITS_1 ;
}

void Serialport_SetFlowControl( Serialport * serialport,const FlowControl   flowControl ){
	 //
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	pthread_mutex_lock(serialport->mutex);
	//
	// Set the flow control.
	//
	switch( flowControl )
	{
	case FLOW_CONTROL_HARD:
		port_settings.c_cflag |= CRTSCTS ;
		break ;
	case FLOW_CONTROL_NONE:
		port_settings.c_cflag &= ~(CRTSCTS) ;
		break ;
	default:
		if(serial_log) printf("\nERROR: Invalid Flow Control.");
		pthread_mutex_unlock(serialport->mutex);
		return;
		break ;
	}
	//
	// Apply the modified settings.
	//
	pthread_mutex_unlock(serialport->mutex);
	if ( tcsetattr( serialport->mFileDescriptor,TCSANOW,&port_settings ) < 0 )	{
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return;
	}
	return ;
}

FlowControl Serialport_GetFlowControl(Serialport * serialport) {
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen ){
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}
	//
	// Get the current port settings.
	//
	termios port_settings ;
	if ( tcgetattr( serialport->mFileDescriptor,&port_settings ) < 0 ){
		if(serial_log) printf("\nERROR: Cannot configure serial port.");
		return -1;
	}
	//
	// If CRTSCTS is set then we are using hardware flow
	// control. Otherwise, we are not using any flow control.
	//
	if ( port_settings.c_cflag & CRTSCTS )
	{
		return FLOW_CONTROL_HARD ;
	}
	return FLOW_CONTROL_NONE ;
}

bool Serialport_IsDataAvailable(Serialport * serialport){
	 //
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return false;
	}
	//
	// Check if any data is available in the input buffer.
	//
	return ( serialport->mInputBuffer_get != serialport->mInputBuffer_put ? true : false ) ;
}

static struct timeval timeval_sub( const struct timeval firstOperand, const struct timeval secondOperand ){
	 /*
	  * This implementation may result in undefined behavior if the
	  * platform uses unsigned values for storing tv_sec and tv_usec
	  * members of struct timeval.
	  */
	 //
	 // Number of microseconds in a second.
	 //
	 const int MICROSECONDS_PER_SECOND = 1000000 ;
	 struct timeval result ;
	 //
	 // Take the difference of individual members of the two operands.
	 //
	 result.tv_sec  = firstOperand.tv_sec - secondOperand.tv_sec ;
	 result.tv_usec = firstOperand.tv_usec - secondOperand.tv_usec ;
	 //
	 // If abs(result.tv_usec) is larger than MICROSECONDS_PER_SECOND,
	 // then increment/decrement result.tv_sec accordingly.
	 //
	 if ( abs( result.tv_usec ) > MICROSECONDS_PER_SECOND )
	 {
		 int num_of_seconds = (result.tv_usec / MICROSECONDS_PER_SECOND ) ;
		 result.tv_sec  += num_of_seconds ;
		 result.tv_usec -= ( MICROSECONDS_PER_SECOND * num_of_seconds ) ;
	 }
	 return result ;
}

static unsigned char sReadByte(Serialport * serialport,	const unsigned int msTimeout){

	pthread_mutex_lock(serialport->mutex);
	//
	// Get the current time. Throw an exception if we are unable
	// to read the current time.
	//
	struct timeval entry_time ;
	if ( gettimeofday( &entry_time, NULL ) < 0 ){
		if(serial_log) printf("\nERROR: %s.", (char*)strerror( errno ));
		pthread_mutex_unlock(serialport->mutex);
		return -1;
	}
	//
	// Wait for data to be available.
	//
	const int MICROSECONDS_PER_MS  = 1000 ;
	const int MILLISECONDS_PER_SEC = 1000 ;
	//
	struct timeval curr_time, elapsed_time ;
	unsigned int elapsed_ms;

	while( serialport->mInputBuffer_get-serialport->mInputBuffer_put == 0 ){
		//
		// Read the current time.
		//

		if ( gettimeofday( &curr_time,NULL ) < 0 ){
			if(serial_log) printf("\nERROR: %s.",(char*)strerror( errno ));
			pthread_mutex_unlock(serialport->mutex);
			return -1;
		}
		//
		// Obtain the elapsed time.
		//
		elapsed_time = timeval_sub(curr_time,entry_time);
		//
		// Increase the elapsed number of milliseconds.
		//
		elapsed_ms = ( elapsed_time.tv_sec  * MILLISECONDS_PER_SEC +
						   elapsed_time.tv_usec / MICROSECONDS_PER_MS ) ;
		//
		// If more than msTimeout milliseconds have elapsed while
		// waiting for data, then we throw a ReadTimeout exception.
		//
		if ( ( msTimeout > 0 ) && ( elapsed_ms > msTimeout ) ){
			//if(serial_log) printf("\nTimeout."); //todo print bom para debug
			pthread_mutex_unlock(serialport->mutex);
			return -1;
		}
		//
		// Wait for 1ms (1000us) for data to arrive.
		//
		pthread_mutex_unlock(serialport->mutex);
		usleep( MICROSECONDS_PER_MS ) ;
		pthread_mutex_lock(serialport->mutex);
	}
	//
	// Return the first byte and remove it from the queue.
	//
	unsigned char next_char = *(serialport->mInputBuffer_get++);

	if(serialport->mInputBuffer_get==serialport->mInputBuffer+INPUTBUFFER_SIZE)
		serialport->mInputBuffer_get = serialport->mInputBuffer;

	if(next_char>32 && next_char<126){
		if(serial_log) printf("R: %x (%d) (%c)\r\n",next_char,next_char,next_char);
	}else
		if(serial_log) printf("R: %x (%d)\r\n",next_char,next_char);

	//if(serial_log) printf("%c",next_char);
	pthread_mutex_unlock(serialport->mutex);
	return next_char ;
}

unsigned char Serialport_ReadByte( Serialport * serialport,	const unsigned int msTimeout){
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}

	return sReadByte(serialport,msTimeout);
}

unsigned int Serialport_Read( Serialport * serialport, char* dataBuffer,
	  const unsigned int numOfBytes,
	  const unsigned int msTimeout){
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return -1;
	}

	unsigned int i;
	if ( numOfBytes == 0 )
		return -1;
	else{
		for(i=0; i<numOfBytes; ++i)
			*(dataBuffer++) = sReadByte(serialport, msTimeout) ;
	}
	if(*(dataBuffer-1)==-1)return -1;
	return i;
}

static void sWrite( Serialport * serialport, char* dataBuffer,
		const unsigned int   bufferSize ){

    //
    // Make sure that the serial port is open.
    //
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
    //
    // Write the data to the serial port. Keep retrying if EAGAIN
    // error is received.
    //
	pthread_mutex_lock(serialport->mutex);
    int num_of_bytes_written = -1 ;
    do
    {
        num_of_bytes_written = write( serialport->mFileDescriptor,
                                      dataBuffer,
                                      bufferSize ) ;
    }
    while ( ( num_of_bytes_written < 0 ) &&
            ( EAGAIN == errno ) ) ;
    //
    if ( num_of_bytes_written < 0 ){
		if(serial_log) printf("\nERROR: Error writing.");
    }
    //
    // :FIXME: What happens if num_of_bytes_written < bufferSize ?
    //
    pthread_mutex_unlock(serialport->mutex);

	int i = bufferSize;
	while(i>0){
		if(dataBuffer[bufferSize-i]>32 && dataBuffer[bufferSize-i]<126){
			if(serial_log) printf("W: %x (%d) (%c)\r\n",dataBuffer[bufferSize-i],dataBuffer[bufferSize-i],dataBuffer[bufferSize-i]);
		}else
			if(serial_log) printf("W: %x (%d) \r\n",dataBuffer[bufferSize-i],dataBuffer[bufferSize-i]);
		//if(serial_log) printf("%c",dataBuffer[bufferSize-i]);
		i--;
	}

    return ;
}

void Serialport_WriteByte(Serialport * serialport,const unsigned char dataByte){
	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
	//
	// Write the byte to the serial port.
	//
	sWrite(serialport,(char*)&dataByte,1);
	return ;
}

void Serialport_Write(Serialport * serialport,char* dataBuffer,
		const unsigned int numOfBytes){

	//
	// Make sure that the serial port is open.
	//
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}

	//
	// Write to the serial port.
	//
	sWrite(serialport,dataBuffer,numOfBytes);
	return ;

}





inline void SetModemControlLine(Serialport * serialport, const int  modemLine,
		const bool lineState ){
    //
    // Make sure that the serial port is open.
    //
	if ( ! serialport->isOpen )	{
		if(serial_log) printf("\nERROR: Serial port is not open.");
		return;
	}
    //
    // :TODO: Check to make sure that modemLine is a valid value.
    //
    // Set or unset the specified bit according to the value of
    // lineState.
    //
	pthread_mutex_lock(serialport->mutex);
    int ioctl_result = -1 ;
    if ( true == lineState )
    {
        int set_line_mask = modemLine ;
        ioctl_result = ioctl( serialport->mFileDescriptor,
                              TIOCMBIS,
                              &set_line_mask ) ;
    }
    else
    {
        int reset_line_mask = modemLine ;
        ioctl_result = ioctl( serialport->mFileDescriptor,
                              TIOCMBIC,
                              &reset_line_mask ) ;
    }
    //
    // Check for errors.
    //
    if ( -1 == ioctl_result ) {
    	if(serial_log) printf("\nERROR: Cannot configure serial port.");
    }
	pthread_mutex_unlock(serialport->mutex);
    return ;
}

inline bool GetModemControlLine(Serialport * serialport, const int modemLine ){
    //
    // Make sure that the serial port is open.
    //
	if ( ! serialport->isOpen )	{
			if(serial_log) printf("\nERROR: Serial port is not open.");
			return -1;
		}
    //
    // Use an ioctl() call to get the state of the line.
    //
    int serial_port_state = 0 ;
    if ( -1 == ioctl( serialport->mFileDescriptor,
                      TIOCMGET,
                      &serial_port_state ) )
    {
    	printf("\nERROR: Cannot configure serial port.");
    	return -1;
    }
    //
    // :TODO: Verify that modemLine is a valid value.
    //
    return ( serial_port_state & modemLine ) ;
}

void Serialport_SetDtr( Serialport * serialport,const bool dtrState){
	SetModemControlLine(serialport, TIOCM_DTR,  dtrState ) ;
}

bool Serialport_GetDtr(Serialport * serialport){
	return GetModemControlLine(serialport, TIOCM_DTR ) ;
}

void Serialport_SetRts( Serialport * serialport,const bool rtsState){
	SetModemControlLine(serialport, TIOCM_RTS, rtsState ) ;
}

bool Serialport_GetRts(Serialport * serialport){
	return GetModemControlLine(serialport, TIOCM_RTS ) ;
}

bool Serialport_GetCts(Serialport * serialport){
	return GetModemControlLine(serialport, TIOCM_CTS ) ;
}

bool Serialport_GetDsr(Serialport * serialport){
	return GetModemControlLine(serialport, TIOCM_DSR ) ;
}

void Serialport_Signal(int signalNumber){
    //
    // We only want to deal with SIGIO signals here.
    //
    if ( SIGIO != signalNumber ){
        return ;
    }
    //
    // Check if any data is available at the specified file
    // descriptor.
    //
    int num_of_bytes_available = 0 ;
    if (ioctl( signal_serialport->mFileDescriptor ,FIONREAD, &num_of_bytes_available ) < 0 ){
        /*
         * Ignore any errors and return immediately.
         */
        return ;
    }
    //
    // If data is available, read all available data and shove
    // it into the corresponding input buffer.
    //
    int i;
    for(i=0; i<num_of_bytes_available; ++i) {
        unsigned char next_byte ;
        if ( read( signal_serialport->mFileDescriptor, &next_byte,1 ) > 0 ) {
            *(signal_serialport->mInputBuffer_put++) = next_byte;


            if(signal_serialport->mInputBuffer_put==signal_serialport->mInputBuffer+INPUTBUFFER_SIZE)
            	signal_serialport->mInputBuffer_put = signal_serialport->mInputBuffer;

            if(signal_serialport->mInputBuffer_put==signal_serialport->mInputBuffer_get){
            	signal_serialport->mInputBuffer_get++;
            	if(signal_serialport->mInputBuffer_get==signal_serialport->mInputBuffer+INPUTBUFFER_SIZE)
            		signal_serialport->mInputBuffer_get = signal_serialport->mInputBuffer;
            }
        }
        else
            break ;
    }
    return ;
}
