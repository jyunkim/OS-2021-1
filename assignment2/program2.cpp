#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <chrono>

using namespace std;

int *arr;
int *sorted;
int *group;
int total_process_num;
int count;
int n;
int shmid;
key_t key = 1234;
void *shm_array;

// 합병
void merge(int left, int mid, int right) {
    // 왼쪽 절반 시작 index
	int i = left;
    // 오른쪽 절반 시작 index
	int j = mid + 1;
    // 임시 배열 시작 index
	int k = left;

    // 왼쪽, 오른쪽 절반을 처음부터 비교하여 크기가 큰 값을 임시 배열에 저장
	while(i <= mid && j <= right) {
		if(arr[i] >= arr[j]) {
			sorted[k++] = arr[i++];
        }
		else {
			sorted[k++] = arr[j++];
        }
	}

    // 오른쪽 절반에 남아 있는 값들을 일괄 저장
    if(i > mid) {
        for(int l = j; l <= right; l++)
            sorted[k++] = arr[l];
    }
    // 왼쪽 절반에 남아 있는 값들을 일괄 저장
    else {
        for(int l = i; l <= mid; l++)
            sorted[k++] = arr[l];
    }

    // 임시 배열에 저장된 값을 원래 배열에 복사
	for(int l = left; l <= right; l++){
        arr[l] = sorted[l];
    }
}

// 데이터를 분할하여 각 프로세스에 분배
void divide(int n, int total_process_num) {
    count = 1;
    
    // 전체 프로세스 수 이상의 그룹이 생성될 때까지 반복
    while(count < total_process_num) {
        count *= 2;
    }
    int group_size = n / total_process_num;

    // 각 프로세스에 시작 index 할당
    for(int i = 0; i < total_process_num; i++) {
        group[i] = i * group_size;
    }
}

int main(int argc, char *argv[]) {
    int left, right;
    char s_left[10000];
    char s_right[10000];
    char s_n[10000];

    // 첫번째 줄 입력
    cin >> n;

    // 합병정렬에 사용할 임시 배열 동적할당
    sorted = new int[n];

    // 자식 프로세스 수 입력 받아옴
    total_process_num = atoi(argv[1]);
    // 프로세스 id 배열 선언
    pid_t pid[total_process_num];

    // 공유 메모리 크기 설정
    size_t SHM_SIZE = sizeof(int) * n;
 
    // 공유 메모리 공간 생성
    if((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        _exit(1);
    }

    // 공유 메모리를 프로세스 메모리에 할당
    if((shm_array = shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("shmat");
        _exit(1);
    }

    // 공유 메모리 배열을 정렬에 사용할 배열에 할당
    arr = (int *)shm_array;

    // 두번째 줄 입력 받아 배열에 저장
    for(int i = 0; i < n; i++) {
        cin >> arr[i];
    }

    // 각 프로세스가 정렬할 시작 index를 저장할 배열 선언
    group = new int[total_process_num];

    // 수행 시간 측정 시작
    chrono::system_clock::time_point start = chrono::system_clock::now();

    // 데이터 분배
    divide(n, total_process_num);
    
    // 입력받은 수만큼 자식 프로세스 생성
    for(int i = 0; i < total_process_num; i++) {
        pid[i] = fork();

        if(pid[i] < 0) {
            perror("Failed to fork process\n");
            _exit(-1);
        }
        // 각 프로세스마다 prgram1으로 정렬 범위(left ~ right) 전달하여 실행
        else if(pid[i] == 0) {
            if(i == total_process_num - 1) {
                left = group[i];
                right = n - 1;
            }
            else {
                left = group[i];
                right = group[i+1] - 1;
            }
            // 정수를 문자열로 변환
            sprintf(s_left, "%d", left);
            sprintf(s_right, "%d", right);
            sprintf(s_n, "%d", n);
            execlp("./program1", "./program1", s_left, s_right, s_n, (char *)0);
        }
    }

    // 각 자식 프로세스가 끝날 때까지 기다림
    int status;
    for(int i = 0; i < total_process_num; i++) {
        waitpid(pid[i], &status, 0);
    }

    // 각 프로세스가 정렬한 부분을 수합하여 전체 정렬
    int group_size = n / total_process_num;
    int k = total_process_num;
    
    while(count > 2) {
        for(int i = 0; i < k - 1; i += 2) {
            if(i == k - 2) {
                left = i * group_size;
                right = n - 1;
                merge(left, left+group_size-1, right);
            }
            else {
                left = i * group_size;
                right = left + 2 * group_size - 1;
                merge(left, (left+right)/2, right);
            }
        }
        group_size *= 2;
        count /= 2;
        k /= 2;
    }
    merge(0, group_size-1, n-1);

    // 수행 시간 측정 종료
    chrono::system_clock::time_point end = chrono::system_clock::now();
    chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(end - start);

    // 정렬된 값 출력
	for(int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << endl << ms.count() << endl;

    // 공유 메모리를 프로세스에서 떼어냄
    if(shmdt(shm_array) == -1) {
        perror("shmdt");
        _exit(1);
    }
 
    // 공유 메모리 제거
    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        _exit(1);
    }

    // 동적할당 해제
    delete[] sorted;
    delete[] group;
    return 0;
}