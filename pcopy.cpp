#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <string>
#include <dirent.h>
#include <signal.h>
#include <cstddef>
#include <cstring>
#include <utime.h>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <sys/sendfile.h>  // sendfile
#include <fcntl.h>         // open
#include <unistd.h>        // close
#include <sys/stat.h>      // fstat
#include <sys/types.h> 

using namespace std;

struct XFiles {
	string path_from;
	string path_to;
};
typedef struct XFiles XFiles;

struct Task {
	int num = 1;
	vector<XFiles>file_roll;
	vector<XFiles>dir_roll;
};

typedef struct Task Task;

Task task;

void analise_dir(string sender, string reciever) {
	DIR *d = opendir(sender.c_str());
	if (d == NULL) {
    	string err = sender.c_str();
 		err.insert(0, "fail opendir ");
    	return;
   	}
   	XFiles temp_XFile;
   	temp_XFile.path_from = sender;
	temp_XFile.path_to = reciever;
    task.dir_roll.push_back(temp_XFile);

	for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
		string fqn = sender + "/" + de->d_name;
		string dest = reciever + "/" + de->d_name;
      	if (de->d_type == DT_DIR) {
        	if (strcmp(de->d_name,".") == 0) continue;
        	if (strcmp(de->d_name,"..") == 0) continue;
        	analise_dir(fqn, dest);
      	}else{
      		if(de->d_type == DT_DIR) continue;
			temp_XFile.path_from = fqn;
	   		temp_XFile.path_to = dest;
      		task.file_roll.push_back(temp_XFile);
      	}
   	}
   	closedir(d);
}

void build_task(char* argv[]) {
	string sender = argv[2], reciever = argv[3];
	sscanf(argv[1],"-t%d", &task.num);
	analise_dir(sender, reciever);
}

void copy_dir(string sender, string reciever) {
 	struct stat stat_buf;
 	off_t offset = 0;
	if(-1 == stat(sender.c_str(), &stat_buf)){
 		string err = sender.c_str();
 		err.insert(0, "fail stat ");
 		perror(err.c_str());
 	}
 	if(-1 == mkdir(reciever.c_str(), stat_buf.st_mode)){
 		string err = reciever.c_str();
 		err.insert(0, "fail create dir ");
 		perror(err.c_str());
 	}
 	struct utimbuf buf_time;
 	buf_time.actime = stat_buf.st_atime;
 	buf_time.modtime =stat_buf.st_mtime;

 	if(-1 == utime(reciever.c_str(), &buf_time)){
 		string err = reciever.c_str();
 		err.insert(0, "fail utime ");
 		perror(err.c_str());
 	}
}

void modify_dir(string sender, string reciever) {
 	struct stat stat_buf;
 	off_t offset = 0;
	if(-1 == stat(sender.c_str(), &stat_buf)){
 		string err = sender.c_str();
 		err.insert(0, "fail stat ");
 		perror(err.c_str());
 	}
 	struct utimbuf buf_time;
 	buf_time.actime = stat_buf.st_atime;
 	buf_time.modtime = stat_buf.st_mtime;

 	if(-1 == utime(reciever.c_str(), &buf_time)){
 		string err = reciever.c_str();
 		err.insert(0, "fail utime ");
 		perror(err.c_str());
 	}
}

void create_dirs(){
	struct stat stat_buf;
	for(int i = 0; i < task.dir_roll.size(); i++){
		if(-1 == stat(task.dir_roll[i].path_to.c_str(), &stat_buf)){
			printf("in create_dirs create dir %s\n", task.dir_roll[i].path_to.c_str());
			copy_dir(task.dir_roll[i].path_from, task.dir_roll[i].path_to);
		}else{
			printf("in create_dirs modify dir %s\n", task.dir_roll[i].path_to.c_str());
			modify_dir(task.dir_roll[i].path_from, task.dir_roll[i].path_to);
		}
	}
}

