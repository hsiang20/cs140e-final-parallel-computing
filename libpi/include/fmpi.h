// #include "_uint8_t.h"

// #include "libunix.h"
#include "rpi.h"
#include "sw-uart.h"

// assume data type only allows "uint8_t"
// typedef int8_t data_type;

#define MAX_RPIS 4

#define BASE_TX 16
#define BASE_RX 17
#define BAUD_RATE (115200 * 4)
// #define BAUD_RATE 460800

#define SEND_SIGNAL 5
#define SYNC_SIGNAL 7
#define GO_SIGNAL 11

#define DELAY_MS 70
#define TIMEOUT_MS 200

void send(void *buffer, int recv_pi, int count, int data_size);
void recv(void *buffer, int send_pi, int count, int data_size);

void send_signal(int recv_pi, uint8_t signal);
void wait_signal(int send_pi, uint8_t signal);
int wait_signal_timeout(int recv_pi, uint8_t signal, uint32_t msec);

void sync_root_first();
void sync_root_last();

void FMPI_Init(uint8_t *rank, uint8_t *size, int root);

void FMPI_Bcast(void *buffer, int count, int data_size);

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount, int data_size);

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount, int data_size);
