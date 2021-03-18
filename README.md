# OS-2021-1
## 1. What is operating system?
### Computer
정보를 처리하는 기계
### A stored-program computer(von Neumann architecture)
메모리에 program(set of instructions)을 저장하고 CPU에서 순차적으로 처리(fetch-execute)
### OS
컴퓨터 시스템을 운영하는 소프트웨어   
컴퓨터에서 항상 실행되고 있는 프로그램   
OS kernel이 기능의 핵심   
하드웨어와 application program, user간의 interface   
- User interface - CLI(shell), GUI
- Application programming interface - system call

Application program에게 system service 제공   
User가 시스템에 대해 알지 못해도 됨   
Ex) program execution, I/O operation, memory management, file system..
### Multiprogramming
Memory에 여러 개의 process을 올려놓음
### Multiprocessing
CPU가 multiprogramming된 processes들을 동시에 처리
### Time sharing
다른 process로 CPU core를 매우 빠르게 교체하여 동시에 처리하는 것처럼 보이게 함
### CPU scheduling
CPU가 준비 상태에 있는 process 중 어떤 것을 실행시킬지 선택
### Dual mode operation
user mode & kernel mode   
Kernel mode에서만 하드웨어를 직접적으로 제어할 수 있게 함으로써 프로그램의 잘못된 동작 방지   
