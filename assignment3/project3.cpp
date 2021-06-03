#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <deque>

using namespace std;


// void round_robin()
// {
// 	printf("Time Quantum for Queue1 is 4\n");
// 	for(i=0;i<j;i++)
// 	{
// 		temp[i]=burst_time1[i];
// 	} 
// 	printf("\nProcess ID\tBurst Time\t Turnaround Time\t Waiting Time\n");
// 	x=j;
//     for(i=0,total=0;x!=0;) 
//     { 
//     	if(temp[i]<=4&&temp[i]>0) 
//         {
// 			printf("\nProcess[%d] of Queue1 is running for %d units",i+1,temp[i]); 
//             total=total+temp[i]; 
//             temp[i]=0; 
//             counter=1; 
//         } 
//         else if(temp[i]>0) 
//         {
// 			printf("\nProcess[%d] of Queue1 is running for 4 units",i+1); 
//             temp[i]=temp[i]-4; 
//             total=total+4; 
//         } 
//         if(temp[i]==0&&counter==1) 
//         { 
//             x--; 
//             printf("\nProcess[%d]\t%d\t%d\t%d",i+1,burst_time1[i],total-arrival_time1[i],total-arrival_time1[i]-burst_time1[i]);
//             avg_waiting_time1=avg_waiting_time1+total-arrival_time1[i]-burst_time1[i]; 
//             avg_turnaround_time1=avg_turnaround_time1+total-arrival_time1[i]; 
//             counter = 0; 
//         } 
//         if(i==j-1) 
//         {
//             i=0; 
//         }
//         else if(arrival_time1[i+1]<=total) 
//         {
//             i++;
//         }
//         else 
//         {
//             i=0;
//         }
//     } 
//     avg_waiting_time1=avg_waiting_time1/j;
//     avg_turnaround_time1=avg_turnaround_time1/j;
//     printf("\nAverage Waiting Time:%f",avg_waiting_time1); 
//     printf("\nAverage Turnaround Time:%f\n",avg_turnaround_time1); 
// }


// void priority()
// {
// 	for(i=0;i<k;i++)
//     {
//         position=i;
//         for(q=i+1;q<k;q++)
//         {
//             if(priority2[q]<priority2[position])
//             {
//                 position=q;
//             }
//         }
//         temp1=priority2[i];
//         priority2[i]=priority2[position];
//         priority2[position]=temp1; 
        
//         temp1=burst_time2[i];
//         burst_time2[i]=burst_time2[position];
//         burst_time2[position]=temp1;
        
//         temp1=process2[i];
//         process2[i]=process2[position];
//         process2[position]=temp1;
//     }
//     waiting_time2[0]=0;
//     for(i=1;i<k;i++)
//     {
//         waiting_time2[i]=0;
//         for(q=0;q<i;q++)
//         {
//             waiting_time2[i]=waiting_time2[i]+burst_time2[j];
//         }
//         sum=sum+waiting_time2[i];
//     }
//     avg_waiting_time2=sum/k;
//     sum=0;
//     printf("\nProcess ID\t\tBurst Time\t Waiting Time\t Turnaround Time\n");
//     for(i=0;i<k;i++)
//     {
//     	turnaround_time2[i]=burst_time2[i]+waiting_time2[i];
//         sum=sum+turnaround_time2[i];
//         printf("\nProcess[%d]\t\t%d\t\t %d\t\t %d\n",process2[i],burst_time2[i],waiting_time2[i],turnaround_time2[i]);
//     }
//     avg_turnaround_time2=sum/k;
//     printf("\nAverage Waiting Time:\t%f",avg_waiting_time2);
//     printf("\nAverage Turnaround Time:\t%f\n",avg_turnaround_time2);
    
//     for(i=0;i<k;i++)
//     {
//     	while(burst_time2[i]!=0)
//     	{
//     		if(burst_time2[i]>10)
//     		{
// 				printf("\nProcess[%d] of Queue2 is running for 10 units",i+1);
// 				burst_time2[i]=burst_time2[i]-10;
// 			}
// 			else if(burst_time2[i]<=10)
// 			{
// 				printf("\nProcess[%d] of Queue2 is running for %d units",i+1,burst_time2[i]);
// 				burst_time2[i]=0;
// 			}
// 		}
// 	}


// }


// void fcfs()
// {
// 	waiting_time3[0] = 0;   
//     for(i=1;i<l;i++)
//     {
//         waiting_time3[i] = 0;
//         for(p=0;p<l;p++)
//         {
//             waiting_time3[i]=waiting_time3[i]+burst_time3[p];
//         }
//     }
//     printf("\nProcess\t\tBurst Time\tWaiting Time\tTurnaround Time\n");
//     for(i=0;i<l;i++)
//     {
//         turnaround_time3[i]=burst_time3[i]+waiting_time3[i];
//         avg_waiting_time3=avg_waiting_time3+waiting_time3[i];
//         avg_turnaround_time3=avg_turnaround_time3+turnaround_time3[i];
//         printf("\nProcess[%d]\t\t%d\t\t%d\t\t%d\n",i+1,burst_time3[i],waiting_time3[i],turnaround_time3[i]);
//     }
//     avg_waiting_time3=avg_waiting_time3/l;
//     avg_turnaround_time3=avg_turnaround_time3/l;
//     printf("\nAverage Waiting Time=%f",avg_waiting_time3);
//     printf("\nAverage Turnaround Time=%f",avg_turnaround_time3);
//     for(i=0;i<l;i++)
//     {
//     	while(burst_time3[i]!=0)
//     	{
//     		if(burst_time3[i]>10)
//     		{
// 				printf("\nProcess[%d] of Queue3 is running for 10 units",i+1);
// 				burst_time3[i]=burst_time3[i]-10;
// 			}
// 			else if(burst_time3[i]<=10)
// 			{
// 				printf("\nProcess[%d] of Queue2 is running for %d units",i+1,burst_time3[i]);
// 				burst_time3[i]=0;
// 			}
// 		}
// 	}
// }


// void round_robin1()
// {
// 	printf("Time Quantum between the 3 queues is 10\n");
// 	for(i=1;i<Total;i=i+10)
// 	{
// 		if(t1>10)
// 		{
// 			printf("Queue1 is running for 10 units\n");
// 			t1=t1-10;
// 		}
// 		else if(t1<=10&&t1!=0)
// 		{
// 			printf("Queue1 is running for %d units\n",t1);
// 			t1=0;
// 		}
// 		if(t2>10)
// 		{
// 			printf("Queue2 is running for 10 units\n");
// 			t2=t2-10;
// 		}
// 		else if(t2<=10&&t2!=0)
// 		{
// 			printf("Queue2 is running for %d units\n",t2);
// 			t2=0;
// 		}
// 		if(t3>10)
// 		{
// 			printf("Queue3 is running for 10 units\n");
// 			t3=t3-10;
// 		}
// 		else if(t3<=10&&t3!=0)
// 		{
// 			printf("Queue3 is running for %d units\n",t3);
// 			t3=0;
// 		}
// 	}
// }

// int arrival_time1[30],arrival_time2[30],priority2[30],process2[30],arrival_time3[30];
// int burst_time1[30],burst_time2[30],burst_time3[30];

// int Total=0,t1=0,t2=0,t3=0;

// int n,i,at[30],bt[30],pr[30],j=0,k=0,l=0;

// int total,x,temp[30],counter=0;
// float avg_waiting_time1=0.0,avg_turnaround_time1=0.0;

// int p,waiting_time3[30],turnaround_time3[30];
// float avg_waiting_time3=0.0,avg_turnaround_time3=0.0;

// int position,q,temp1,sum=0,waiting_time2[30],turnaround_time2[30];
// float avg_waiting_time2,avg_turnaround_time2;


// 실행 작업
class Process {
public:
	int start_cycle;
	string name;
	int priority;
	int pid;
    deque<pair<int, int>> instructions;

	Process() {}

	Process(int start_cycle_, string name_, int priority_, int pid_) {
		start_cycle = start_cycle_;
		name = name_;
		priority = priority_;
		pid = pid_;
	}

