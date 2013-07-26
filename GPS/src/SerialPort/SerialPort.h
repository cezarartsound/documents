/*
 * serialport.h
 *
 *  Created on: 22 de Dez de 2011
 *      Author: fabio32883
 */

#ifndef Serialport_H_
#define Serialport_H_

#include <termios.h>
#include <pthread.h>
#include <stdbool.h>

extern bool serial_log;


    /**
     * The allowed set of baud rates.
     */
    typedef enum _BaudRate {
        BAUD_50      = B50,
        BAUD_75      = B75,
        BAUD_110     = B110,
        BAUD_134     = B134,
        BAUD_150     = B150,
        BAUD_200     = B200,
        BAUD_300     = B300,
        BAUD_600     = B600,
        BAUD_1200    = B1200,
        BAUD_1800    = B1800,
        BAUD_2400    = B2400,
        BAUD_4800    = B4800,
        BAUD_9600    = B9600,
        BAUD_19200   = B19200,
        BAUD_38400   = B38400,
        BAUD_57600   = B57600,
        BAUD_115200  = B115200,
        BAUD_230400  = B230400,
        BAUD_460800 = B460800,
        BAUD_500000 = B500000,
        BAUD_576000 = B576000,
        BAUD_921600 = B921600,
        BAUD_1000000 = B1000000,
        BAUD_1152000 = B1152000,
        BAUD_1500000 = B1500000,
        BAUD_2000000 = B2000000,
        BAUD_2500000 = B2500000,
        BAUD_3000000 = B3000000,
        BAUD_3500000 = B3500000,
        BAUD_4000000 = B4000000,
        BAUD_DEFAULT = BAUD_57600
    } BaudRate;

    typedef enum _CharacterSize {
        CHAR_SIZE_5  = CS5, //!< 5 bit characters.
        CHAR_SIZE_6  = CS6, //!< 6 bit characters.
        CHAR_SIZE_7  = CS7, //!< 7 bit characters.
        CHAR_SIZE_8  = CS8, //!< 8 bit characters.
        CHAR_SIZE_DEFAULT = CHAR_SIZE_8
    } CharacterSize;

    typedef enum _StopBits {
        STOP_BITS_1,   //! 1 stop bit.
        STOP_BITS_2,   //! 2 stop bits.
        STOP_BITS_DEFAULT = STOP_BITS_1
    } StopBits;

    typedef enum _Parity {
        PARITY_EVEN,     //!< Even parity.
        PARITY_ODD,      //!< Odd parity.
        PARITY_NONE,     //!< No parity i.e. parity checking disabled.
        PARITY_DEFAULT = PARITY_NONE
    } Parity;

    typedef enum _FlowControl {
        FLOW_CONTROL_HARD,
        FLOW_CONTROL_SOFT,
        FLOW_CONTROL_NONE,
        FLOW_CONTROL_DEFAULT = FLOW_CONTROL_NONE
    } FlowControl;

    typedef struct termios termios;


	#define INPUTBUFFER_SIZE 1024

    typedef struct _Serialport {
    	int 		  mFileDescriptor;
		bool		  isOpen;
		termios* 	  mOldPortSettings;
		char * 		  mInputBuffer;
		char * 		  mInputBuffer_get;
		char * 		  mInputBuffer_put;
		pthread_mutex_t* mutex;
	} Serialport;



	Serialport * Serialport_Create();

	void Serialport_Destroy(Serialport* serialport);


    /**
	* Open the serial port with the specified settings. A serial port
	* cannot be used till it is open.
	*/

	void Serialport_Open(Serialport * serialport, const char * 	name,
				const BaudRate      baudRate, const CharacterSize charSize,
				const Parity        parityType, const StopBits      stopBits,
				const FlowControl   flowControl);

	void Serialport_Open_Default(Serialport * serialport, const char * 	name);

    /**
	 * Check if the serial port is open for I/O.
	 */
    bool Serialport_IsOpen(Serialport * serialport);

    /**
     * Close the serial port. All settings of the serial port will be
     * lost and no more I/O can be performed on the serial port.
     */
    void Serialport_Close(Serialport * serialport);

    /**
     * Set the baud rate for the serial port to the specified value
     * (baudRate).
     */
    void Serialport_SetBaudRate(Serialport * serialport, BaudRate baudRate );

    /**
     * Get the current baud rate for the serial port.
     *
     */
    BaudRate Serialport_GetBaudRate(Serialport * serialport);

    /**
     * Set the character size for the serial port.
     */
    void Serialport_SetCharSize( Serialport * serialport,CharacterSize charSize ) ;

    /**
     * Get the current character size for the serial port.
     */
    CharacterSize Serialport_GetCharSize(Serialport * serialport);

    /**
     * Set the parity type for the serial port.
     *
     */
    void Serialport_SetParity( Serialport * serialport,Parity parityType );

    /**
     * Get the parity type for the serial port.
     *
     */
    Parity Serialport_GetParity(Serialport * serialport);

    /**
     * Set the number of stop bits to be used with the serial port.
     *
     */

    void Serialport_SetNumOfStopBits( Serialport * serialport,StopBits numOfStopBits );

    /**
     * Get the number of stop bits currently being used by the serial
     * port.
     *
     */
    StopBits Serialport_GetNumOfStopBits(Serialport * serialport);

    /**
     * Set flow control.
     *
     */
    void Serialport_SetFlowControl( Serialport * serialport,
    		const FlowControl   flowControl );

    /**
     * Get the current flow control setting.
     *
     */
    FlowControl Serialport_GetFlowControl(Serialport * serialport) ;

    /**
     * Check if data is available at the input of the serial port.
     */
    bool Serialport_IsDataAvailable(Serialport * serialport);

    /**
     * Read a single byte from the serial port. If no data is
     * available in the specified number of milliseconds (msTimeout),
     * then this method will throw ReadTimeout exception. If msTimeout
     * is 0, then this method will block till data is available.
     */
    unsigned char Serialport_ReadByte( Serialport * serialport,
    		const unsigned int msTimeout);

    /**
     * Read the specified number of bytes from the serial port. The
     * method will timeout if no data is received in the specified
     * number of milliseconds (msTimeout). If msTimeout is 0, then
     * this method will block till all requested bytes are
     * received. If numOfBytes is zero, then this method will keep
     * reading data till no more data is available at the serial
     * port. In all cases, all read data is available in dataBuffer on
     * return from this method.
     */
    unsigned int Serialport_Read( Serialport * serialport, char* dataBuffer,
          const unsigned int numOfBytes,
          const unsigned int msTimeout);

    /**
     * Send a single byte to the serial port.
     */
    void Serialport_WriteByte(Serialport * serialport,const unsigned char dataByte);

    /**
     * Write the data from the specified vector to the serial port.
     */
    void Serialport_Write(Serialport * serialport,char* dataBuffer,
    		const unsigned int numOfBytes);

    /**
     * Set the DTR line to the specified value. (Data Terminal Ready)
     */
    void Serialport_SetDtr( Serialport * serialport,const bool dtrState);

    /**
     * Get the status of the DTR line.
     */
    bool Serialport_GetDtr(Serialport * serialport);

    /**
     * Set the RTS line to the specified value. (Request to Send)
     */
    void Serialport_SetRts( Serialport * serialport,const bool rtsState);

    /**
     * Get the status of the RTS line.
     */
    bool Serialport_GetRts(Serialport * serialport);

    bool Serialport_GetCts(Serialport * serialport);

    bool Serialport_GetDsr(Serialport * serialport);

    void Serialport_Signal(int signalNumber );

#endif /* serialport_H_ */
