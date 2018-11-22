#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <cstddef>
#include <cstring>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <sys/sendfile.h>  // sendfile
#include <fcntl.h>         // open
#include <unistd.h>        // close
#include <sys/stat.h>      // fstat
#include <sys/types.h> 

using namespace std;

struct Files{
	string path;
	bool dir; //0 for file, 1 for dir
};

struct Task{
	int num = 1;
	vector<struct Files>fileRoll;
};

struct Task task;

void analiseArgs(int argc, char* argv[]){

}

void analiseDir(string name) {
	DIR *d = opendir(name.c_str());
	if (d == NULL) {
    	perror(name.c_str());
    	return;
   	}
	for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
		string fqn = name + "/" + de->d_name;
      	if (de->d_type == DT_DIR) {
        	if (strcmp(de->d_name,".") == 0) continue;
        	if (strcmp(de->d_name,"..") == 0) continue;
        	dir(fqn);
      	}
      	printf("%s %s\n", fqn.c_str(), de->d_type==DT_DIR?"[dir]" : "");
   	}
   	closedir(d);
}

void copyDir(string sender, string reciever){

}

void copyFile(string sender, string reciever){
	int read_fd;
	int write_fd;
 	struct stat stat_buf;
 	off_t offset = 0;
 	read_fd = open (sender.c_str(), O_RDONLY);
 	if(read_fd == 0){
 		perror(sender.c_str());
 	}
  	fstat (read_fd, &stat_buf);
 	write_fd = open (reciever.c_str(), O_WRONLY | O_CREAT, stat_buf.st_mode);
 	if(write_fd == 0){
 		perror(reciever.c_str());
 	}
 	sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
 	close (read_fd);
 	close (write_fd);
}


int main (int argc, char* argv[])
{
	string sender = argv[1];
	cout << sender << "------------------my test" << endl;
//	string reciever = argv[2];
	//cout << reciever << endl;
	dir(sender);
	//copyFile(sender, reciever);
}
