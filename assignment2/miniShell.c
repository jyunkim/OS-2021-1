#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#define MAX_LINE 128
#define MAX_LENGTH 256

int proc_status;
pid_t pid;

// cd 명령어 수행 함수
bool cmd_cd(int argc, char *argv[]) {
    // cd만 입력 시 home으로 이동
    if(argc == 1) {
        chdir(getenv("HOME"));
    }
    else if(argc == 2) {
        // cd ~ 입력 시 home으로 이동
        if(strcmp(argv[1], "~") == 0) {
            chdir(getenv("HOME"));
        }
        // 디렉토리 이동(성공 시 0 반환)
        else if(chdir(argv[1])) {
            printf("No such directory\n");
        }
    }
    // 인자가 3개 이상일 경우
    else {
        printf("Too many arguments\n");
    }
    return false;
}

// 사용자 입력을 공백을 기준으로 분리하는 함수
int parse_line(char *cmdline, char *argv[]) {
    int count = 0;
    char *token;

    // 첫번째 공백부터 하나씩 분리하여 저장
    token = strtok(cmdline, " ");
    while(token != NULL) {
        argv[count] = token;
        token = strtok(NULL, " ");
        count++;
    }
    argv[count] == NULL;
    return count;
}

// 파싱한 명령어를 실행하는 함수
bool execute(char *cmdline) {
    int i;
    int token_count;
    char *tokens[MAX_LINE];
    bool back = false;

    // 버퍼 초기화
    memset(tokens, '\0', MAX_LINE);

    // exit 입력 시 종료
    if(strncmp(cmdline, "exit\n", 5) == 0) {
        return true;
    }
    
    // 백그라운드 명령
    // 프로그램 종료를 기다리지 않음
    for(i = 0; i < strlen(cmdline); i++) { 
        if(cmdline[i] == '&') {
            cmdline[i] = '\0';
            back = true;
            break;
        }
    }

    // 마지막 문자인 \n을 null로 변환
    cmdline[strlen(cmdline)-1] = '\0';

    // 사용자 입력 공백 기준으로 분리
    token_count = parse_line(cmdline, tokens);

    // Enter만 입력 시 continue
    if(token_count == 0) {
        return false;
    }
    
    // cd 명령어 처리
    else if(strcmp(tokens[0], "cd") == 0) {
        return cmd_cd(token_count, tokens);
    }

    // 자식 프로세스 생성
    pid = fork();
    // 자식 프로세스
    if(pid == 0) {
        // 리다이렉션 검사
        for(i = 0; i < token_count; i++) {
            if(strcmp(tokens[i], ">") == 0) {
                // 임시 파일 식별자
                int out;
                // 출력 파일 open
                out = open(tokens[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                // 예외처리
                if(out < 0) {
                    perror("File open error\n");
                    exit(-1);
                }
                // 표준 출력을 해당 파일로 변경
                dup2(out, 1);
                // 임시 파일 식별자 close
                close(out);
                // 입력 토큰에서 > 제거
                tokens[i] = NULL;
            }
            else if(strcmp(tokens[i], "<") == 0) {
                // 임시 파일 식별자
                int in;
                // 입력 파일 open
                in = open(tokens[i+1], O_RDONLY);
                // 예외처리
                if(in < 0) {
                    perror("File open error\n");
                    exit(-1);
                }
                // 표준 입력을 해당 파일로 변경
                dup2(in, 0);
                // 임시 파일 식별자 close
                close(in);
                // 입력 토큰에서 < 제거
                tokens[i] = NULL;
            }
        }
        execvp(tokens[0], tokens);
        printf("Execution failed\n");
        exit(0);
    }
    // 부모 프로세스
    else if(pid > 0 && back == false) {
        // 부모 프로세스는 자식 프로세스가 끝나길 기다림
        waitpid(pid, &proc_status, 0);
    }
    // fork 에러
    else if(pid < 0) {
        printf("Failed to fork process\n");
        exit(0);
    }
    return false;
}

int main() {
    char cmdline[MAX_LINE];
    time_t timer;
    struct tm* t;
    char *user;
    char cwd[MAX_LENGTH];
    bool done;

    while(!done) {
        // 버퍼 초기화
        memset(cmdline, '\0', MAX_LINE);

        // 현재 시간
        timer = time(NULL);
        t = localtime(&timer);

        // 사용자 이름
        user = getenv("USER");
        // 현재 작업 디렉토리 경로
        getcwd(cwd, MAX_LENGTH);

        printf("[%02d:%02d:%02d]%s@%s$", t->tm_hour, t->tm_min, t->tm_sec, user, cwd);
        
        // 사용자 입력
        fgets(cmdline, MAX_LINE-1, stdin);

        // 명령어 실행
        done = execute(cmdline);
    }
    return 0;
}