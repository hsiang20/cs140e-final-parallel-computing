// #include "_uint8_t.h"

// #include "libunix.h"
#include "rpi.h"
#include "sw-uart.h"

// assume data type only allows "uint8_t"
typedef int8_t data_type;

#define TX_ASYNC 18
#define RX_ASYNC 19
#define BAUD_RATE 115200

#define SEND_SIGNAL 5
#define SYNC_SIGNAL 7

#define DELAY_MS 20
#define TIMEOUT_MS 100

typedef struct {
    uint8_t command;       
    uint32_t address;      
    uint32_t data;         
} Packet;


void FMPI_Init_async(int rank, int size, int root);
void send_async(void *buffer, int count);
void recv_async(void *buffer, int count);
void send_signal_async(uint8_t signal);
void wait_signal_async(uint8_t signal);
int wait_signal_timeout_async(uint8_t signal, uint32_t msec);
void sync_me_last();
void sync_receiver();
void FMPI_PUT(uint32_t *buffer, uint32_t *address, int count);
