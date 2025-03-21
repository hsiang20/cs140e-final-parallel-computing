#include "fmpi.h"

static sw_uart_t u[MAX_RPIS], u_async;
static int _rank, _size, _root;



void send(void *buffer, int recv_pi, int count, int data_size) {
    for (int i = 0; i < count * data_size; i++) {
        sw_uart_put8(&u[recv_pi], ((char *)buffer)[i]);
    }
}

void recv(void *buffer, int send_pi, int count, int data_size) {
    for (int i = 0; i < count * data_size; i++) {
        int val = sw_uart_get8(&u[send_pi]);
        ((char *)buffer)[i] = val;
    }
}

void send_signal(int recv_pi, uint8_t signal) {
    sw_uart_put8(&u[recv_pi], signal);
}

void wait_signal(int send_pi, uint8_t signal) {
    while (sw_uart_get8(&u[send_pi]) != signal)
        ;
}

int wait_signal_timeout(int recv_pi, uint8_t signal, uint32_t msec) {
    return sw_uart_get8_timeout(&u[recv_pi], 1000*msec);
}

void sync_root_last() {
    // printk("%d in sync\n", _rank);
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            // printk("%d\n", i);
            if (i == _root) continue;
            do {
                send_signal(i, SYNC_SIGNAL);
            } while (wait_signal_timeout(i, SYNC_SIGNAL, TIMEOUT_MS) != SYNC_SIGNAL);
        }
        // after all ready
        // for (int i = 0; i < _size; i++) {
        //     if (i == _root) continue;
        //     do {
        //         send_signal(i, GO_SIGNAL);
        //     } while (wait_signal_timeout(i, GO_SIGNAL, TIMEOUT_MS) != GO_SIGNAL);
        // }
    } else {
        wait_signal(_root, SYNC_SIGNAL);
        // printk("%d got SYNC_SIGNAL\n", _rank);
        delay_ms(DELAY_MS);
        send_signal(_root, SYNC_SIGNAL);
        
        // printk("%d ready\n", _rank);
        // wait for all ready signal
        // wait_signal(_root, GO_SIGNAL);
        // printk("%d got GO_SIGNAL\n", _rank);
        // delay_ms(DELAY_MS);
        // send_signal(_root, GO_SIGNAL);
    }
    // printk("%d out of sync\n", _rank);
}

void FMPI_Init(uint8_t *rank, uint8_t *size, int root) {
    _rank = *(uint8_t *)(0x8000);
    _size = *(uint8_t *)(0x8000 + 1);
    assert(_size <= MAX_RPIS);
    *rank = _rank;
    *size = _size;
    _root = root;
    if (_rank == _root) {
        int idx = 0;
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            u[i] = sw_uart_init(BASE_TX+2*idx, BASE_RX+2*idx, BAUD_RATE);
            idx++;
        }
    } else {
        u[_root] = sw_uart_init(BASE_TX, BASE_RX, BAUD_RATE); 
    }
}

void FMPI_Bcast(void *buffer, int count, int data_size) {
    sync_root_last();
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            // delay_ms(DELAY_MS);
            // wait_signal(i, SEND_SIGNAL);
            // delay_ms(DELAY_MS);
            // send_signal(i, SEND_SIGNAL);
            // delay_ms(DELAY_MS);
            send(buffer, i, count, data_size);
        }
    } else {
        // do {
        //     send_signal(_root, SEND_SIGNAL);
        // } while (wait_signal_timeout(_root, SEND_SIGNAL, TIMEOUT_MS) != SEND_SIGNAL);
        recv(buffer, _root, count, data_size);
    }
}

void FMPI_Scatter(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount, int data_size) {
    // don't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    // printk("%d in Scatter\n", _rank);
    sync_root_last();
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            delay_ms(DELAY_MS);
            // wait_signal(i, SEND_SIGNAL);
            // delay_ms(DELAY_MS);
            // send_signal(i, SEND_SIGNAL);
            // delay_ms(DELAY_MS);
            send((char *)sendbuff + i * sendcount * data_size, i, sendcount, data_size);
        }
        memcpy(recvbuff, sendbuff, recvcount * data_size);
    } else {
        // do {
        //     send_signal(_root, SEND_SIGNAL);
        // } while (wait_signal_timeout(_root, SEND_SIGNAL, TIMEOUT_MS) != SEND_SIGNAL);
        recv(recvbuff, _root, recvcount, data_size);
    }
}

void FMPI_Gather(void *sendbuff, int sendcount,
                    void *recvbuff, int recvcount, int data_size) {
    // don't know diff betw sencount, recvcount
    // so assume they should be the same for now
    assert(sendcount == recvcount);
    sync_root_last();
    // delay_ms(DELAY_MS * 5);
    // printk("%d after gather sync\n", _rank);
    if (_rank == _root) {
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            // send_signal(i, SEND_SIGNAL);
            // do {
            //     send_signal(i, SEND_SIGNAL);
            // } while (wait_signal_timeout(i, SEND_SIGNAL, TIMEOUT_MS) != SEND_SIGNAL);    
            // wait_signal(i, SEND_SIGNAL);
            // delay_ms(DELAY_MS);
            send_signal(i, SEND_SIGNAL);
            recv((char *)recvbuff + i * sendcount * data_size, i, sendcount, data_size);
        }
        memcpy(recvbuff, sendbuff, recvcount * data_size);
    } else {
        // do {
        //     send_signal(_root, SEND_SIGNAL);
        // } while (wait_signal_timeout(_root, SEND_SIGNAL, TIMEOUT_MS) != SEND_SIGNAL);
        wait_signal(_root, SEND_SIGNAL);
        // delay_ms(DELAY_MS);
        // send_signal(_root, SEND_SIGNAL);
        delay_ms(DELAY_MS);
        send((char *)sendbuff, _root, sendcount, data_size);
    }
}

void FMPI_Barrier() {
    sync_root_last();
}