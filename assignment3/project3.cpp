#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <deque>
#include <list>

using namespace std;

int aid = 0;  // allocation id
int page_fault = 0;  // page fault 발생 수

// Page Table
class PageTable {
public:
    int *page_ids;
    int *valid_bits;
    int *allocation_ids;
    int *reference_bits;
    deque<int> reference_byte;
    list<int> lru_stack;

    PageTable() {}

    // 여러 종류의 bit를 담는 배열을 page 개수 크기로 생성
    // 배열 요소를 -1로 초기화
    PageTable(int page_num) {
        page_ids = new int[page_num];
        valid_bits = new int[page_num];
        allocation_ids = new int[page_num];
        reference_bits = new int[page_num];

        fill_n(page_ids, page_num, -1);
        fill_n(valid_bits, page_num, -1);
        fill_n(allocation_ids, page_num, -1);
        fill_n(reference_bits, page_num, -1);
    }
};


// 실행 작업
class Process {
public:
    // input으로 받는 정보들
	int start_cycle;
	string name;
	int priority;
	int pid;

    bool blocked = false;  // 중지될 프로세스 인지 체크(출력하고 나서 쫓아냄)
    
    deque<pair<int, int>> instructions;  // 작업 별 명령어 리스트
    int current_index = 0;  // 현재 수행 중인 명령어 index
    int run_time = 0;  // 실행된 cycle 수
    int sleep_time = 0;  // 남은 sleep time
    int time_quantum = 10;  // 남은 time quantum

    PageTable *page_table;
    int page_id = 0;  // 다음에 할당할 page id
    int page_index = 0;  // 다음에 할당할 page index

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

    for(iter = sleep_list->begin(); iter != sleep_list->end();) {
        iter->sleep_time--;
        // Sleep 종료
        if(iter->sleep_time == 0) {
            run_queues[iter->priority].push_back(*iter);
            sleep_list->erase(iter++);
        }
        else {
            iter++;
        }
    }    
}


// IO 작업 시행 여부 검사
void checkIO(deque<Process> run_queues[], deque<IO> *ios, list<Process> *iowait_list, int cycle) {
    int count = 0;
    list<Process>::iterator iter;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < ios->size(); i++) {
        IO io = ios->at(i);
        if(io.start_cycle == cycle) {
            for(iter = iowait_list->begin(); iter != iowait_list->end();) {
                // IO 작업 종료
                if(io.pid == iter->pid) {
                    run_queues[iter->priority].push_back(*iter);
                    iowait_list->erase(iter++);
                    count++;
                }
                else {
                    iter++;
                }
            }
        }
    }

    for(int i = 0; i < count; i++) {
        ios->pop_front();
    }
}


// 프로세스 생성 작업 시행
void create_process(deque<Process> run_queues[], deque<Process> *programs, list<Process> *processes, int cycle, int page_num) {
    int count = 0;

    // 같은 time에 여러 작업이 들어올 수 있으므로 순회
    for(int i = 0; i < programs->size(); i++) {
        Process process = programs->at(i);
        if(process.start_cycle == cycle) {
            process.page_table = new PageTable(page_num);
            processes->push_back(process);
            run_queues[process.priority].push_back(process);
            count++;
        }
    }

    for(int i = 0; i < count; i++) {
        programs->pop_front();
    }
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
}


