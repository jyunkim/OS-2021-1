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
Memory에 여러 개의 process을 올려놓음(I/O operation동안 다른 process 실행)

### Multiprocessing
여러 개의 processor가 process를 병렬 처리

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
Interrupt가 발생하면 현재 실행중인 process의 context를 저장하고, 다른 process로 CPU core switch(PCB정보를 CPU register(PC)에 load)

### Linux process operation
- fork() system call을 통해 process 생성
- child process는 parent process의 주소 공간을 복사해서 사용
- fork()의 return code가 0이면 child process
- child의 nonzero pid가 parent process로 return됨

## Process간 통신
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

---
## 3. Thread & Concurrency
### Multithreading
Thread: light weight process
CPU가 실행하는 단위가 process -> thread  
Thread 별로 다른 program counter, register set, stack 정보를 가지고, 같은 code, data, file 정보를 가짐   
장점   
- Responsiveness   
Process가 다 실행되지 못하고 block 됐을 때, thread를 추가하고 실행을 계속할 수 있음
- Resource sharing   
Thread는 process의 resource를 공유하므로 통신에 용이
- Economy   
Process를 추가하는 것보다 thread를 추가하는게 경제적임(context switching-thread switching)
- Scalability   
Multiprocessor architecture를 활용하기 좋음

### Multicore system
- single core: time sharing을 통해 concurrent하게 실행
- multi core: 병렬 처리   
어떤 task를 나누고 어떻게 나눌 것인지가 중요

### Multithreading Models
- user thread: user mode에서 사용하는 thread - kernel support x
- kernel thread: kernel mode에서 사용하는 thread - 운영체제가 직접 관리

user thread - kernel thread -> 1:1, 1:M, M:N 관계 존재

### Implicit Threading
Concurrent & parallel application(multithreading in multicore system)을 개발하는게 쉽지 않음
-> compiler나 run-time library가 대신 하도록 함   
Ex) Thread Pool, Fork & Join, OpenMP, GCD

---
## 4. CPU Scheduling
### CPU Scheduling
목적   
- CPU utilization 향상
- Throughput 향상   
단위 시간 동안 완료한 process 수
- Turnaround time 최소화   
process 도착 ~ 완료 시간
- Waiting time 최소화   
process가 ready queue에서 대기하는 시간의 합
- Response time 최소화     
사용자 응답 시간

용도   
context switching을 통해 여러 개의 process를 concurrent하게 처리
- CPU burst time: CPU 사용 시간 -> running 상태
- I/O burst time: I/O 대기 시간 -> waiting, ready 상태   
=> 일반적으로 I/O burst time이 더 많음
- CPU-bound: CPU burst time이 많은 process
- I/O-bound: I/O burst time이 많은 process

CPU Scheduler가 memory에 있는 ready 상태의 process 중 CPU를 할당할 process 선택   
**How?**
- Preemptive(선점형)   
scheduler가 process를 교체할 수 있음
- Non-preemptive(비선점형)   
process가 스스로 끝날 때까지 유지

**When?**
1. running -> waiting
2. running -> ready
3. waiting -> ready
4. terminate   

1, 4 -> 항상 non-preemptive

### Dispatcher
CPU core의 소유권을 넘겨주는(context switching) 모듈   
=> process 선택은 scheduler, 교체는 dispatcher
user mode switching, user program을 resume하기 위한 적절한 위치로의 jump 기능
dispatcher latency: process를 교체하는데 드는 시간

## Scheduling Algorithms
Ready queue에 있는 process 중 어떤 process에 CPU core를 할당할 것인지 결정
### FCFS(First Come First Served)
먼저 온 것 먼저 실행(non-preemptive)   
queue를 이용하여 CPU에 먼저 요청한 process에 먼저 할당   
CPU-burst time에 따라 waiting time이 크게 달라져 잘 사용하지 않음

### SJF(Shortest Job First)
가장 짧은 job 먼저 실행   
CPU-burst time이 같으면 FCFS로 결정   
waiting time을 최소화하기 때문에 optimal 일 수 있음   
But, 다음 CPU burst time을 알 방법이 없음   
-> 해당 process의 이전 CPU burst time을 통해 예측   
preemptive, non-preemptive 둘 다 가능   
Ex) 10만큼 소요되는 process(p1)가 5만큼 실행됐을 때 1만큼 소요되는 process(p2)가 새로 도착   
p1 마저 실행 -> non-preemptive   
중단하고 p2 먼저 실행 -> preemptive

### SRTF(Shortest Remaining Time First)
Preemptive SJF scheduling   
남은 시간이 가장 짧은 것 먼저 실행   
현재 실행 중인 process의 남은 시간보다 burst time이 적은 process가 도착하면 새로운 process로 선점   
SJF보다 waiting time 단축

