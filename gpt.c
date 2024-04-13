#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

// GCC 컴파일 명령어 템플릿
#define COMPILE_CMD "gcc -fsanitize=address %s -o %s"

// 테스트 실행 및 결과 검증에 필요한 함수 및 로직
void run_tests(const char* input_dir, const char* output_dir, const char* executable, int time_limit) {
    // 여기에 테스트 실행 및 결과 검증 로직 구현
}

int main(int argc, char *argv[]) {
	int opt;
	char *input_dir = NULL;
	char *answer_dir = NULL;
	int time_limit = 0;
	char *source_file = NULL;

	while((opt= getopt(argc,argv,"i:a:t:"))!= -1){
		switch(opt){
			case 'i':
				input_dir = optarg;
				break;
			case 'a':
				answer_dir = optarg;
				break;
			case 't':
				time_limit = atoi(optarg);
				break;
		}
	}

	if (optind < argc) {
    source_file = argv[optind];
  }

	printf("%s",source_file);
	printf("%d",time_limit);
	printf("%s",answer_dir);
	printf("%s",input_dir);
	printf("Well loading\n");

	if (!input_dir || !answer_dir || !time_limit || !source_file) {
		fprintf(stderr, "All options -i, -a, -t and target source file are required.\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