// Round Robin
void rr(deque<Process> run_queues[], Process cpu[]) {
    Process null_process;
    int priority = cpu[0].priority;

    // 주어진 time quantum을 다 사용하면 교체
    if(cpu[0].time_quantum == 0) {
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
}


// Memory Allocation 명령어 수행
void memoryAllocation(Process cpu[], list<Process> *processes, int page_num) {
    list<Process>::iterator iter;
    int page_id = cpu[0].page_id;
    int page_index = cpu[0].page_index;
    int *pid = cpu[0].page_table->page_ids;
    int *valid = cpu[0].page_table->valid_bits;

    // 할당할 page 수만큼 page id, valid bit 입력
    for(int i = page_index; i < page_index + page_num; i++) {
        pid[i] = page_id;
        valid[i] = 0;
    }
    cpu[0].page_id++;
    cpu[0].page_index += page_num;

    // processes 리스트도 업데이트
    for(iter = processes->begin(); iter != processes->end(); iter++) {
        if(iter->pid == cpu[0].pid) {
            pid = iter->page_table->page_ids;
            valid = iter->page_table->valid_bits;
            
            for(int i = page_index; i < page_index + page_num; i++) {
                pid[i] = page_id;
                valid[i] = 0;
            }
            iter->page_id++;
            iter->page_index += page_num;
            break;
        }
    }
}


// Memory Access 명령어 수행
void memoryAccess(Process cpu[], list<Process> *processes, int physical_memory[], int page_id, int page_num, int frame_num, string algorithm) {
    list<Process>::iterator iter;
    int p_num = 0;
    int buddy_size = frame_num;
    int my_aid = aid;
    bool update_aid = true;
    bool is_page_fault = true;
    list<int> *stk = &(cpu[0].page_table->lru_stack);

    // 해당 page id에 해당하는 allocation id 검색
    for(int i = 0; i < page_num; i++) {
        if(cpu[0].page_table->page_ids[i] == page_id && cpu[0].page_table->allocation_ids[i] != -1) {
            my_aid = cpu[0].page_table->allocation_ids[i];
            update_aid = false;
            break;
        }
    }
    if(update_aid) {
        aid++;
    }

    // Physical memory에 할당된 프레임이 있는지 확인
    for(int i = 0; i < frame_num; i++) {
        if(physical_memory[i] == my_aid) {
            is_page_fault = false;
            break;
        }
    }

    // Physical memory에 할당된 프레임이 없는 경우
    if(is_page_fault) {
        page_fault++;

        // 주어진 page id에 해당하는 page table 내 page 수 계산
        // 해당 page의 allocation id 할당
        for(int i = 0; i < page_num; i++) {
            if(cpu[0].page_table->page_ids[i] == page_id) {
                p_num++;
                cpu[0].page_table->allocation_ids[i] = my_aid;
                cpu[0].page_table->valid_bits[i] = 1;
            }
        }
        // processes 리스트도 업데이트
        for(iter = processes->begin(); iter != processes->end(); iter++) {
            if(iter->pid == cpu[0].pid) {
                for(int i = 0; i < page_num; i++) {
                    if(iter->page_table->page_ids[i] == page_id) {
                        iter->page_table->allocation_ids[i] = my_aid;
                        iter->page_table->valid_bits[i] = 1;
                    }
                }
                break;
            }
        }

        // Buddy system에 의해 할당할 frame 수 계산
        while(p_num <= buddy_size/2) {
            buddy_size /= 2;
        }
        
        bool available = false;
        // Physical memory에 할당 가능한 frame이 있는지 확인한 후, 있으면 할당
        for(int i = 0; i < frame_num; i += buddy_size) {
            if(physical_memory[i] == -1 && physical_memory[i+buddy_size-1] == -1) {
                for(int j = i; j < i + buddy_size; j++) {
                    physical_memory[j] = my_aid;
                }
                available = true;
                break;
            }
        }

        int victim_aid;
        // 할당 가능한 frame이 없다면 페이지 교체 알고리즘 수행
        while(!available) {
            if(algorithm == "lru") {
                victim_aid = stk->front();
                stk->pop_front();
            }
            else if(algorithm == "sampled") {

            }
            else if(algorithm == "clock") {

            }

            // Victim swap out
            for(int i = 0; i < frame_num; i++) {
                if(physical_memory[i] == victim_aid) {
                    physical_memory[i] = -1;
                }
            }

            // Valid bit 수정
            for(int i = 0; i < page_num; i++) {
                if(cpu[0].page_table->allocation_ids[i] == victim_aid) {
                    cpu[0].page_table->valid_bits[i] = 0;
                }
            }
            // processes 리스트도 업데이트
            for(iter = processes->begin(); iter != processes->end(); iter++) {
                if(iter->pid == cpu[0].pid) {
                    for(int i = 0; i < page_num; i++) {
                        if(iter->page_table->allocation_ids[i] == victim_aid) {
                            iter->page_table->valid_bits[i] = 0;
                        }
                    }
                    break;
                }
            }

            // Swap out 후 할당 가능한지 다시 확인
            for(int i = 0; i < frame_num; i += buddy_size) {
                if(physical_memory[i] == -1 && physical_memory[i+buddy_size-1] == -1) {
                    for(int j = i; j < i + buddy_size; j++) {
                        physical_memory[j] = my_aid;
                    }
                    available = true;
                    break;
                }
            }
        }
    }

    // lru 스택 업데이트
    if(algorithm == "lru") {
        list<int>::iterator iter;
        for(iter = stk->begin(); iter != stk->end(); iter++) {
            if(*iter == my_aid) {
                stk->erase(iter);
                break;
            }
        }
        stk->push_back(my_aid);
    }
    else if(algorithm == "sampled") {

    }
    else if(algorithm == "clock") {

    }
}


// Sleep 명령어 수행
void sleepInstruction(Process cpu[], list<Process> *sleep_list, int sleep_cycle) {
    cpu[0].sleep_time = sleep_cycle;

    // 마지막 명령어가 아닌 경우
    if(cpu[0].current_index < cpu[0].instructions.size()-1) {
        sleep_list->push_back(cpu[0]);
        sleep_list->back().blocked = false;
        sleep_list->back().current_index++;
    }
    cpu[0].blocked = true;
}


// 프로세스 명령어 수행
void executeInstruction(Process cpu[], deque<Process> run_queues[], list<Process> *sleep_list, list<Process> *iowait_list, list<Process> *processes, int physical_memory[], int page_num, int frame_num, string algorithm) {
    int current_index = cpu[0].current_index;
    int opcode = cpu[0].instructions[current_index].first;
    int arg = cpu[0].instructions[current_index].second;
    int priority = cpu[0].priority;

    // Memory allocation
    if(opcode == 0) {
        memoryAllocation(cpu, processes, arg);
    }
    // Memory access
    else if(opcode == 1) {
        memoryAccess(cpu, processes, physical_memory, arg, page_num, frame_num, algorithm);
    }
    // Memory release
    else if(opcode == 2) {
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
            iowait_list->back().blocked = false;
            iowait_list->back().current_index++;
        }
        cpu[0].blocked = true;
    }

    // 우선순위가 5 이상일 경우
    // time quantum이 1이면 실행 이후 time quantum이 0이 되므로 run queue에 미리 넣어줌
    // 마지막 명령어라면 수행 x
    if(priority >= 5 && cpu[0].time_quantum == 1 && cpu[0].current_index < cpu[0].instructions.size()-1) {
        run_queues[priority].push_back(cpu[0]);
        run_queues[priority].back().current_index++;
    }
}


