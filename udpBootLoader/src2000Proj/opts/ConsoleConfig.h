#ifndef CONSOLE_CONFIG_H
#define CONSOLE_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o> Default console interface: <0=>close Console <1=>UART Console <2=>CAN Console <3=> RTT Console
// <4=> UDP Console 
// 	<i>Default: 0 
#define DEFAULT_CONSOLE 4


// <e> Allow UART console :
// </e>
#define ENABLE_UART_CONSOLE 0

// <e> Allow CAN console :
// </e>
#define ENABLE_CAN_CONSOLE 0

// <e> Allow UDP console :
// </e>
#define ENABLE_UDP_CONSOLE 1

// <e> Allow RTT console :
// </e>
#define ENABLE_RTT_CONSOLE 0

// <e> Use low weight printf :
// </e>
#define USE_MINI_PRINT 1

// <<< end of configuration section >>

#endif
//end of file
