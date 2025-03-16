// #include "_uint8_t.h"

// #include "libunix.h"
#include "rpi.h"
#include "sw-uart.h"

// assume data type only allows "uint8_t"
typedef int8_t data_type;

#define MAX_RPIS 4

#define TX_ASYNC 12
#define RX_ASYNC 13
#define BAUD_RATE 115200

#define SEND_SIGNAL 5
#define SYNC_SIGNAL 7

#define DELAY_MS 20
#define TIMEOUT_MS 100


// command: 1 bytes (PUT: 1, GET: 2)
// address: 4 bytes
// data: 4 bytes
typedef struct __attribute__((packed, scalar_storage_order("big-endian"))) {
    uint8_t command;       
    uint32_t address;      
    uint32_t data;         
} Packet;


void FMPI_Init_async(int rank, int size, int root);
void send_async(void *buffer, int recv_pi, int count, int data_size);
void recv_async(uint8_t *buffer, int send_pi, int count, int data_size);
void send_signal_async(int recv_pi, uint8_t signal);
void wait_signal_async(int send_pi, uint8_t signal);
int wait_signal_timeout_async(int recv_pi, uint8_t signal, uint32_t msec);
void sync_me_last(int recv_pi);
int sync_receiver(int send_pi);
void FMPI_PUT(int recv_pi, uint32_t *buffer, uint32_t *address);
uint32_t FMPI_GET(int recv_pi, uint32_t *address);