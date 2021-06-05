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
    int current_index = 0;  // 현재 수행 중인 명령어 index
    int run_time = 0;  // 실행된 cycle 수
    int sleep_time = 0;  // 남은 sleep time
    int time_quantum = 10;  // 남은 time quantum

	Process() {
        pid = -1;
    }

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
void checkSleepOver(deque<Process> run_queues[], list<Process> *sleep_list) {
    list<Process>::iterator iter;

    for(iter = sleep_list->begin(); iter!= sleep_list->end(); iter++) {
        Process *process = &*iter;
        process->sleep_time--;
        // Sleep 종료
        if(process->sleep_time == 0) {
            // process->time_quantum = 10;
            run_queues[process->priority].push_back(*process);
            sleep_list->erase(iter);
            return;
        }
    }    
    return;
}


// IO 작업 시행 여부 검사
void checkIO(deque<Process> run_queues[], deque<IO> *ios, list<Process> *iowait_list, int cycle) {
    int count = 0;
    list<Process>::iterator iter;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < ios->size(); i++) {
        IO io = ios->at(i);
        if(io.start_cycle == cycle) {
            for(iter = iowait_list->begin(); iter!= iowait_list->end(); iter++) {
                Process *process = &*iter;
                // IO 작업 종료
                if(io.pid == process->pid) {
                    // process->time_quantum = 10;
                    run_queues[process->priority].push_back(*process);
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
void create_process(deque<Process> run_queues[], deque<Process> *processes, int cycle) {
    int count = 0;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < processes->size(); i++) {
        Process process = processes->at(i);
        if(process.start_cycle == cycle) {
            run_queues[process.priority].push_back(process);
            count++;
        }
    }

    for(int i = 0; i < count; i++) {
        processes->pop_front();
    }
    return;
}


// First-come First-served
void fcfs(deque<Process> run_queues[], Process cpu[]) {
    int priority = cpu[0].priority;

    // 현재 실행되고 있는 프로세스보다 우선순위가 높은 프로세스가 있으면 교체
    for(int i = 0; i < priority; i++) {
        if(!run_queues[i].empty()) {
            run_queues[priority].push_back(cpu[0]);
            cpu[0] = run_queues[i].front();
            run_queues[i].pop_front();
            cpu[0].run_time = 0;
            cpu[0].time_quantum = 10;
            break;
        }
    }
    return;
}


// Round Robin
void rr(deque<Process> run_queues[], Process cpu[]) {
    Process null_process;
    int priority = cpu[0].priority;

    // 주어진 time quantum을 다 사용하면 run queue로 이동
    if(cpu[0].time_quantum == 0) {
        // running_process->time_quantum = 10;
        run_queues[priority].push_back(cpu[0]);
        cpu[0] = null_process;

        // 대기하고 있는 프로세스가 있으면 cpu 할당
        for(int i = 0; i < 10; i++) {
            if(!run_queues[i].empty()) {
                cpu[0] = run_queues[i].front();
                run_queues[i].pop_front();
                cpu[0].run_time = 0;
                cpu[0].time_quantum = 10;
                break;
            }
        }
    }
    // time quantum이 남았을 경우
    else {
        // 현재 실행되고 있는 프로세스보다 우선순위가 높은 프로세스가 있으면 교체
        for(int i = 0; i < priority; i++) {
            if(!run_queues[i].empty()) {
                run_queues[priority].push_back(cpu[0]);
                cpu[0] = run_queues[i].front();
                run_queues[i].pop_front();
                cpu[0].run_time = 0;
                cpu[0].time_quantum = 10;
                break;
            }
        }
    }
    return;
}


// CPU 스케줄링
void schedule(deque<Process> run_queues[], Process cpu[]) {
    // 현재 실행 중인 프로세스가 없을 경우
    if(cpu[0].pid == -1) {
        for(int i = 0; i < 10; i++) {
            if(!run_queues[i].empty()) {
                cpu[0] = run_queues[i].front();
                run_queues[i].pop_front();
                cpu[0].run_time = 0;
                cpu[0].time_quantum = 10;
                break;
            }
        }
    }
    // 현재 실행 중인 프로세스가 있을 경우
    else {
        int priority = cpu[0].priority;
        // 우선순위 0~4
        if(priority <= 4) {
            fcfs(run_queues, cpu);
        }
        // 우선순위 5~9
        else {
            rr(run_queues, cpu);
        }
    }
    return;
}


void sleepInstruction(Process cpu[], list<Process> *sleep_list, int sleep_cycle) {
    cpu[0].sleep_time = sleep_cycle;

    // 마지막 명령어가 아닌 경우
    if(cpu[0].current_index < cpu[0].instructions.size()-1) {
        sleep_list->push_back(cpu[0]);
    }
}


void executeInstruction(Process cpu[], list<Process> *sleep_list, list<Process> *iowait_list) {
    int current_index = cpu[0].current_index;
    int opcode = cpu[0].instructions[current_index].first;
    int arg = cpu[0].instructions[current_index].second;

    // Memory allocation
    if(opcode == 0) {
    }
    // Memory access
    else if(opcode == 1) {
    }
    // Memory release
    else if(opcode == 2) {
    }
    // Non-memory instruction
    else if(opcode == 3) {
    }
    // Sleep
    else if(opcode == 4) {
        sleepInstruction(cpu, sleep_list, arg);
    }
    // IO wait
    else if(opcode == 5) {
        // 마지막 명령어가 아닌 경우
        if(cpu[0].current_index < cpu[0].instructions.size()-1) {
            iowait_list->push_back(cpu[0]);
        }
    }
}


void printSchedule(FILE *fout, deque<Process> run_queues[], list<Process> *sleep_list, list<Process> *iowait_list, Process cpu[], int cycle) {
    int pid = cpu[0].pid;
    const char *name;
    int priority;
    int current_index;
    int op;
    int arg;
    list<Process>::iterator iter;

    // 실행 중인 프로세스가 있을 경우
    if(pid >= 0) {
        name = cpu[0].name.c_str();
        priority = cpu[0].priority;
        current_index = cpu[0].current_index;
        op = cpu[0].instructions[current_index].first;
        arg = cpu[0].instructions[current_index].second;
    }

    // line 1
    fprintf(fout, "[%d Cycle] Scheduled Process: ", cycle);
    if(cpu[0].run_time == 0 && pid >= 0) {
        fprintf(fout, "%d %s (priority %d)\n", pid, name, priority);
    }
    else {
        fprintf(fout, "None\n");
    }

    // line 2
    fprintf(fout, "Running Process: ");
    if(pid >= 0) {
        fprintf(fout, "Process#%d(%d) running code %s line %d(op %d, arg %d)\n", pid, priority, name, current_index+1, op, arg);
    }
    else {
        fprintf(fout, "None\n");
    }

    // line 3
    for(int i = 0; i < 10; i++) {
        fprintf(fout, "RunQueue %d: ", i);
        if(run_queues[i].empty()) {
            fprintf(fout, "Empty");
        }
        else {
            for(int j = 0; j < run_queues[i].size(); j++) {
                fprintf(fout, "%d(%s) ", run_queues[i][j].pid, run_queues[i][j].name.c_str());
            }
        }
        fprintf(fout, "\n");
    }

    // line 4
    fprintf(fout, "SleepList: ");
    if(sleep_list->empty()) {
        fprintf(fout, "Empty");
    }
    else {
        for(iter = sleep_list->begin(); iter != sleep_list->end(); iter++) {
            fprintf(fout, "%d(%s) ", (*iter).pid, (*iter).name.c_str());
        }
    }
    fprintf(fout, "\n");

    // line 5
    fprintf(fout, "IOWait List: ");
    if(iowait_list->empty()) {
        fprintf(fout, "Empty");
    }
    else {
        for(iter = iowait_list->begin(); iter != iowait_list->end(); iter++) {
            fprintf(fout, "%d(%s) ", (*iter).pid, (*iter).name.c_str());
        }
    }
    fprintf(fout, "\n");

    fprintf(fout, "\n");
}


// ./project3 -page=lru -dir=/home/jihyun/OS-2021-1/assignment3
int main(int argc, char *argv[]) {
    int total_event_num;
    int vm_size;
    int pm_size;
    int page_size;
    // Default 옵션
    string dir = ".";
    string page = "lru";

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

	deque<Process> processes;  // 전체 프로그램 저장
    deque<IO> ios;  // 전체 IO 작업 저장
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
    deque<Process> run_queues[10];  // index = priority
    list<Process> sleep_list;
    list<Process> iowait_list;

    // 현재 실행 중인 프로세스(기본 pid = -1)
    Process cpu[1];
    Process running_process;
    cpu[0] = running_process;
    
    int total_process_num = processes.size();
    int cycle = 0;

    // 출력 파일
    string schedule_file = dir + "/scheduler.txt";
    FILE* fout1 = fopen(schedule_file.c_str(), "w");

    string memory_file = dir + "/memory.txt";
    FILE* fout2 = fopen(memory_file.c_str(), "w");

    // Cycle 시작
    while(total_process_num > 0) {
        checkSleepOver(run_queues, &sleep_list);
        checkIO(run_queues, &ios, &iowait_list, cycle);
        create_process(run_queues, &processes, cycle);
        
        schedule(run_queues, cpu);

        // 실행할 프로세스가 있을 때
        if(cpu[0].pid >= 0) {
            executeInstruction(cpu, &sleep_list, &iowait_list);
        }

        printSchedule(fout1, run_queues, &sleep_list, &iowait_list, cpu, cycle);

        // 종료되지 않은 프로세스 수
        total_process_num = sleep_list.size() + iowait_list.size();
        for(int i = 0; i < 10; i++) {
            total_process_num += run_queues[i].size();
        }
        if(cpu[0].pid >= 0) {
            total_process_num++;
        }

        cpu[0].run_time++;
        cpu[0].current_index++;
        cpu[0].time_quantum--;
        cycle++;
    }
    // open한 파일 close
	fin.close();
    fclose(fout1);
    fclose(fout2);

	return 0;
}