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

---
## 2. Processes
### Process
실행 중인 프로그램   
HDD에 있는 프로그램이 메모리에 load된 상태
### Memory 영역
- text: 실행 가능한 코드   
- data: 전역 변수   
  - initialized data
  - uninitialized data
- heap: 동적 할당   
- stack: 함수 관련(parameter, return address, local variables)   
### Process states
- new: 생성된 상태(fork)
- ready: CPU에 할당될 준비가 완료된 상태
- running: 실행 중인 상태(CPU 점유), time interrupt 시 ready 상태로
- waiting: 아직 끝나지 않고 다른 일이 끝날 때 까지 기다리는 상태(ex. I/O completion)
- terminated: 실행 종료된 상태
### PCB(Process Control Block)
일종의 구조체  
Process에 관한 정보 포함 - process state, program counter(fetch할 memory 주소), CPU, memory..
### Multithreading
Process가 여러 개의 threads of execution를 가질 수 있게 함
### Context switch
context - PCB에 나타남   
Interrupt가 발생하면 현재 실행중인 process의 context를 저장하고, 다른 process로 CPU core switch(PCB정보를 CPU register에 load)
### Linux process operation
- fork() system call을 통해 process 생성
- child process는 parent process의 주소 공간을 복사해서 사용
- fork()의 return code가 0이면 child process
- child의 nonzero pid가 parent process로 return됨
### IPC(Inter-Process Communication)
동시에 실행되는 process에는 서로 데이터를 공유하지 않는 independent process와 서로 데이터를 공유하는 cooperating process가 존재   
데이터를 교환하기 위해선 IPC mechanism이 요구됨   
shared memory, message passing 두가지 모델이 존재
### Shared memory system
Producer-Consumer problem(producer는 정보 제공, consumer는 정보 소비)에서 producer는 buffer를 채우고, consumer는 buffer를 비우면서 하나의 buffer를 공유 -> 동시에 실행   
메모리 영역에 접근하는 코드를 application programmer가 작성해야함   
Ex) POSIX
### Message Passing
운영체제가 shared memory 관리(system call) - send(), receive()  
메세지를 주고 받는 communication link 필요
- direct communication   
정보를 주거나 받을 대상을 명시해야 함   
두 process간의 하나의 link가 생성됨
- indirect communication   
메세지가 port(mailbox)를 통해 전달됨   
port에 메세지를 넣고, 가져가는 방식   
두 개 이상의 process가 port 공유 가능
- synchronous(blocking) communication   
메세지를 다 보내지 못하거나 받지 못하면 process가 중단됨
- asynchronous(non-blocking) communication   
메세지를 다 보내지 못하거나 받지 못해도 process 그대로 진행   
정상적으로 전송됐는지, 받았는지 확인할 수 없지만 더 빠름   
Ex) Pipes
### Communication in Client-Server systems
서로 다른 PC의 process 간 통신   
(IPC - 하나의 PC 내의 process 간 통신)
- Socket   
pipe 형태로 원격의 두 컴퓨터 연결   
각 컴퓨터를 IP 주소로 특정하고 두 컴퓨터 간 pipe를 port로 특정(통신의 endpoints)   
Socket = IP:port
- RPC(Remote Procedure Call)   
네트워크로 연결된 시스템의 process 간 procedure call 추상화   
원격지의 함수 호출