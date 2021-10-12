#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

int daemon_init(void);
void monitoring(void);
int checkTime(char *namelist, char *pathname, int point);
void create(char *namelist, char *pathname, int point);
void delete(char *namelist,char *pathname, int point);
void modify(char *namelist, char *pathname, int point);

char path[200];
int count;
FILE *lfp;

typedef struct saveName {
	char name[20];
	time_t saveTime;
}saveName;

saveName sN[1024];

static int filter(const struct dirent *dirent) //scandir filter함수
{
	if(!(strncmp(dirent->d_name, ".",1)))
		return 0;
	else
		return 1;
}

int main()
{
	struct dirent **namelist;
	int i;
	struct stat statbuf;
	char pathname[200];
	
	getcwd(path, 200);

	sprintf(pathname, "%s/check",path);

	if((access(pathname, 0)) < 0) { //check디렉토리가 존재하지 않으면 
		mkdir(pathname,0777);
	}
	
	if((count = scandir(pathname, &namelist, *filter, alphasort)) == -1) {
		fprintf(stderr, "scan error\n");
		exit(1);
	}

	chdir(pathname);
	for(i=0;i<count;i++) {
			strcpy(sN[i].name, namelist[i]->d_name);
			stat(namelist[i]->d_name, &statbuf);
			sN[i].saveTime = statbuf.st_mtime;
		}

	chdir(path);

	daemon_init();
	
	return 0;
}
void monitoring(void) 
{
	struct dirent **namelist;
	int count2;
	int i=0, j=0;
	char pathname[200];

	chdir(path);

	sprintf(pathname, "%s/check",path);

	count2 = scandir(pathname, &namelist, filter, alphasort);

	if(count <= count2) { //생성 또는 수정
		for(i=0;i<count2;i++) {
			if(!(strcmp(sN[i].name, namelist[i]->d_name))) { //이름이 같으면
				if(checkTime(namelist[i]->d_name,pathname, i) == 0) //시간이 다르면
					return; //수정이므로 종료
				else
					continue;
			}
			else{  //이름이 다르면
				create(namelist[i]->d_name, pathname, i);
				return;
			}
		}
	}
	else { // count > count2 -> 삭제
		for(i=0;i<count;i++) {
			if((strcmp(sN[i].name, namelist[i]->d_name))!= 0) { //이름이 다르면
				delete(sN[i].name, pathname, i);
				return;
			}
		}
	}

	return;
}

int checkTime(char *namelist, char *pathname, int point) //시간을 체크해준다 같으면 1을 리턴 다르면 수정함수 호출
{
	struct tm times;
	struct stat statbuf;

	chdir(pathname);
	stat(namelist, &statbuf);
	
	if(sN[point].saveTime == statbuf.st_mtime) {
		chdir(path);
		return 1;
	}
	else {
		modify(namelist,pathname,point);
		return 0;
	}
}

void modify(char *namelist, char *pathname, int point)
{
	struct tm times;
	struct stat statbuf;

	chdir(pathname);
	stat(namelist, &statbuf);
	localtime_r(&statbuf.st_mtime, &times);
	fprintf(lfp, "[%04d-%02d-%02d %02d:%02d:%02d][modify_%s]\n",times.tm_year + 1900, times.tm_mon+1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec, namelist);
	fflush(lfp);

	sN[point].saveTime = statbuf.st_mtime; //전역변수 갱신

	chdir(path);

	return;
}

void create(char *namelist, char *pathname, int point)
{
	struct tm times;
	struct stat statbuf;
	int i;

	chdir(pathname);
	stat(namelist, &statbuf);
	localtime_r(&statbuf.st_mtime, &times);
	fprintf(lfp, "[%04d-%02d-%02d %02d:%02d:%02d][creat_%s]\n",times.tm_year + 1900, times.tm_mon+1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec, namelist);
	fflush(lfp);

	for(i = count-1; i>=point; i--) {
		sN[i+1] = sN[i];
	}

	strcpy(sN[point].name,namelist); //구조체 초기화
	sN[point].saveTime = statbuf.st_mtime;
	count++;

	chdir(path);

	return;
}

void delete(char *namelist, char *pathname, int point)
{
	struct tm times;
	struct stat statbuf;
	time_t t = time(NULL);
	int i;

	chdir(pathname);
	times = *localtime(&t);
	fprintf(lfp, "[%04d-%02d-%02d %02d:%02d:%02d][delete_%s]\n",times.tm_year + 1900, times.tm_mon+1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec, namelist);
	fflush(lfp);

	for(i=point; i<count-1; i++) {
		sN[i] = sN[i+1];
	}

	memset(sN[count-1].name, 0, 20); //구조체 초기
	sN[count-1].saveTime = 0;
	count--;

	chdir(path);

	return;
}

int daemon_init(void)
{
	pid_t pid;
	int fd, maxfd;

	if((pid = fork()) <0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}
	else if(pid != 0) 
		exit(0);

	pid = getpid();
	setsid();

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	maxfd = getdtablesize();

	for(fd = 0; fd < maxfd; fd++)
		close(fd); 

	umask(0);
	chdir("/");
	
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0); 
	
 	chdir(path);
	lfp = fopen("log.txt","w+");

	while(1){
		monitoring();
	}

	return 0;
}
