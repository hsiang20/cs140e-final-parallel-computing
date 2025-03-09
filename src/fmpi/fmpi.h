#include "sw-uart.h"

// assume data type only allows "uint8_t"
typedef uint8_t data_type;

#define SEND_SIGNAL 5

void send_bcast(void *buffer, int count);
void recv_bcast(void *buffer, int count);

void send_signal();
void wait_for_signal_from_root();

void FMPI_Init();

void FMPI_Bcast(void *buffer, int count, int root, int rank);

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount,
                    int root, int rank, int size);

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount,
                    int root, int rank, int size);