### RR(Round-Robin)
preemptive FCFS with time quantum(작은 단위 시간)   
-> time quantum만큼 실행되면 interrupt를 일으켜 process 변경(context switch)   
circular queue를 이용한 time sharing 방식   
-> 덜 끝난 process는 ready queue의 tail로 보내짐   
적절한 time quantum을 잡는 것이 중요   
average waiting time은 SJF보다 약간 더 길다

### Priority-based
time sharing 방식에서 우선순위 부여   
우선순위가 같으면 FCFS로 결정   
SJF - 우선순위가 CPU burst의 역   
우선순위가 작은 process는 영원히 실행되지 못하는 경우 발생(starvation)   
-> 오랫동안 waiting하고 있는 process의 우선순위를 점진적으로 높여줌   
**RR + Priority scheduling**   
우선순위가 높은 process를 먼저 실행하되, 우선순위가 같을 경우 RR로 결정

### MLQ(Multi Level Queue)
ready queue를 분리하여 각각에 우선순위 부여   
우선순위가 높은 ready queue의 process들을 모두 실행하면 다음 우선순위의 ready queue의 process들 실행   
real-time process -> system process -> interactive process(UI) -> batch process   

### MLFQ(Multi Level Feedback Queue)
우선순위가 높은 ready queue는 quantum을 짧게 주고, 우선순위가 작을수록 quantum을 많이줌   

### Thread Scheduling
현대 OS에서는 process scheduling을 하지 않고 thread scheduling함(kernel thread)   
user thread는 thread library가 관리

### Real-Time CPU Scheduling
Real-Time(실시간) OS에서의 scheduling   
- Soft Realtime   
real-time process가 반드시 deadline내에 실행되어야 하지는 않지만, 우선순위는 존재
- Hard Realtime   
task가 반드시 deadline내에 실행되어야 함

---
## 5. Process Synchronization
Cooperating process들이 공유 데이터에 동시에 접근할 때 data inconsistency 발생 가능   
-> 순서 보장 필요   
- concurrent execution   
interrupt에 의해 context switch가 일어나 다른 process에 CPU core 할당
- parallel execution
분리된 CPU core에 의해 여러 process가 실행됨   

Ex) producer-consumer problem   
두개의 process가 buffer를 이용하여 asynchronously(동작을 맞추지 않음)하게 실행되며 데이터 공유

count++, count--
![Alt text](Synchronization1.PNG)
![Alt text](Synchronization2.PNG)
count에 5가 저장되어야 하지만 경우에 따라 4, 5, 6이 저장됨

### Race Condition(경쟁 상황)
여러 processes(threads) 간 공유 데이터로의 접근 순서에 따라 실행 결과가 달라지는 상황   
이를 방지하기 위해서 process(thread) synchronization이 필요

### Critical Section Problem(임계 영역 문제)
각 process의 코드 영역 중 공유 데이터에 접근하는 코드 영역(code segment)    
코드 영역은 크게 4가지로 나뉨
- entry section: critical section에 진입하기 위한 허가 요청 영역
- critical section
- exit section: critical section에서 나오는 영역
- remainder section

### Mutual Exclusion(상호 배제)
한 process가 critical section을 실행하고 있으면, 다른 process들은 critical section을 실행할 수 없음   
-> race condition 방지(Synchronization)

### Progress
deadlock 방지   
critical section을 실행하고 있는 process가 없으면, critical section에 진입할 다음 process를 선택하는 것이 지연돼서는 안됨

### Bounded Waiting
starvation 방지   
critical section에 진입할 수 있는 시간을 한정하여 다른 process들의 critical section이 영원히 실행되지 못하는 것 방지

## SW Solutions
### Peterson Algorithm
아키텍처에 따라 제대로 동작하지 않을 수 있음   
mutual exclusion, progress, bounded waiting 증명 가능

![Alt text](peterson.PNG)

## HW Solutions
Critical-section problem 해결을 위해 hardware instruction 제공
### Atomic Operation
더이상 나눌 수 없는 operation 단위(1clock에 실행)   
Atomic instruction을 만들어 문제 해결   
-> interrupt 불가 -> mutual exclusion 보장   
Ex) test_and_set(), compare_and_swap()
이를 이용하여 atomic variable 생성

## OS Solution
### Spinlock
P(), V()라는 atomic operation으로만 접근 가능한 정수형 변수   
-> OS가 전체 operation이 한 instruction cycle에 수행되도록 보장

![Alt text](spinlock.PNG)
Spinlock(S)가 1이면 critical section을 실행 중인 process가 있고, 0이면 없다는 의미   
-> mutual exclusion 보장