// 스케줄링 기록 출력
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
            fprintf(fout, "%d(%s) ", iter->pid, iter->name.c_str());
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
            fprintf(fout, "%d(%s) ", iter->pid, iter->name.c_str());
        }
    }
    fprintf(fout, "\n");

    fprintf(fout, "\n");
}


// 메모리 정보 출력
void printMemory(FILE *fout, list<Process> *processes, Process cpu[], int physical_memory[], int cycle, int page_num, int frame_num) {
    int pid = cpu[0].pid;
    const char *name;
    int current_index;
    int op;
    int arg;
    int page_id;
    int p_num = 0;
    list<Process>::iterator iter;

    // 실행 중인 프로세스가 있을 경우
    if(pid >= 0) {
        name = cpu[0].name.c_str();
        current_index = cpu[0].current_index;
        op = cpu[0].instructions[current_index].first;
        arg = cpu[0].instructions[current_index].second;
        page_id = cpu[0].page_id - 1;
    }

    // line 1
    // 현재 실행 중인 프로세스가 없을 경우
    if(pid == -1) {
        fprintf(fout, "[%d Cycle] Input: Function[NO-OP]\n", cycle);
    }
    // 현재 실행 중인 프로세스가 있을 경우
    else {
        // Memory allocation
        if(op == 0) {
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[ALLOCATION] Page ID[%d] Page Num[%d]\n", cycle, pid, page_id, arg);
        }
        // memory access
        else if(op == 1) {
            for(int i = 0; i < page_num; i++) {
                if(cpu[0].page_table->page_ids[i] == arg) {
                    p_num++;
                }
            }
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[ACCESS] Page ID[%d] Page Num[%d]\n", cycle, pid, arg, p_num);
        }
        // memory release
        else if(op == 2) {
            for(int i = 0; i < page_num; i++) {
                if(cpu[0].page_table->page_ids[i] == arg) {
                    p_num++;
                }
            }
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[RELEASE] Page ID[%d] Page Num[%d]\n", cycle, pid, arg, p_num);
        }
        // Non-memory instruction
        else if(op == 3) {
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[NON-MEMORY]\n", cycle, pid);
        }
        // Sleep
        else if(op == 4) {
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[SLEEP]\n", cycle, pid);
        }
        // IO Wait
        else if(op == 5) {
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[IOWAIT]\n", cycle, pid);
        }
    }

    // line 2
    int aid;
    fprintf(fout, "%-30s", ">> Physical Memory: ");
    for(int i = 0; i < frame_num; i++) {
        aid = physical_memory[i];
        if(i % 4 == 0) {
            fprintf(fout, "|");
        }
        if(aid == -1) {
            fprintf(fout, "-");
        }
        else {
            fprintf(fout, "%d", aid);
        }
    }
    fprintf(fout, "|\n");

    // line 3 ~
    int valid_bit;
    int reference_bit;
    for(iter = processes->begin(); iter != processes->end(); iter++) {
        pid = iter->pid;
        fprintf(fout, ">> pid(%d)%-20s", pid, " Page Table(PID): ");
        for(int i = 0; i < page_num; i++) {
            page_id = iter->page_table->page_ids[i];
            if(i % 4 == 0) {
                fprintf(fout, "|");
            }
            if(page_id == -1) {
                fprintf(fout, "-");
            }
            else {
                fprintf(fout, "%d", page_id);
            }
        }
        fprintf(fout, "|\n");

        fprintf(fout, ">> pid(%d)%-20s", pid, " Page Table(AID): ");
        for(int i = 0; i < page_num; i++) {
            aid = iter->page_table->allocation_ids[i];
            if(i % 4 == 0) {
                fprintf(fout, "|");
            }
            if(aid == -1) {
                fprintf(fout, "-");
            }
            else {
                fprintf(fout, "%d", aid);
            }
        }
        fprintf(fout, "|\n");

        fprintf(fout, ">> pid(%d)%-20s", pid, " Page Table(Valid): ");
        for(int i = 0; i < page_num; i++) {
            valid_bit = iter->page_table->valid_bits[i];
            if(i % 4 == 0) {
                fprintf(fout, "|");
            }
            if(valid_bit == -1) {
                fprintf(fout, "-");
            }
            else {
                fprintf(fout, "%d", valid_bit);
            }
        }
        fprintf(fout, "|\n");

        fprintf(fout, ">> pid(%d)%-20s", pid, " Page Table(Ref): ");
        for(int i = 0; i < page_num; i++) {
            reference_bit = iter->page_table->reference_bits[i];
            if(i % 4 == 0) {
                fprintf(fout, "|");
            }
            if(reference_bit == -1) {
                fprintf(fout, "-");
            }
            else {
                fprintf(fout, "%d", reference_bit);
            }
        }
        fprintf(fout, "|\n");
    }
    fprintf(fout, "\n");
}


