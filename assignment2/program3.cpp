#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <chrono>

using namespace std;

int *arr;
int *sorted;
int *group;
int total_thread_num;
int count;
int n;
int num = 0;

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

// 스레드 별 합병 정렬 시작
void *start_merge_sort(void *arg) {
    // 스레드에게 0부터 번호 부여
    int thread_id = num++;
    
    // 각 스레드가 정렬할 범위 할당
    int left = group[thread_id];
    int right = group[thread_id+1] - 1;

    // 마지막 스레드
    if(thread_id == total_thread_num - 1){
        right = n - 1;
    }

    merge_sort(left, right);
    pthread_exit(0);
}

// 데이터를 분할하여 각 스레드에 분배
void divide(int n, int total_thread_num) {
    count = 1;

    // 전체 스레드 수 이상의 그룹이 생성될 때까지 반복
    while(count < total_thread_num) {
        count *= 2;
    }
    int group_size = n / total_thread_num;

    // 각 스레드에 시작 index 할당
    for(int i = 0; i < total_thread_num; i++) {
        group[i] = i * group_size;
    }
}

int main(int argc, char *argv[]) {
    // 첫번째 줄 입력
    cin >> n;

    // 입력 값 받을 배열 선언
    arr = new int[n];
    // 합병정렬에 사용할 임시 배열 동적할당
    sorted = new int[n];

    // 스레드 수 입력 받아옴
    total_thread_num = atoi(argv[1]);

    // 스레드 id 배열 선언
    pthread_t tid[total_thread_num];

    // 두번째 줄 입력 받아 배열에 저장
    for(int i = 0; i < n; i++) {
        cin >> arr[i];
    }
    
    // 각 스레드가 정렬할 시작 index를 저장할 배열 선언
    group = new int[total_thread_num];

    // 수행 시간 측정 시작
    chrono::system_clock::time_point start = chrono::system_clock::now();

    // 데이터 분배
    divide(n, total_thread_num);

    // 각 스레드마다 정렬 범위(left ~ right)를 가지고 정렬 수행
    for(int i = 0; i < total_thread_num; i++) {
        if(pthread_create(&tid[i], NULL, start_merge_sort, NULL) < 0) {
            cout << "Failed to create thread" << endl;
            exit(1);
        }
    }

    // 생성한 스레드가 종료될 때까지 기다림
    for(int i = 0; i < total_thread_num; i++) {
        pthread_join(tid[i], NULL);
    }

    // 각 스레드가 정렬한 부분을 수합하여 전체 정렬
    int left, right;
    int group_size = n / total_thread_num;
    int k = total_thread_num;
    
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

    // 동적할당 해제
    delete[] sorted;
    delete[] group;
    delete[] arr;
    return 0;
}