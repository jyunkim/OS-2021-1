#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <chrono>

using namespace std;

int *arr;
int *sorted;
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

// 분할
void merge_sort(int left, int right) {
	int mid;

	if(left < right) {
		mid = (left + right) / 2;
        // 왼쪽 절반 분할
		merge_sort(left, mid);
        // 오른쪽 절반 분할
		merge_sort(mid+1, right);
        // 합병 수행
		merge(left, mid, right);
	}
}

int main(int argc, char *argv[]) {
    // 일반 실행
    if(argc == 1) {
        // 첫번째 줄 입력
        cin >> n;
        
        // 입력 값 받을 배열 선언
        arr = new int[n];
        // 합병정렬에 사용할 임시 배열 동적할당
        sorted = new int[n];

        // 두번째 줄 입력 받아 배열에 저장
        for(int i = 0; i < n; i++) {
            cin >> arr[i];
        }

        // 수행 시간 측정 시작
        chrono::system_clock::time_point start = chrono::system_clock::now();

        // 합병정렬 수행
        merge_sort(0, n-1);

        // 수행 시간 측정 종료
        chrono::system_clock::time_point end = chrono::system_clock::now();
        chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(end - start);

        // 정렬된 값 출력
        for(int i = 0; i < n; i++) {
            cout << arr[i] << " ";
        }
        cout << endl << ms.count() << endl;

        // 동적할당 해제
        delete[] sorted;
        delete[] arr;
        
    }
    // program2 실행
    else {
        // 문자열을 정수로 변환
        // 전체 데이터 수
        n = atoi(argv[3]);
        sorted = new int[n];

        // program2에서 만든 공유 메모리 공간 불러옴
        if((shmid = shmget(key, 0, 0666)) < 0) {
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

        // 문자열을 정수로 변환
        // 각 프로세스가 정렬할 index 범위
        int left = atoi(argv[1]);
        int right = atoi(argv[2]);

        // left ~ right 범위 내에서 합병 정렬 수행
        merge_sort(left, right);
        
        // 동적할당 해제
        delete[] sorted;
    }
    return 0;
}