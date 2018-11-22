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

struct XFiles {
	string path_from;
	string path_to;
	bool dir; //0 for file, 1 for dir
};
typedef struct XFiles XFiles;

struct Task {
	int num = 1;
	vector<XFiles>file_roll;
};

typedef struct Task Task;

Task task;

void analise_dir(string sender, string reciever) {
	DIR *d = opendir(sender.c_str());
	if (d == NULL) {
    	perror(sender.c_str());
    	return;
   	}
	for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
		string fqn = sender + "/" + de->d_name;
		string dest = reciever + "/" + de->d_name;
      	if (de->d_type == DT_DIR) {
        	if (strcmp(de->d_name,".") == 0) continue;
        	if (strcmp(de->d_name,"..") == 0) continue;
        	analise_dir(fqn, dest);
      	}
	  	XFiles temp_XFile;
		temp_XFile.path_from = fqn;
	   	temp_XFile.path_to = dest;
      	temp_XFile.dir = (de->d_type==DT_DIR);
      	task.file_roll.push_back(temp_XFile);
   	}
   	closedir(d);
}

void build_task(char* argv[]) {
	string sender = argv[2], reciever = argv[3];
	sscanf(argv[1],"-t%d", &task.num);
	analise_dir(sender, reciever);
}


void copy_file(string sender, string reciever) {
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

int main(int argc, char* argv[]) {
	build_task(argv);
	printf("%d\n", task.num);

	for(int i = 0; i<task.file_roll.size(); i++) 
		printf("%s -> %s %s \n",
			task.file_roll[i].path_from.c_str(),
			task.file_roll[i].path_to.c_str(),
			task.file_roll[i].dir ? "[dir]" : "[file]"); 
}