But, multi processor 시스템에서만 사용 가능(parallel execution)

## Language-Level Solution
SW, HW, OS Solution -> Low-level mechanism => 사용하기 어려움, error 발생 확률 높음
Language-Level Solution -> High-level Mechanism => 프로그래밍 언어 단에서 해결
### Monitor
최대 하나의 process만 접근할 수 있는 공유 데이터와 critical section의 집합   
구조
- entry queue: 모니터 내 함수 수만큼 존재
- condition queue: 특정 이벤트를 기다리는 process가 대기
- signaler queue: 신호 제공을 위한(signal 명령을 실행한) process가 임시 대기

특징
- mutual exclusion: 모니터 내에는 하나의 process만 진입 가능
- information hiding: 공유 데이터는 모니터 내의 process만 접근 가능

모니터 내에 자원을 요청하는 function, 자원을 반납하는 function 존재   
자원을 할당 받을 수 있을 때를 기다리는 condition queue에 signaler queue에서 신호 제공

![Alt text](monitor.PNG)

과정
1. 처음 도착한 process(p1)가 자원 요청
2. 자원이 p1에 할당되고 p1이 모니터에서 나감
3. p2, p3가 요청을 위한 entry queue에 도착
4. p2가 모니터에 들어와 자원 요청
5. 하지만 모니터에 자원이 없으므로 condition queue로 이동
6. p1이 자원 사용을 끝내고 반납을 위한 entry queue에 도착
7. p1이 모니터에 들어와 자원 반납
8. p1이 signaler queue로 이동 후 condition queue에 신호 전달
9. p2가 모니터에 들어와 자원 할당 받은 후 나감
10. p1이 다시 모니터에 들어와 남은 일을 수행하고 나감

장점
- 사용이 쉬움
- error 발생 가능성 낮음(ex.deadlock)

단점
- 지원하는 언어에서만 사용 가능
- 컴파일러가 OS를 이해해야 함

---
## 6. Deadlock
### 정의
Process들이 서로가 가진 자원을 기다리며 block된 상태   
각자 일부 자원을 가지고 있는 상태에서 상대방의 자원을 요구할 때 발생   

### Resource
하드웨어, 소프트웨어를 포함하는 개념   
Ex) I/O device, CPU cycle, memory space   
**Process의 자원 사용 절차**   
Request -> Allocate -> Use -> Release   

### 발생 조건
- Mutual exclusion(상호배제)   
매 순간 하나의 process만이 자원 사용 가능
- Non-preemption(비선점)   
process의 자원을 강제로 빼앗지 못함
- Hold and wait(보유대기)   
자원을 가진 process가 추가 자원을 얻기 위해 자원을 계속 가지고 있음   
- Circular wait(순환대기)   
process 서로의 자원을 기다리는 사이클이 형성됨

### Resource-Allocation Graph(자원 할당 그래프)
Deadlock 확인 방법  
P: process   
R: resource   
P -> R: request edge   
R -> P: assignment edge

![Alt text](deadlock.PNG)   
1 -> deadlock   
2 -> deadlock x
- 그래프에 cycle(화살표를 따라 시작으로 돌아옴)이 없으면 deadlock이 아님
- 그래프에 cycle이 있고, cycle에 포함된 resource에 하나의 instance만 있으면 deadlock

### 처리 방법
방지   
- Deadlock Prevention   
자원 할당 시 deadlock의 4가지 필요 조건 중 하나를 차단   
But 여러 제약 조건으로 자원 이용률(utilization), 성능(throughput) 저하
- Deadlock Avoidance   
Deadlock의 가능성이 없는 경우에만 자원 할당   
Process가 시작될 때 해당 process가 평생 쓸 자원의 양을 파악하여 deadlock 가능성 파악 후 자원 할당 결정   
-> process가 추가로 필요한 최대(평생 쓸) 자원의 양이 가용 자원보다 많으면 할당하지 않고, 자원 반납을 통해 가용 자원이 추가되면 다시 확인   
=> 최악의 상황을 고려   
Ex) Banker's Algorithm
![Alt text](banker.PNG)

처리   
- Deadlock Detection and Recovery   
Deadlock 발생은 허용하되 발견 시 recover   
Banker's Algorithm을 낙관적으로 사용하여 detection   
=> 최선의 상황을 고려    
Deadlock에 연루된 process들을 모두 종료시키거나, 하나씩 자원을 빼앗아서 recover
- Deadlock Ignorance   
Deadlock을 시스템이 책임지지 않음   
Deadlock은 빈번히 발생하는 것이 아니기 떄문에 미연에 방지하기 위해 조치를 취하는 것이 더 많은 overhead 발생시킴 -> 대부분 이 방식 사용