    // 프로그램 명령어 저장
    void addInstruction(string file_name) {
        int total_instruction_num;
        int opcode;
        int arg;

        ifstream fin;
        fin.open(file_name);
        fin >> total_instruction_num;

        for(int i = 0; i < total_instruction_num; i++) {
            fin >> opcode >> arg;
            instructions.push_back(make_pair(opcode, arg));
        }
        fin.close();
    }
};


// I/O 작업
class IO {
public:
    int start_cycle;
    string name;
    int pid;

	IO() {}

    IO(int start_cycle_, string name_, int pid_) {
		start_cycle = start_cycle_;
		name = name_;
		pid = pid_;
	}
};


// ./project3 -page=lru -dir=/home/jihyun/OS-2021-1/assignment3
int main(int argc, char *argv[]) {
    string dir = ".";
    string page = "lru";
	int total_event_num;
    int vm_size;
    int pm_size;
    int page_size;

	// 프로그램 실행 명령어 파싱
	for(int i = 1; i < argc; i++) {
        string option = argv[i];
        int oper = option.find("=") + 1;
        string opt_name = option.substr(0, oper);
        string opt_detail = option.substr(oper, option.length());

        if(opt_name == "-page=") {
            page = opt_detail;
        }
		else if(opt_name == "-dir=") {
            dir = opt_detail;
        }
    }

	// input 파일 파싱
	string input_file = dir + "/input";
    ifstream fin;
    fin.open(input_file);

	// input 첫 번째 줄
    fin >> total_event_num >> vm_size >> pm_size >> page_size;

	deque<Process> processes;
    deque<IO> ios;
	int start_cycle;
	string code;
	int priority;
    int pid = 0;

	// input 두 번째 줄 이후
    for(int i = 0; i < total_event_num; i++) {
        fin >> start_cycle >> code >> priority;

        // I/O 작업
        if(code == "INPUT") {
			IO io(start_cycle, code, priority);
            ios.push_back(io);
        }
        // 실행 작업
		else {
            Process process(start_cycle, code, priority, pid);
            string program_file = dir + "/" + process.name;
            process.addInstruction(program_file);
            processes.push_back(process);
            pid++;
        }
    }

    // 프로세스 상태 큐
    Process running_process;
    deque<Process> run_queue0;
    deque<Process> run_queue1;
    deque<Process> run_queue2;
    deque<Process> run_queue3;
    deque<Process> run_queue4;
    deque<Process> run_queue5;
    deque<Process> run_queue6;
    deque<Process> run_queue7;
    deque<Process> run_queue8;
    deque<Process> run_queue9;
    deque<Process> sleep_list;
    deque<Process> iowait_list;

    // 출력 파일
    string schedule_file = dir + "/scheduler.txt";
    ofstream fout1;
    fout1.open(schedule_file);

    string memory_file = dir + "/memory.txt";
    ofstream fout2;
    fout2.open(memory_file);

    int process_num = processes.size();
    int cycle = 0;

    //작업 수행
    while(process_num > 0) {
        Process process = processes.front();

        if(process.start_cycle == cycle) {
            if(process.priority == 0) {
                run_queue0.push_back(process);
            }
            else if(process.priority == 1) {
                run_queue1.push_back(process);
            }
            else if(process.priority == 2) {
                run_queue2.push_back(process);
            }
            else if(process.priority == 3) {
                run_queue3.push_back(process);
            }
            else if(process.priority == 4) {
                run_queue4.push_back(process);
            }
            else if(process.priority == 5) {
                run_queue5.push_back(process);
            }
            else if(process.priority == 6) {
                run_queue6.push_back(process);
            }
            else if(process.priority == 7) {
                run_queue7.push_back(process);
            }
            else if(process.priority == 8) {
                run_queue8.push_back(process);
            }
            else if(process.priority == 9) {
                run_queue9.push_back(process);
            }
        }
        cycle++;
        process_num--;
    }

	fin.close();
	fout1.close();
    fout2.close();
	return 0;
}