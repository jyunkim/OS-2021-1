#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <deque>
#include <list>

using namespace std;


// 실행 작업
class Process {
public:
    // input으로 받는 정보들
	int start_cycle;
	string name;
	int priority;
	int pid;
    
    deque<pair<int, int>> instructions;  // 작업 별 명령어 리스트
    int current_instruction;  // 현재 수행 중인 명령어 index
    int sleep_time;  // 남은 sleep time

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


// Sleep된 프로세스의 종료 여부 검사
void checkSleepOver(deque<Process> *run_queue, list<Process> *sleep_list) {
    list<Process>::iterator iter;

    for(iter = sleep_list->begin(); iter!= sleep_list->end(); iter++) {
        Process process = *iter;
        process.sleep_time--;
        if(process.sleep_time == 0) {
            run_queue->push_back(process);
            sleep_list->erase(iter);
            return;
        }
    }    
    return;
}


// IO 작업 시행 여부 검사
void checkIO(deque<Process> *run_queue, deque<IO> *ios, list<Process> *iowait_list, int cycle) {
    int count = 0;
    list<Process>::iterator iter;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < ios->size(); i++) {
        IO io = ios->at(i);
        if(io.start_cycle == cycle) {
            for(iter = iowait_list->begin(); iter!= iowait_list->end(); iter++) {
                Process process = *iter;
                if(io.pid == process.pid) {
                    run_queue->push_back(process);
                    iowait_list->erase(iter);
                    count++;
                    break;
                }
            }
        }
    }

    for(int i = 0; i < count; i++) {
        ios->pop_front();
    }
    return;
}


// 프로세스 생성 작업 시행
void create_process(deque<Process> *run_queue, deque<Process> *processes, int cycle) {
    int count = 0;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < processes->size(); i++) {
        Process process = processes->at(i);
        if(process.start_cycle == cycle) {
            run_queue->push_back(process);
            count++;
        }
    }

    for(int i = 0; i < count; i++) {
        processes->pop_front();
    }
    return;
}


void executeInstruction(Process process, int cur) {
    int opcode = process.instructions[cur].first;
    int arg = process.instructions[cur].second;

    // Memory allocation
    if(opcode == 0) {
        // cmd_malloc(currentCpuTask, all_pages, runCmd[1], aid);
    }
    // Memory access
    else if(opcode == 1) {
        // cmd_memAccess(currentCpuTask, runningTask, physicalMem, physicalNum, runCmd[1], currentCycle, page_opt, page_fault);
    }
    // Memory release
    else if(opcode == 2) {
        // cmd_memFree(currentCpuTask, runCmd[1], physicalMem, physicalNum);
    }
    // Non-memory instruction
    else if(opcode == 3) {
        // currentCpuTask->flagToEmpty = false;
        // currentCpuTask->flagToComplete = true;
    }
    // Sleep
    else if(opcode == 4) {
        // cmd_sleep(currentCpuTask, runCmd[1], sleepList, sched_opt);
        
    }
    // IO wait
    else if(opcode == 5) {
        // cmd_ioWait(currentCpuTask, ioWaitList, sched_opt);
    }
}


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
    list<Process> sleep_list;
    list<Process> iowait_list;

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
        

        if(running_process.start_cycle == cycle) {
            if(running_process.priority == 0) {
                run_queue0.push_back(running_process);
            }
            else if(running_process.priority == 1) {
                run_queue1.push_back(running_process);
            }
            else if(running_process.priority == 2) {
                run_queue2.push_back(running_process);
            }
            else if(running_process.priority == 3) {
                run_queue3.push_back(running_process);
            }
            else if(running_process.priority == 4) {
                run_queue4.push_back(running_process);
            }
            else if(running_process.priority == 5) {
                run_queue5.push_back(running_process);
            }
            else if(running_process.priority == 6) {
                run_queue6.push_back(running_process);
            }
            else if(running_process.priority == 7) {
                run_queue7.push_back(running_process);
            }
            else if(running_process.priority == 8) {
                run_queue8.push_back(running_process);
            }
            else if(running_process.priority == 9) {
                run_queue9.push_back(running_process);
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