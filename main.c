#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define optionR 0
#define optionI 0
#define optionD 0
#define optionL 0

void tree(char *pathname,char *dirName, int depth);
void delete(char *filename, char *date, char *deleteTime, char *option);
void size();
void recover(char *filename, char *option);
void help();
void menu(char *list[5], int count);
void delete_alarm(int signo);
void sortInfo(char*, char*);
int checkInfoSize(char *infoPath, char *filesPath);
int isSelect(char *filename, char *get, int check);

char path[200];
char forDelete[200];
char forDeleteFile[20];

typedef struct infoFile {
	char name[20];
	time_t tmm;
}infoFile;

static int filter(const struct dirent *dirent)
{
	if(!strncmp(dirent->d_name,".",1))
		return 0;
	else
		return 1;
}

int main() 
{
	pid_t pid;
	int count = 0;
	char getstring[100];
	char *get[5] = {0};

	if((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid == 0) {
		execl("./ssu_daemon", "ssu_daemon",(char *)0);
		fprintf(stderr, "execl error\n");
		exit(0);
	}
	

	getcwd(path, 200);

	while(1) {
		printf("20172618>");
		fgets(getstring, 100, stdin); //명령어 및 옵션들을 받아온다
		
		if(!strcmp(getstring, "\n"))
			continue;

		get[count] = strtok(getstring, " "); //공백을 기준으로 자른다
		
		

		while(get[count] != NULL) {  // 더이상 자를게 없을때까지 잘라준다
			count++;
			get[count] = strtok(NULL, " ");
		}
		
		menu(get,count); //menu함수 호출

		count = 0;
	}

	return 0;
}

void menu(char *list[5], int count)
{
	char pathname[1024];
 	
	if(!strncmp(list[0], "delete",6)) {
		
		memset(forDelete, '\0', sizeof(forDelete));
		memset(forDeleteFile, '\0', sizeof(forDeleteFile));
		
		if(list[1] != NULL) {
			delete(list[1],list[2],list[3],list[4]);
		}
		else {
			printf("파일이름을 주어야 합니다.\n");
			return;
		}
	}
	else if(!strncmp(list[0], "size",4))
		size();
	else if(!strncmp(list[0], "recover",7)) {

		if(list[1] != NULL) 
			recover(list[1], list[2]); 
		else {
			printf("파일 이름을 주어야 합니다.\n");
			return;
		}
	}
	else if(!strncmp(list[0], "tree",4)) {
		sprintf(pathname,"%s/%s",path,"check");
		tree(pathname,"check", 0);
		printf("\n");
		return;
	}
	else if(!strncmp(list[0], "exit",4)) {
		fflush(stdout);
		fflush(stdin);
		exit(0);
	}
	else if(!strncmp(list[0], "help",4))
		help();
	else {
		printf("명령어를 정확히 입력해 주세요.\n");
		return;
	}
}

void help()
{
	printf("명령어 사용법\n");
	printf("delete [filename] [end_time] [option]\n");
	printf("option -i : 삭제시 'trash'디렉토리로 삭제 파일과 정보를 이동시키지 않고 파일 삭제\n");
	printf("option -r : 지정한 시간에 삭제 시 삭제여부 재 확인\n");
	printf("size [filename] [option]\n");
	printf("option -d number : numer 단계만큼의 하위 디렉토리까지 출력\n");
	printf("recover [filename] [option]\n");
	printf("option -l : 'trash'디렉토리 밑에있는 파일과 삭제 시간들을 삭제 시간이 오래된 순으로 출력\n");
	printf("tree\n");
	printf("exit\n");

	return;
}

void tree(char *pathname,char *dirName,int depth)
{
	struct dirent **namelist;
	struct stat statbuf;
	char checkPath[1024]; //check디렉토리인지 확인
	char movePath[1024];
	int i=0,j=0, count;
	int checkflag = 0;
	int length;
	int empty = 15;
	
	sprintf(checkPath,"%s/check", path);

	if(!strcmp(pathname,checkPath)) {
		checkflag = 1;
		printf("check----------");
		depth++;
	}

	if((count = scandir(pathname, &namelist, *filter, alphasort)) < 0) {
		fprintf(stderr, "scandir error at %s\n", pathname);
		return;
	}

	for(i=0;i<count;i++) {
		sprintf(movePath,"%s/%s", pathname,namelist[i]->d_name);
		stat(movePath, &statbuf);
		if(i == 0 && checkflag == 1)
			printf("%s", namelist[i]->d_name);
		else if(i==0) {
			length = empty - strlen(dirName);
			for(j=0;j<length;j++) 
				printf("-");
			printf("%s",namelist[i]->d_name);
		}
		else {
			for(j=0;j<empty*depth;j++) 
				printf(" ");
			printf("%s",namelist[i]->d_name);
			
		}
		if(S_ISDIR(statbuf.st_mode)) {
		//	printf("%s",namelist[i]->d_name);
			tree(movePath,namelist[i]->d_name,depth + 1);
		}
		else {
			printf("\n");
			if(i != count-1) {
				for(j=0;j<empty*depth; j++)
					printf(" ");
		  		printf("|");
			}
			printf("\n");
		}
	}

	printf("\n");
	return;
}

void delete(char *filename, char *date, char *deleteTime, char *option)
{
	struct tm delete_t;
	struct tm current_t;
	time_t c_time = time(NULL); 
	time_t d_time;
	int d_date[3]={0};
	int dd_time[2]={0};
	int i =0;
	char dir_check[200];
	char dir_trash[200];
	char dir_files[200];
	char dir_info[200];
	char *ptr;

	sprintf(dir_check, "%s/check", path);
	sprintf(dir_trash, "%s/trash", path);
	sprintf(dir_files, "%s/files", dir_trash);
	sprintf(dir_info, "%s/info", dir_trash);

	chdir(dir_check);

	if(date == NULL) {
		filename[strlen(filename) -1 ] = '\0';
	}

	if(filename[0] == '/') {
		strcpy(forDelete, filename);

		if((access(filename, F_OK)) < 0) { //파일이 없을시 에러
			printf("파일이 존재하지 않습니다.\n");
			return;
		}

		ptr = strtok(filename, "/");
		while(ptr != NULL) {
			strcpy(forDeleteFile, ptr); //파일이름을 가져오기 위한
			ptr = strtok(NULL, "/");
		}
	}
	else if(filename[0] == '.') {
		realpath(filename, forDelete);

		if((access(forDelete, F_OK)) < 0) { //파일이 없을시 에러
			printf("파일이 존재하지 않습니다.\n");
			return;
		}

		ptr = strtok(filename, "/");
		while(ptr != NULL) {
			strcpy(forDeleteFile, ptr);
			ptr = strtok(NULL, "/");
		}
	}
	else {
		realpath(filename, forDelete); //forDelete는 전역변수, 파일의 절대경로를 저장해준다

		if((access(filename, F_OK)) < 0) { //파일이 없을시 에러
			printf("파일이 존재하지 않습니다.\n");
			return;
		}

		strcpy(forDeleteFile, filename); // 파일이름을 저장해준다
	}

	signal(SIGALRM,delete_alarm);
	
	if(option != NULL) {
		if(option[1] == 'r')
			printf("r옵션\n");
		else if(option[1] == 'i')
			printf("i옵션\n");
		else {
			printf("옵션을 정확히 입력해 주세요.\n");
			return;
		}
	}

	if((access(dir_trash, F_OK) < 0)) { //trash 디렉토리가 존재하지 않으면
		mkdir(dir_trash,0777);
	}

	if((access(dir_files, F_OK) < 0)) { //trash/files 디렉토리가 존재하지 않으면
		mkdir(dir_files,0777);
	}

	if((access(dir_info, F_OK) < 0)) { //trash/info 디렉토리가 존재하지 않으면
		mkdir(dir_info,0777);
	}

	if(date != NULL && deleteTime != NULL) {
		d_date[0] = atoi(strtok(date, "-"));
		d_date[1] = atoi(strtok(NULL, "-"));
		d_date[2] = atoi(strtok(NULL, " "));

		dd_time[0] = atoi(strtok(deleteTime, ":"));
		dd_time[1] = atoi(strtok(NULL, " "));

		delete_t.tm_year = d_date[0] - 1900;
		delete_t.tm_mon = d_date[1] - 1;
		delete_t.tm_mday = d_date[2];
		delete_t.tm_hour = dd_time[0];
		delete_t.tm_min = dd_time[1];
		delete_t.tm_sec = 0;

		d_time = mktime(&delete_t);
		current_t = *localtime(&c_time);
	}
	else {
		delete_alarm(SIGALRM);
		return;
	}
	
	alarm(d_time - c_time);

	return;
}

void delete_alarm(int signo) {
	char dir_trash[200];
	char dir_files[200];
	char dir_info[200];
	char moveFile[200];
	char beforeMove[200];
	char beforeFile[20];
	time_t d_t = time(NULL);
	struct tm d_times;
	struct tm m_times;
	struct stat statbuf;
	int check = 1;
	FILE *fp;

	sprintf(dir_trash, "%s/trash", path);
	sprintf(dir_files, "%s/files", dir_trash);
	sprintf(dir_info, "%s/info", dir_trash);
	//최종 수정시간 저장 & 삭제시간 저장
	stat(forDelete, &statbuf);
	localtime_r(&statbuf.st_mtime, &m_times);
	d_times = *localtime(&d_t);

	//beforeMove와 beforFile에 수정될 경로와 파일 이름 저장
	//경로가 바뀌거나 파일명이 변할 수 있음
	//moveFile에는 원래 경로 설정, rename때 사용
	strcpy(moveFile, forDelete);
	sprintf(beforeMove, "%s/0_%s", dir_files, forDeleteFile);
	sprintf(beforeFile, "0_%s", forDeleteFile);

	//중복확인
	while(1) {
		if((access(beforeMove, 0)) == 0) { //중복되는 파일이 있다면
			sprintf(beforeMove, "%s/%d_%s", dir_files, check, forDeleteFile);
			sprintf(beforeFile, "%d_%s" ,check, forDeleteFile);
			check++;
		}
		else { //중복이 없다면
			break;
		}
	}

	//옮기기
	if((rename(moveFile, beforeMove))!= 0) {
			printf("이름바꾸기를 실패했어요!\n");
			return;
	}

	//info에 정보저장
	chdir(dir_info);
	checkInfoSize(dir_info, dir_files);
	

	if((fp = fopen(beforeFile, "w+")) < 0) {
		fprintf(stderr, "file open error for %s\n", beforeFile);
		return;
	}

	fflush(stdout);
	fprintf(fp, "[Trash info]\n");
	fflush(fp);
	fprintf(fp, "%s\n", forDelete);
	fflush(fp);
	fprintf(fp, "D : %04d-%02d-%02d %02d:%02d:%02d\n", d_times.tm_year + 1900, d_times.tm_mon+1, d_times.tm_mday, d_times.tm_hour, d_times.tm_min, d_times.tm_sec); 
	fflush(fp);
	fprintf(fp, "M : %04d-%02d-%02d %02d:%02d:%02d\n", m_times.tm_year + 1900, m_times.tm_mon+1, m_times.tm_mday, m_times.tm_hour, m_times.tm_min, m_times.tm_sec); 
	fflush(fp);

	chdir(path);

	return;
}

int checkInfoSize(char *infoPath, char *filesPath)
{
	struct stat statbuf;
	struct dirent  **namelist;
	int count; //info에 있는 파일 개수 저장
	int i;
	long allSize = 0;
	long fileSize;

	while(1) {
		count = scandir(infoPath, &namelist, *filter, alphasort);

		for(i=0;i<count;i++) {
			stat(namelist[i]->d_name, &statbuf);
			fileSize = statbuf.st_size;
			allSize += fileSize;
		}

		if(allSize >= 2048) {
			sortInfo(infoPath, filesPath);
			allSize = 0;
		}
		else 
			return 0;
	}
}

void sortInfo(char *infoPath, char *filesPath)
{
	struct stat statbuf;
	struct dirent **namelist;
	infoFile fileinfo[100];
	infoFile temp;
	int count;
	int i,j;
	char deleteFile[200];
	char deleteInfo[200];
	
	count = scandir(infoPath, &namelist, *filter, alphasort);
	
	for(i=0;i<count;i++) {
		strcpy(fileinfo[i].name,namelist[i]->d_name);
		stat(namelist[i]->d_name, &statbuf);
		fileinfo[i].tmm = statbuf.st_mtime;
	}

	for(i=0;i < count - 1; i++) {
		for( j= i ; j<count;j++) {
			if(i== j)
				continue;
			else {
				if(fileinfo[i].tmm > fileinfo[j].tmm) {
					temp = fileinfo[i];
					fileinfo[i] = fileinfo[j];
					fileinfo[j] = temp;
				}
			}
		}
	}

	sprintf(deleteFile, "%s/%s", filesPath, fileinfo[0].name);
	sprintf(deleteInfo, "%s/%s", infoPath, fileinfo[0].name);
	unlink(deleteFile);
	unlink(deleteInfo);

	return;
}

void size()
{
}

void recover(char *filename, char *option)
{
	char dir_check[200];
	char dir_trash[200];
	char dir_files[200];
	char dir_info[200];
	int check = 0 ,check2 = 1; //중복되는 파일이 있는지 확인, 복귀될 폴더에 중복되는것이 있는지 확인
	int i,count;
	FILE *fp;
	struct dirent **namelist;
	char *checkName;
	char get[100];
	char moveFile[200], trashFile[200], deleteFile[200];

	sprintf(dir_check, "%s/check", path);
	sprintf(dir_trash, "%s/trash", path);
	sprintf(dir_files, "%s/files", dir_trash);
	sprintf(dir_info, "%s/info", dir_trash);

	if(option != NULL) {
		if(option[1] == 'l') 
			printf("l옵션 \n");
		else
			printf("정확한 옵션을 주어야 합니다. (소문자 l)\n");
	}
	else {
		filename[strlen(filename) - 1] = '\0';
	}
	
	count = scandir(dir_info, &namelist, *filter, alphasort);
	//중복확인
	for(i=0;i<count;i++) {
		checkName = strtok(namelist[i]->d_name, "_");
		checkName = strtok(NULL, " ");
		if((checkName != NULL) && (!strcmp(filename, checkName)))
			check++;
	}

	if(check == 0) {
		printf("파일이 존재하지 않습니다.\n");
		return;
	}

	if(check > 1) {
		if((isSelect(filename, get, check)) == 0) {
		//	printf("delete수행시 순서대로 되었던 [n_filename]이 중간숫자가 비어있으면 되지 않습니다...\n");
		}
	}
	else {
		sprintf(get, "0_%s", filename);
	}

	sprintf(moveFile, "%s/%s", dir_check, filename);
	sprintf(deleteFile, "%s/%s", dir_info, get);
	sprintf(trashFile, "%s/%s", dir_files, get);

	
	while(1) {
		if((access(moveFile, 0)) == 0) { //중복되는 파일이 있다면
			sprintf(moveFile, "%s/%d_%s", dir_check, check2, filename);
			check2++;
		}
		else { //중복이 없다면
//			sprintf(beforeFile, "0_%s", forDeleteFile);
			break;
		}
	}
	
	if((rename(trashFile, moveFile) < 0)) {
		printf("recover error! , 0_로 시작하는 파일이 있어야 합니다 하하\n");
	}
	unlink(deleteFile);

	return;
}

int isSelect(char *filename, char *get, int check)
{
	int i, j, choose;
	FILE *fp;
	char buf[1000];
	char dir_check[200];
	char dir_trash[200];
	char dir_files[200];
	char dir_info[200];

	sprintf(dir_check, "%s/check", path);
	sprintf(dir_trash, "%s/trash", path);
	sprintf(dir_files, "%s/files", dir_trash);
	sprintf(dir_info, "%s/info", dir_trash);
	
	chdir(dir_info);
	for(i=0;i<check;i++) {
		sprintf(get, "%s/%d_%s", dir_info, i, filename);
		if((fp = fopen(get, "r")) == NULL) {
			printf("delete수행시 순서대로 되었던 [n_filename]이 중간숫자가 비어있으면 되지 않습니다...\n");
			return 0;
		}
		printf("%d. %s   ", i+1, filename);
		for(j=0;j<4;j++) {
			fgets(buf, 1000, fp);
			if(j > 1) {
				buf[strlen(buf) - 1] = '\0';
				printf("%s ", buf);
			}
		}
		printf("\n");
		fflush(fp);
	}
	
	printf("choose : ");
	scanf("%d", &choose);
	while(getchar() != '\0')
		break;
	sprintf(get, "%d_%s", choose-1, filename);
	
	chdir(path);
	return 1;
}
