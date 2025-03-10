// #include "_uint8_t.h"

// #include "libunix.h"
#include "rpi.h"
#include "sw-uart.h"

// assume data type only allows "uint8_t"
typedef uint8_t data_type;

#define TX 16
#define RX 17
#define BAUD_RATE 115200

#define SEND_SIGNAL 5
#define SYNC_SIGNAL 7

#define DELAY_MS 100

void send(void *buffer, int count);
void recv(void *buffer, int count);

void send_signal(uint8_t signal);
void wait_signal(uint8_t signal);

void sync_root_first();
void sync_root_last();

void FMPI_Init(int rank, int size, int root);

void FMPI_Bcast(void *buffer, int count);

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount);

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount);