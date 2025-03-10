#include "fmpi.h"

static sw_uart_t u;
static int _rank, _size, _root;



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

void send_signal(uint8_t signal) {
    sw_uart_put8(&u, signal);
}

void wait_signal(uint8_t signal) {
    while (sw_uart_get8(&u) != signal)
        ;
}

void sync_root_last() {
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            send_signal(SYNC_SIGNAL);
            delay_ms(DELAY_MS);
            wait_signal(SYNC_SIGNAL);
        }
    } else {
        wait_signal(SYNC_SIGNAL);
        send_signal(SYNC_SIGNAL);
    }
}

void FMPI_Init(int rank, int size, int root) {
    u = sw_uart_init(TX, RX, BAUD_RATE);   
    _rank = rank;
    _size = size;
    _root = root;
}

void FMPI_Bcast(void *buffer, int count) {
    sync_root_last();
    if (_rank == _root) {
        send(buffer, count);
    } else {
        recv(buffer, count);
    }
}

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount) {
    // doesn't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    sync_root_last();
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            send((data_type *)sendbuff + i * sendcount, sendcount);
        }
        memcpy(recvbuff, sendbuff, recvcount * sizeof(data_type));
    } else {
        recv(recvbuff, recvcount);
    }
}

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount) {
    // doesn't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    sync_root_last();
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            send_signal(SEND_SIGNAL);
            recv((data_type *)sendbuff + i * sendcount, sendcount);
        }
        memcpy(recvbuff, sendbuff, recvcount * sizeof(data_type));
    } else {
        wait_signal(SEND_SIGNAL);
        send((data_type *)sendbuff, sendcount);
    }
}