// 출력 이후 상태 갱신
void updateState(Process cpu[], list<Process> *processes, int physical_memory[], int frame_num, int page_num) {
    Process null_process;
    list<Process>::iterator iter;

    // 현재 실행 중인 프로세스가 있을 경우
    if(cpu[0].pid >= 0) {
        cpu[0].run_time++;
        cpu[0].time_quantum--;
        cpu[0].current_index++;

        // 현재 실행 중인 프로세스가 block될 프로세스이거나, 마지막 명령어일 경우
        if(cpu[0].blocked || cpu[0].current_index == cpu[0].instructions.size()) {
            // 마지막 명령어일 경우
            if(cpu[0].current_index == cpu[0].instructions.size()) {
                // 프로세스 종료
                for(iter = processes->begin(); iter != processes->end(); iter++) {
                    if(iter->pid == cpu[0].pid) {
                        processes->erase(iter);
                        break;
                    }
                }

                // Physical memory 할당 해제
                int aid;
                for(int i = 0; i < page_num; i++) {
                    aid = cpu[0].page_table->allocation_ids[i];
                    if(aid != -1) {
                        for(int j = 0; j < frame_num; j++) {
                            if(aid == physical_memory[j]) {
                                physical_memory[j] = -1;
                            }
                        }
                    }
                }
            }
            cpu[0] = null_process;
        }
    }
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

	deque<Process> programs;  // 전체 프로그램 저장
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
            programs.push_back(process);
            pid++;
        }
    }

    // 프로세스 상태 큐
    deque<Process> run_queues[10];  // index = priority
    list<Process> sleep_list;
    list<Process> iowait_list;
    list<Process> processes;  // 종료되지 않은 프로세스

    // 현재 실행 중인 프로세스(기본 pid = -1)
    Process cpu[1];
    Process running_process;
    cpu[0] = running_process;

    int page_num = vm_size / page_size;
    int frame_num = pm_size / page_size;
    // Physical memory
    int physical_memory[frame_num];
    fill_n(physical_memory, frame_num, -1);  // -1로 초기화

    // 출력 파일
    string schedule_file = dir + "/scheduler.txt";
    FILE* fout1 = fopen(schedule_file.c_str(), "w");

    string memory_file = dir + "/memory.txt";
    FILE* fout2 = fopen(memory_file.c_str(), "w");

    int total_process_num = programs.size();
    int cycle = 0;

    // Cycle 시작
    while(total_process_num > 0) {
        checkSleepOver(run_queues, &sleep_list);
        checkIO(run_queues, &ios, &iowait_list, cycle);

        create_process(run_queues, &programs, &processes, cycle, page_num);
        
        schedule(run_queues, cpu);
        
        // 실행할 프로세스가 있을 때
        if(cpu[0].pid >= 0) {
            executeInstruction(cpu, run_queues, &sleep_list, &iowait_list, &processes, physical_memory, page_num, frame_num, page);
        }

        printSchedule(fout1, run_queues, &sleep_list, &iowait_list, cpu, cycle);
        printMemory(fout2, &processes, cpu, physical_memory, cycle, page_num, frame_num);

        updateState(cpu, &processes, physical_memory, frame_num, page_num);

        // 남아있는 프로세스 수
        total_process_num = sleep_list.size() + iowait_list.size();
        for(int i = 0; i < 10; i++) {
            total_process_num += run_queues[i].size();
        }
        if(cpu[0].pid >= 0) {
            total_process_num++;
        }

        cycle++;
    }
    fprintf(fout2, "page fault = %d\n", page_fault);

    // open한 파일 close
	fin.close();
    fclose(fout1);
    fclose(fout2);

	return 0;
}