//---------------------------------------------------------------------------------------
void modify_file(string sender, string reciever) {
 	struct stat stat_buf;
 	off_t offset = 0;
	if(-1 == stat(sender.c_str(), &stat_buf)){
 		string err = sender.c_str();
 		err.insert(0, "in modify_file stat ");
 		perror(err.c_str());
 	}
 	struct utimbuf buf_time;
 	buf_time.actime = stat_buf.st_atime;
 	buf_time.modtime =stat_buf.st_mtime;

 	if(-1 == utime(reciever.c_str(), &buf_time)){
 		string err = reciever.c_str();
 		err.insert(0, "in modify_file utime ");
 		perror(err.c_str());
 	}
}
//------------------------------------------------------------------------------------
void copy_file(string sender, string reciever) {
	int read_fd;
	int write_fd;
 	struct stat stat_buf;
 	off_t offset = 0;

 	read_fd = open (sender.c_str(), O_RDONLY);
 	if(-1 == read_fd){
 		string err = sender.c_str();
 		err.insert(0, "fail open ");
 		perror(err.c_str());
 		return;
 	}

  	if(-1 == fstat(read_fd, &stat_buf)){
  	 	string err = sender.c_str();
 		err.insert(0, "fail stat ");
  	 	perror(err.c_str());
  	 	return;
  	}

 	write_fd = open (reciever.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
 	if(-1 == write_fd){
 		string err = reciever.c_str();
 		err.insert(0, "fail create ");
 		perror(err.c_str());
 		return;
 	}
 	
 	if(-1 == sendfile (write_fd, read_fd, &offset, stat_buf.st_size)){
 	 	string err = sender.c_str();
 	 	err.insert(0 ,"fail sendfile ");
 	 	perror(err.c_str());
 	 	return;
 	}
 	close (read_fd);
 	close (write_fd);
}
//-------------------------------------------------------------------------------

pthread_mutex_t mutex;

void* thread_func(void *a)
{
	for(;;)
	{
		pthread_mutex_lock(&mutex);
		if(task.file_roll.size() == 0){
			pthread_mutex_unlock(&mutex);
			return NULL;
		}
		string sender = task.file_roll.back().path_from;
		string reciever = task.file_roll.back().path_to;
		task.file_roll.pop_back();
		cout << "in thread_func copy file " << sender << endl;
		pthread_mutex_unlock(&mutex);
		copy_file(sender, reciever);
	}
}

void create_files(int num){
	struct stat stat_buf;
	for(int i = 0; i < task.file_roll.size(); i++){	//REMOVE
		if(-1 != stat((task.file_roll[i].path_to + ".old").c_str(), &stat_buf)){
			printf("in create_files remove file %s\n", (task.file_roll[i].path_to+".old").c_str());
			remove((task.file_roll[i].path_to + ".old").c_str());
		}
	}
	for(int i = 0; i < task.file_roll.size(); i++){
		if(-1 != stat((task.file_roll[i].path_to.c_str()), &stat_buf)){ //RENAME
			printf("in create_files rename file %s\n", task.file_roll[i].path_to.c_str());
			rename(task.file_roll[i].path_to.c_str(),(task.file_roll[i].path_to + ".old").c_str());
		}
	}
	//-------------------------------------------------------------------------
	// creating threads
	pthread_mutex_init(&mutex, NULL);
	
	pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)*num);
	for(int i = 0; i < num; i++){
		pthread_create(threads + i, NULL, thread_func, NULL);
	}
	//--------------------------------------------------------------------------
	//copy
	for(int i = 0; i < num; i++){
		pthread_join(threads[i],NULL);
	}
}

int main(int argc, char* argv[]) {
	if(argv[3] == NULL){
		printf("FAIL: Not enough arguments\n");
		return -1;
	}
//*
	build_task(argv);
	printf("%d threads will be created\n", task.num);
	for(int i = 0; i<task.dir_roll.size(); i++) 
		printf("%s -> %s [DIR] \n",
			task.dir_roll[i].path_from.c_str(),
			task.dir_roll[i].path_to.c_str());
	for(int i = 0; i<task.file_roll.size(); i++) 
		printf("%s -> %s [FILE] \n",
			task.file_roll[i].path_from.c_str(),
			task.file_roll[i].path_to.c_str());
	printf("--------------------------------------------------\n");
	create_dirs();
	create_files(task.num);
	
}

