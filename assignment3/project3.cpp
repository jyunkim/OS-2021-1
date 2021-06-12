#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <deque>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

int aid = 0;  // allocation id
int page_fault = 0;  // page fault 발생 수
list<int> lru_stack;  // lru stack
int clock_hand = -1;  // allocation id를 가리키는 clock

// Page Table
class PageTable {
public:
    int *page_ids;
    int *valid_bits;
    int *allocation_ids;
    int *reference_bits;
    vector<pair<int, deque<int>>> reference_bytes;

    PageTable() {}
    
    PageTable(int page_num) {
        // 여러 종류의 bit를 담는 배열을 page 개수 크기로 생성
        page_ids = new int[page_num];
        valid_bits = new int[page_num];
        allocation_ids = new int[page_num];
        reference_bits = new int[page_num];

        // 배열 요소 초기화
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

    PageTable *page_table;  // Page table
    int page_id = 0;  // 다음에 할당할 page id
    int release_num = 0;  // Release한 page 수

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

        ifstream fin1;
        fin1.open(file_name);
        fin1 >> total_instruction_num;

        for(int i = 0; i < total_instruction_num; i++) {
            fin1 >> opcode >> arg;
            instructions.push_back(make_pair(opcode, arg));
        }
        fin1.close();
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


// 분할된 frame
class Node {
public:
    Node *left = nullptr;
    Node *right = nullptr;
    Node *parent = nullptr;
    int start;
    int end;
    int size;
    bool free = true;

    Node() {}

    Node(int frame_num) {
        start = 0;
        end = frame_num - 1;
        size = frame_num;
    }

    Node(int start_, int end_) {
        start = start_;
        end = end_;
        size = end - start + 1;
    }
};


// 정렬에 사용할 비교 함수
bool compare(Node *n1, Node *n2) {
    // 크기가 같으면 시작 주소가 작은 순서
    if(n1->size == n2->size) {
        return n1->start < n2->start;
    }
    // 크기가 작은 순서
    else {
        return n1->size < n2->size;
    }
}


// 분할된 frame들을 담는 트리
class Tree {
public:
    Node *root;
    Node *cur;
    list<Node*> all_nodes;

    Tree() {}

    Tree(int frame_num) {
        root = new Node(frame_num);
        all_nodes.push_back(root);
        cur = root;
    }

    // 분할
    void divide(Node *n) {
        int mid = (n->end + n->start) / 2;
        n->left = new Node(n->start, mid);
        n->right = new Node(mid+1, n->end);

        n->left->parent = n;
        n->right->parent = n;

        all_nodes.push_back(n->left);
        all_nodes.push_back(n->right);

        n->free = false;
    }

    // 병합
    void merge(Node *n) {
        all_nodes.remove(n->left);
        all_nodes.remove(n->right);
        n->left = nullptr;
        n->right = nullptr;

        n->free = true;
    }

    // Free frame을 담은 리스트 반환
    vector<Node*> getLeaves() {
        vector<Node*> leaves;
        list<Node*>::iterator iter;
        for(iter = all_nodes.begin(); iter != all_nodes.end(); iter++) {
            // 자식 노드가 항상 2개씩 생기므로 right만 검사
            if((*iter)->right == nullptr && (*iter)->free) {
                leaves.push_back(*iter);
            }
        }
        // 크기, 주소 오름차순 정렬
        sort(leaves.begin(), leaves.end(), compare);
        return leaves;
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


// Reference byte 갱신
void updateReferenceByte(Process cpu[], list<Process> *processes, deque<Process> run_queues[], list<Process> *sleep_list, list<Process> *iowait_list, int page_num) {
    list<Process>::iterator iter;
    int page_id;
    deque<int> *reference_byte;

    for(iter = processes->begin(); iter != processes->end(); iter++) {
        // reference byte 갱신
        for(int i = 0; i < iter->page_table->reference_bytes.size(); i++) {
            page_id = iter->page_table->reference_bytes[i].first;
            reference_byte = &(iter->page_table->reference_bytes[i].second);
            // reference bytes에서 가지고 있는 page id와 같은 page id를 가진 page table의 reference bit을 앞에 삽입
            for(int j = 0; j < page_num; j++) {
                if(iter->page_table->page_ids[j] == page_id) {
                    reference_byte->push_front(iter->page_table->reference_bits[j]);
                    reference_byte->pop_back();
                    break;
                }
            }
        }
        // reference bit 초기화
        for(int i = 0; i < page_num; i++) {
            if(iter->page_table->page_ids[i] != -1) {
                iter->page_table->reference_bits[i] = 0;
            }
        }
    }
}


// Memory Allocation 명령어 수행
void memoryAllocation(Process cpu[], list<Process> *processes, int page_num) {
    list<Process>::iterator iter;
    int page_id = cpu[0].page_id;
    int *pid = cpu[0].page_table->page_ids;
    int i;
    int start = 0;
    bool free = false;

    // 할당 가능한 공간 탐색
    while(!free) {
        for(i = start; i < start + page_num; i++) {
            if(pid[i] != -1) {
                break;
            }
        }
        // 해당 범위가 모두 비어있으면
        if(i == start + page_num) {
            free = true;
        }
        // 해당 범위에 하나라도 차있으면 이후 범위 재탐색
        else {
            start = i + 1;
        }
    }
    
    // 할당할 page 수만큼 page table 입력
    for(int i = start; i < start + page_num; i++) {
        pid[i] = page_id;
        cpu[0].page_table->valid_bits[i] = 0;
        cpu[0].page_table->reference_bits[i] = 0;
    }
    cpu[0].page_id++;

    // processes 리스트도 업데이트
    for(iter = processes->begin(); iter != processes->end(); iter++) {
        if(iter->pid == cpu[0].pid) {
            iter->page_id++;
            break;
        }
    }

    // Reference byte 생성
    deque<int> dq(8);
    cpu[0].page_table->reference_bytes.push_back(make_pair(page_id, dq));
}


// Reference byte의 크기 측정
int byteToInt(deque<int> reference_byte) {
    int sum = 0;
    int magnitude = pow(2, 7);
    for(int i = 0; i < 8; i++) {
        sum += reference_byte[i] * magnitude;
        magnitude /= 2;
    }
    return sum;
}


// 가장 작은 reference byte를 가진 page의 allocation id 반환
int sampledLru(list<Process> *processes, int physical_memory[], int page_num, int frame_num) {
    int my_aid;
    int victim_aid;
    int min_byte = 256;
    int page_id;
    int tmp;
    deque<int> reference_byte;
    list<Process>::iterator iter;

    for(iter = processes->begin(); iter != processes->end(); iter++) {
        for(int i = 0; i < iter->page_table->reference_bytes.size(); i++) {
            page_id = iter->page_table->reference_bytes[i].first;
            reference_byte = iter->page_table->reference_bytes[i].second;
            // 해당 page id의 allocation id 찾음
            for(int j = 0; j < page_num; j++) {
                if(iter->page_table->page_ids[j] == page_id) {
                    my_aid = iter->page_table->allocation_ids[j];
                    break;
                }
            }
            // 할당된 적이 있으면
            if(my_aid != -1) {
                // 해당 allocation id가 physical memory에 있는지 확인
                for(int j = 0; j < frame_num; j++) {
                    if(physical_memory[j] == my_aid) {
                        tmp = byteToInt(reference_byte);
                        // 그 중 가장 작은 byte 선택
                        if(tmp < min_byte) {
                            min_byte = tmp;
                            victim_aid = my_aid;
                        }
                        // 크기가 같으면 allocation id가 작은 것 선택
                        else if(tmp == min_byte) {
                            if(my_aid < victim_aid) {
                                min_byte = tmp;
                                victim_aid = my_aid;
                            }
                        }
                    }
                }
            }
        }
    }
    return victim_aid;
}


// Clock algorithm
int clockLru(list<Process> *processes, int page_num, int my_aid) {
    list<Process>::iterator iter;

    // 이전 victim aid 다음 aid부터 시작
    clock_hand++;
    if(clock_hand >= aid) {
        clock_hand = 0;
    }

    while(true) {
        for(iter = processes->begin(); iter != processes->end(); iter++) {
            for(int i = 0; i < page_num; i++) {
                if(iter->page_table->allocation_ids[i] == clock_hand && iter->page_table->valid_bits[i] == 1 && clock_hand != my_aid) {
                    if(iter->page_table->reference_bits[i] == 0) {
                        return clock_hand;
                    }
                    else if(iter->page_table->reference_bits[i] == 1) {
                        iter->page_table->reference_bits[i] = 0;
                    }
                }
            }
        }
        clock_hand++;
        if(clock_hand >= aid) {
            clock_hand = 0;
        }
    }
}


// Memory Access 명령어 수행
void memoryAccess(Process cpu[], list<Process> *processes, int physical_memory[], Tree *buddy, int page_id, int page_num, int frame_num, string algorithm) {
    list<Process>::iterator iter;
    int p_num = 0;
    int buddy_size = frame_num;
    int my_aid = aid;
    bool update_aid = true;
    bool is_page_fault = true;

    // Reference bit 업데이트
    for(int i = 0; i < page_num; i++) {
        if(cpu[0].page_table->page_ids[i] == page_id) {
            cpu[0].page_table->reference_bits[i] = 1;
        }
    }

    // 해당 page id에 해당하는 allocation id 검색
    for(int i = 0; i < page_num; i++) {
        if(cpu[0].page_table->page_ids[i] == page_id && cpu[0].page_table->allocation_ids[i] != -1) {
            my_aid = cpu[0].page_table->allocation_ids[i];
            update_aid = false;
            break;
        }
    }
    // 이전에 할당한 적이 없으면 aid + 1
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

        // Page table 갱신
        // 주어진 page id에 해당하는 page table 내 page 수 계산
        // 해당 page의 allocation id 할당
        for(int i = 0; i < page_num; i++) {
            if(cpu[0].page_table->page_ids[i] == page_id) {
                p_num++;
                cpu[0].page_table->allocation_ids[i] = my_aid;
                cpu[0].page_table->valid_bits[i] = 1;
            }
        }

        // Buddy system에 의해 할당할 frame 수 계산
        while(p_num <= buddy_size/2) {
            buddy_size /= 2;
        }
        
        bool available = false;
        vector<Node*> free_frames = buddy->getLeaves();  // Free frames
        Node *target_frame;

        // 요청된 page를 수용 가능한 공간 탐색
        for(int i = 0; i < free_frames.size(); i++) {
            // 크기로 오름차순 정렬되어 있기 때문에 할당해야 되는 frame 수 이상이면 선택
            if(buddy_size <= free_frames[i]->size) {
                target_frame = free_frames[i];
                available = true;
                break;
            }
        }

        // 할당 가능한 공간이 있으면
        if(available) {
            // Buddy system에 의해 공간 분할
            while(buddy_size != target_frame->size) {
                buddy->divide(target_frame);
                target_frame = target_frame->left;
            }

            // Frame 크기가 같아지면 physical memory frame 할당
            target_frame->free = false;
            for(int i = target_frame->start; i <= target_frame->end; i++) {
                physical_memory[i] = my_aid;
            }
        }

        // 할당 가능한 frame이 없다면 페이지 교체 알고리즘 수행
        while(!available) {
            int victim_aid;
            // lru 스택 앞에서 victim 가져옴
            if(algorithm == "lru") {
                victim_aid = lru_stack.front();
                lru_stack.pop_front();
            }
            else if(algorithm == "sampled") {
                victim_aid = sampledLru(processes, physical_memory, page_num, frame_num);
            }
            else if(algorithm == "clock") {
                victim_aid = clockLru(processes, page_num, my_aid);
            }

            int start;
            int count = 0;
            // Victim swap out
            for(int i = 0; i < frame_num; i++) {
                if(physical_memory[i] == victim_aid) {
                    if(count == 0) {
                        start = i;
                    }
                    physical_memory[i] = -1;
                    count++;
                }
            }
            int end = start + count - 1;

            // 할당했던 frame을 free frame으로 변환
            list<Node*>::iterator iter2;
            for(iter2 = buddy->all_nodes.begin(); iter2 != buddy->all_nodes.end(); iter2++) {
                if((*iter2)->start == start && (*iter2)->end == end) {
                    (*iter2)->free = true;
                    // Root 노드 일 경우
                    if((*iter2)->parent == nullptr) {
                        target_frame = (*iter2);
                    }
                    else {
                        target_frame = (*iter2)->parent;
                    }
                    break;
                }
            }

            // Buddy 병합
            // 트리에 root 노드 하나만 있을 경우 병합 x
            while(buddy->all_nodes.size() > 1 && target_frame->left->free && target_frame->right->free) {
                buddy->merge(target_frame);
                // Root 노드면 중단
                if(target_frame->parent == nullptr) {
                    break;
                }
                else {
                    target_frame = target_frame->parent;
                }
            }

            // Victim aid를 가진 프로세스의 page table 수정
            for(iter = processes->begin(); iter != processes->end(); iter++) {
                for(int i = 0; i < page_num; i++) {
                    if(iter->page_table->allocation_ids[i] == victim_aid) {
                        iter->page_table->valid_bits[i] = 0;
                        iter->page_table->reference_bits[i] = 0;
                    }
                }
            }

            // 요청된 page를 수용할 수 있는지 확인
            free_frames = buddy->getLeaves();
            for(int i = 0; i < free_frames.size(); i++) {
                if(buddy_size <= free_frames[i]->size) {
                    target_frame = free_frames[i];
                    available = true;
                    break;
                }
            }

            // 할당 가능한 공간이 있으면
            if(available) {
                // Buddy system에 의해 공간 분할
                while(buddy_size != target_frame->size) {
                    buddy->divide(target_frame);
                    target_frame = target_frame->left;
                }

                // Physical memory frame 할당
                target_frame->free = false;
                for(int i = target_frame->start; i <= target_frame->end; i++) {
                    physical_memory[i] = my_aid;
                }
            }
        }
    }

    // lru 스택 업데이트
    // Access된 page의 aid를 스택 뒤에 삽입
    lru_stack.remove(my_aid);
    lru_stack.push_back(my_aid);
}


// Memory release 명령어 수행
void memoryRelease(Process cpu[], list<Process> *processes, int physical_memory[], Tree *buddy, int page_id, int page_num, int frame_num, string algorithm) {
    int my_aid;
    int count = 0;
    list<Process>::iterator iter;

    // Virtual memory에서 해제
    for(int i = 0; i < page_num; i++) {
        if(cpu[0].page_table->page_ids[i] == page_id) {
            my_aid = cpu[0].page_table->allocation_ids[i];
            cpu[0].page_table->page_ids[i] = -1;
            cpu[0].page_table->allocation_ids[i] = -1;
            cpu[0].page_table->valid_bits[i] = -1;
            cpu[0].page_table->reference_bits[i] = -1;
            count++;
        }
    }
    cpu[0].release_num = count;

    // Physical memory에서 해당 allocation id 해제
    int start;
    count = 0;
    for(int i = 0; i < frame_num; i++) {
        if(physical_memory[i] == my_aid) {
            if(count == 0) {
                start = i;
            }
            physical_memory[i] = -1;
            count++;
        }
    }
    int end = start + count - 1;

    // 할당했던 frame을 free frame으로 변환
    list<Node*>::iterator iter2;
    Node *target_frame;
    for(iter2 = buddy->all_nodes.begin(); iter2 != buddy->all_nodes.end(); iter2++) {
        if((*iter2)->start == start && (*iter2)->end == end) {
            (*iter2)->free = true;
            // Root 노드 일 경우
            if((*iter2)->parent == nullptr) {
                target_frame = (*iter2);
            }
            else {
                target_frame = (*iter2)->parent;
            }
            break;
        }
    }

    // Buddy 병합
    // 트리에 root 노드 하나만 있을 경우 병합 x
    while(buddy->all_nodes.size() > 1 && target_frame->left->free && target_frame->right->free) {
        buddy->merge(target_frame);
        // Root 노드면 중단
        if(target_frame->parent == nullptr) {
            break;
        }
        else {
            target_frame = target_frame->parent;
        }
    }

    // lru stack에서 제거
    lru_stack.remove(my_aid);

    // reference byte 제거
    vector<pair<int, deque<int>>>::iterator iter4;
    for(iter4 = cpu[0].page_table->reference_bytes.begin(); iter4 != cpu[0].page_table->reference_bytes.end(); iter4++) {
        if(iter4->first == page_id) {
            cpu[0].page_table->reference_bytes.erase(iter4);
            break;
        }
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
void executeInstruction(Process cpu[], deque<Process> run_queues[], list<Process> *sleep_list, list<Process> *iowait_list, list<Process> *processes, int physical_memory[], Tree *buddy, int page_num, int frame_num, string algorithm) {
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
        memoryAccess(cpu, processes, physical_memory, buddy, arg, page_num, frame_num, algorithm);
    }
    // Memory release
    else if(opcode == 2) {
        memoryRelease(cpu, processes, physical_memory, buddy, arg, page_num, frame_num, algorithm);
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
void printMemory(FILE *fout, list<Process> *processes, Process cpu[], int physical_memory[], int cycle, int page_num, int frame_num, string algorithm) {
    int pid = cpu[0].pid;
    const char *name;
    int current_index;
    int op;
    int arg;
    int page_id;
    int p_num = 0;
    int release_num;
    list<Process>::iterator iter;

    // 실행 중인 프로세스가 있을 경우
    if(pid >= 0) {
        name = cpu[0].name.c_str();
        current_index = cpu[0].current_index;
        op = cpu[0].instructions[current_index].first;
        arg = cpu[0].instructions[current_index].second;
        page_id = cpu[0].page_id - 1;
        release_num = cpu[0].release_num;
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
            fprintf(fout, "[%d Cycle] Input: Pid[%d] Function[RELEASE] Page ID[%d] Page Num[%d]\n", cycle, pid, arg, release_num);
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
    int my_aid;
    fprintf(fout, "%-29s", ">> Physical Memory: ");
    for(int i = 0; i < frame_num; i++) {
        my_aid = physical_memory[i];
        if(i % 4 == 0) {
            fprintf(fout, "|");
        }
        if(my_aid == -1) {
            fprintf(fout, "-");
        }
        else {
            fprintf(fout, "%d", my_aid);
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
            my_aid = iter->page_table->allocation_ids[i];
            if(i % 4 == 0) {
                fprintf(fout, "|");
            }
            if(my_aid == -1) {
                fprintf(fout, "-");
            }
            else {
                fprintf(fout, "%d", my_aid);
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
        if(algorithm == "lru") {
            for(int i = 0; i < page_num; i++) {
                if(i % 4 == 0) {
                    fprintf(fout, "|");
                }
                fprintf(fout, "-");
            }
        }
        else {
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
        }
        fprintf(fout, "|\n");
    }
    fprintf(fout, "\n");
}


// 출력 이후 상태 갱신
void updateState(Process cpu[], list<Process> *processes, int physical_memory[], Tree *buddy, int frame_num, int page_num) {
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
                // 프로세스 종료 - processes 리스트에서 제거
                for(iter = processes->begin(); iter != processes->end(); iter++) {
                    if(iter->pid == cpu[0].pid) {
                        processes->erase(iter);
                        break;
                    }
                }

                // Physical memory 할당 해제
                int my_aid;
                for(int i = 0; i < page_num; i++) {
                    my_aid = cpu[0].page_table->allocation_ids[i];
                    if(my_aid != -1) {
                        int start;
                        int count = 0;

                        for(int j = 0; j < frame_num; j++) {
                            if(physical_memory[j] == my_aid) {
                                if(count == 0) {
                                    start = j;
                                }
                                physical_memory[j] = -1;
                                count++;
                            }
                        }

                        // Physical frame에 해당 aid가 없으면 pass
                        if(count == 0) {
                            continue;
                        }
                        int end = start + count - 1;

                        // 할당했던 frame을 free frame으로 변환
                        list<Node*>::iterator iter2;
                        Node *target_frame;
                        for(iter2 = buddy->all_nodes.begin(); iter2 != buddy->all_nodes.end(); iter2++) {
                            if((*iter2)->start == start && (*iter2)->end == end) {
                                (*iter2)->free = true;
                                // Root 노드 일 경우
                                if((*iter2)->parent == nullptr) {
                                    target_frame = (*iter2);
                                }
                                else {
                                    target_frame = (*iter2)->parent;
                                }
                                break;
                            }
                        }

                        // Buddy 병합
                        // 트리에 root 노드 하나만 있을 경우 병합 x
                        while(buddy->all_nodes.size() > 1 && target_frame->left->free && target_frame->right->free) {
                            buddy->merge(target_frame);
                            // Root 노드면 중단
                            if(target_frame->parent == nullptr) {
                                break;
                            }
                            else {
                                target_frame = target_frame->parent;
                            }
                        }
                        
                        // lru stack에서 제거
                        lru_stack.remove(my_aid);
                    }
                }
            }
            cpu[0] = null_process;
        }
    }
}


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
        int name_size = option.find("=") + 1;
        int detail_length = option.size() - name_size;
        string opt_name = option.substr(0, name_size);
        string opt_detail = option.substr(name_size, detail_length);

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
            string program_file = dir + "/" + code;
            process.addInstruction(program_file);
            programs.push_back(process);
            pid++;
        }
    }

    list<Process> processes;  // 종료되지 않은 프로세스

    // 프로세스 상태 큐
    deque<Process> run_queues[10];  // index = priority
    list<Process> sleep_list;
    list<Process> iowait_list;

    // 현재 실행 중인 프로세스(기본 pid = -1)
    Process cpu[1];
    Process running_process;
    cpu[0] = running_process;

    // Page, frame 총 개수
    int page_num = vm_size / page_size;
    int frame_num = pm_size / page_size;

    // Physical memory
    int physical_memory[frame_num];
    fill_n(physical_memory, frame_num, -1);  // -1로 초기화

    // Buddy가 담긴 트리
    Tree buddy(frame_num);

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
        
        // Sampled lru일 경우 time interval(8 cycle) 마다 reference byte 갱신
        if(page == "sampled" && cycle % 8 == 0) {
            updateReferenceByte(cpu, &processes, run_queues, &sleep_list, &iowait_list, page_num);
        }
        
        // 실행할 프로세스가 있을 때
        if(cpu[0].pid >= 0) {
            executeInstruction(cpu, run_queues, &sleep_list, &iowait_list, &processes, physical_memory, &buddy, page_num, frame_num, page);
        }

        printSchedule(fout1, run_queues, &sleep_list, &iowait_list, cpu, cycle);
        printMemory(fout2, &processes, cpu, physical_memory, cycle, page_num, frame_num, page);

        updateState(cpu, &processes, physical_memory, &buddy, frame_num, page_num);

        // 남아있는 프로세스 수
        total_process_num = processes.size() + programs.size();

        cycle++;
    }
    fprintf(fout2, "page fault = %d\n", page_fault);

    // open한 파일 close
	fin.close();
    fclose(fout1);
    fclose(fout2);

	return 0;
}