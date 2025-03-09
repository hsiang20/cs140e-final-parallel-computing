#include "fmpi.h"

static sw_uart_t u;

void send(void *buffer, int count) {
    for (int i = 0; i < count; i++) {
        sw_uart_put8(&u, ((data_type *)buffer)[i]);
    }
}

void recv(void *buffer, int count) {
    for (int i = 0; i < count; i++) {
        int val = sw_uart_get8(&u);
        ((data_type *)buffer)[i] = val;
    }
}

void send_signal() {
    sw_uart_put8(&u, SEND_SIGNAL);
}

void wait_for_signal_from_root() {
    while (sw_uart_get8(&u) != SEND_SIGNAL)
        ;
}

void FMPI_Init() {
    u = sw_uart_init(16, 17, 115200);   
}

void FMPI_Bcast(void *buffer, int count, int root, int rank) {
    if (rank == root) {
        send(buffer, count);
    } else {
        recv(buffer, count);
    }
}

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount,
                    int root, int rank, int size) {
    // doesn't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i == root) continue;
            send((data_type *)sendbuff + i * sendcount, sendcount);
        }
        memcpy(recvbuff, sendbuff, recvcount * sizeof(data_type));
    } else {
        recv(recvbuff, recvcount);
    }
}

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount,
                    int root, int rank, int size) {
    // doesn't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    if (rank == root) {
        for (int i = 0; i < size; i++) {
            if (i == root) continue;
            send_signal();
            recv((data_type *)sendbuff + i * sendcount, sendcount);
        }
        memcpy(recvbuff, sendbuff, recvcount * sizeof(data_type));
    } else {
        wait_for_signal_from_root();
        send((data_type *)sendbuff, sendcount);
    }
}