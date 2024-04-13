#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define COMPILE_CMD "gcc -fsanitize=address %s -o %s"
#define TestFile "test.out"


//함수 원형들 선언
void normalize_string(char *str);
void strip_newline(char *str);
void run_tests(char* input_dir, char* answer_dir, int time_limit , int w_file);
char* openFiles(char *input_dir , char *file_name);
bool compile(int file);
// 방법이없어서 이렇게만들어뒀어요
char txt[10][7] ={"/1.txt","/2.txt","/3.txt","/4.txt","/5.txt","/6.txt","/7.txt","/8.txt","/9.txt","/10.txt"};

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

	if (!input_dir || !answer_dir || !time_limit || !source_file) {
		fprintf(stderr, "All options -i, -a, -t and target source file are required.\n");
		exit(EXIT_FAILURE);
	}
	
	int w_file = open("result.txt", O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR );
	if(w_file == -1){
		printf("fail to open write file");
		return 1;
	}

	write(w_file , "Autojudege.c Result\n" , strlen("Autojudege.c Result\n"));

	// 컴파일하기
	bool isCompile = compile(w_file);
	printf("Compile = %d\n" , isCompile);
	if(isCompile){
		// 여기에서 runtest
		run_tests(input_dir, answer_dir, time_limit , w_file);
		// 런테스트 끝나면 파일삭제
		// execlp("sh","sh","-c","rm ay.out",(char*)NULL);
	}
	
	printf("program well done\n");

	//일단 임시로 파일이있으면 삭제
	if((access(TestFile,F_OK) != -1)){
		execlp("sh","sh","-c","rm test.out",(char*)NULL);
	}

	close(w_file);
	return 0;
}

// 인풋 주소와 파일이름을 넣으면 작동함
char* openFiles(char *input_dir , char *file_name ){
	char path[1024];
	// 아래는 경로만들기
	snprintf(path, sizeof(path), "%s%s" , input_dir , file_name);
	
	FILE* file = fopen(path , "rt");
	if(file == NULL){
		perror("Fail to open file");
		return NULL;
	}
	fseek(file , 0 , SEEK_END);
	long file_size = ftell(file);
	rewind(file);

	char* content = (char*)malloc(file_size + 1);
	if(content == NULL){
		perror("Fail to load file \n");
		exit(EXIT_FAILURE);
	}

	size_t bytes_read = fread(content, 1 , file_size , file);
	if(bytes_read != file_size){
		perror("asadf");
		exit(EXIT_FAILURE);
	}
	content[file_size] = '\0';

	fclose(file);

	return content;
}


bool compile(int w_file){
	bool isCompile;
	char command[256];
	snprintf(command,sizeof(command),COMPILE_CMD,"./target/pathname.c",TestFile);
	
	pid_t pid =fork();
	if(pid == 0){
    dup2(w_file, STDERR_FILENO);
		close(w_file);
		execlp("sh","sh","-c",command,(char*)NULL);
		perror("execlp");
		printf("If see this fail to command\n");
		exit(EXIT_FAILURE);
	}else if(pid > 0){
		wait(NULL);
		printf("success to delete child\n");

		// 파일이 생성됬는지로 컴파일유무 확인
		if((access(TestFile,F_OK) != -1)){
			write(w_file , "Compile >> Success !\n" , strlen("Compile >> Success !\n"));		
			printf("compile >> Success \n");
			return true;
		}else{
			write(w_file , "Compile >> Error \n" , strlen("Compile >> Error \n"));	
			printf("Compile >> Error \n");
			return false;
		}

	}else{
		perror("Fail to fork\n");
		exit(EXIT_FAILURE);
	}
}

void run_tests(char* input_dir, char* answer_dir, int time_limit , int w_file) {
	char results[1024] = "";

	for (int i = 0; i < 6; i++) {
		// 실제로는 10으로 해야함 7에서 런타임에러 발생
		char* input_content = openFiles(input_dir, txt[i]);
		char* answer_content = openFiles(answer_dir, txt[i]);
		char buffer[1024] = {0};
		strip_newline(input_content);
		strip_newline(answer_content);
		printf("Test case %d:\n", i + 1);
		int pipefd[2];

		char command[256];
		snprintf(command,sizeof(command),"./%s < %s%s",TestFile, input_dir,txt[i]);

		if(pipe(pipefd) == -1){
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		pid_t pid = fork();

		if(pid == 0){
			close(pipefd[0]);  
			dup2(pipefd[1], STDOUT_FILENO); 
			close(pipefd[1]);

			execlp("sh","sh","-c",command,(char*)NULL);
			perror("execlp");  
			exit(EXIT_FAILURE);
		}else if(pid > 0){
			close(pipefd[1]);
			memset(buffer , 0 , sizeof(buffer));
			ssize_t num_read = read(pipefd[0], buffer, sizeof(buffer)-1);
			if (num_read > 0) {
					buffer[num_read] = '\0';

					printf("buffertext: \n%s", buffer);
					printf("answer_content: \n%s",answer_content);
					// 답이 맞는지 검증
					snprintf(results + strlen(results), sizeof(results) - strlen(results), "Q%d. >> %s\n", i + 1, (strcmp(buffer, answer_content) == 0 ? "correct" : "wrong"));
					printf("-> %d\n",strcmp(buffer,answer_content));
					printf("-> %d\n",strncmp(buffer,answer_content, strlen(buffer)));
					printf("buffer length - %ld\n", strlen(buffer));
					printf("answer length - %ld\n", strlen(answer_content));
					    int equal = 1; // 기본적으로 두 문자열이 같다고 가정

					// 두 문자열의 각 문자를 비교
					for (int i = 0; buffer[i] != '\0' && answer_content[i] != '\0'; ++i) {
							if (buffer[i] != answer_content[i]) {
									equal = 0; // 두 문자열이 다르면 equal을 0으로 설정하고 반복문 종료
									printf("problem here -> %d\n",i);
									break;
							}
					}

					if(strcmp(buffer,answer_content)){
						printf("Q%d. >> collect\n",i+1);
					}else{
						printf("Q%d. >> wrong\n",i+1);
						
					}
			}
			close(pipefd[0]); 
			wait(NULL);  
		}else{
			perror("fork");
			exit(EXIT_FAILURE);
		}
		free(input_content);
		free(answer_content);
	}
	write(w_file , results , strlen(results));
}

void normalize_string(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
}

void strip_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}