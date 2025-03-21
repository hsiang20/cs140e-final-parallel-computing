# Open MPI for Distributed Systems
## Author
Chia-Hsiang Chang (hsiangc)
Jeng-Chi Weng (jengchi)

## Summary
We have developed a distributed system using multiple Raspberry Pis that can execute Open MPI code. The system supports an unlimited number of processors, enabling parallel computing through Open MPI calls.

## Setup
Compile the bootloader and set the resulting executable as an environment variable.
Connect the Raspberry Pis via wired connections. The root Pi transmits through GPIO 16–21, while other nodes use GPIO 16–17.
Adapt the Open MPI code using our provided API and run the system.

## Features
* Raspberry Pis communicate using software UART.
* Implements a custom message-passing interface, including:
    * Synchronous communication: Scatter, Broadcast, Gather
    * Asynchronous communication: PUT32, GET32
    * Utility functions: Init, Barrier
* Supports execution of real Open MPI code.
* Scalable to additional nodes; successfully tested with a four-Pi setup.

