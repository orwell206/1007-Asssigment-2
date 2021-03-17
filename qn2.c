#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>


void perm_to_letters(mode_t filemode, char file_perms[]){
	/*
		Desc: Converts the file's permissions to letters.

		Params: mode_t filemode - File type and mode
				char str[] - The file permissions 
	*/			
	strcpy(file_perms, "----------" );
	if(S_ISREG(filemode)) file_perms[0] = '-';
	if(S_ISDIR(filemode)) file_perms[0] = 'd';
	if(S_ISLNK(filemode)) file_perms[0] = 'l';

	// User permission bits
	if(filemode & S_IRUSR) file_perms[1] = 'r';
	if(filemode & S_IWUSR) file_perms[2] = 'w';
	if(filemode & S_IXUSR) file_perms[3] = 'x';

	// Group permission bits
	if(filemode & S_IRGRP) file_perms[4] = 'r';
	if(filemode & S_IWGRP) file_perms[5] = 'w';
	if(filemode & S_IXGRP) file_perms[6] = 'x';

	// Other group permission bits
	if(filemode & S_IROTH) file_perms[7] = 'r';
	if(filemode & S_IWOTH) file_perms[8] = 'w';
	if(filemode & S_IXOTH) file_perms[9] = 'x';
}


void get_metadata(char *file_name, struct stat *file_stats){
	/*
		Desc: Gets the metadata of the file. 

		Params: char *file_name - Name of the opened file. 
				struct stat *file_stats - stat structure
	*/		
	mode_t st_mode = file_stats->st_mode;									// File type and mode
	char file_perms[10];													// Variable to store the file permissions
	char time[50];															// Variable to store timestamps 
	perm_to_letters(st_mode, file_perms);
	printf("%s\t", getpwuid(file_stats->st_uid)->pw_name);  				// User name
	printf("%s\t", getgrgid(file_stats->st_gid)->gr_name); 					// Group name
	printf("%s\t", file_perms);  											// File permissions
	printf("%ld\t", file_stats->st_size); 									// File size
	printf("%ld\t", file_stats->st_ino);  									// Inode number 
	printf("%ld\t", file_stats->st_dev);  									// Device number of the device in which the file is stored
	printf("%ld\t", file_stats->st_nlink); 									// Symbolic link
	strftime(time, 50, "%b %d %H:%M", localtime(&file_stats->st_atime));	// Last access-time
	printf ("%s\t", time);
	strftime(time, 50, "%b %d %H:%M", localtime(&file_stats->st_atime));  	// Last modified-time
	printf ("%s\t", time);
	strftime(time, 50, "%b %d %H:%M", localtime(&file_stats->st_ctime));  	// File status changes time
	printf ("%s\t", time);
	printf( "%s\n", file_name);												// File name
}


void open_file(char *file_name){
	/*
		Desc: Opens the file and get its metadata. 

		Params: char *file_name - Name of the file to be opened.
	*/	
	struct stat file_stats;

	// Check that the file can be opened.
	if (stat(file_name, &file_stats) == -1 ){
		printf("Error opening %s!\n", file_name );
		return;
	}
	else{
		get_metadata(file_name, &file_stats);
	}
}


int list_dir(char current_dir[]) {
	/*
		Desc: Lists all the files in the current directory.

		Params: char current_dir[] - The current directory path. 
	*/
	DIR* pDirStream;  				// Pointer to the directory stream. The stream is positioned at the first etry in the directory. 
	struct dirent* pCurrent_dir;  	// Pointer to the dirent structure that represents the next directory entry in the directory stream. 
	
	// Check that the current directory can open. 
	if((pDirStream = opendir(current_dir)) == NULL){
		perror("read dir");
		exit(-1);
	 }

	// If the current directory successfully opens, open each file in the directory and list their metadata. 
	else {
		while ((pCurrent_dir = readdir(pDirStream)) != NULL){
			// Check that the file is not "." and "..". 
			if (strcmp(pCurrent_dir->d_name, ".") == 0 || strcmp(pCurrent_dir->d_name,"..") == 0){
				continue;
			}
			else {
				open_file(pCurrent_dir->d_name);
			}
		}
		closedir(pDirStream);
	}
}


int main(int argc, char *argv[])
{
	/*
		Desc: Main function.

		Params: int argc - Number of arguments provided to the main function.
				char *argv[] - The list of arguments provided to the main function. 
	*/
	// List all meta data of the files in the current directory if no arguments are provided. 
	if(argc == 1) {
		list_dir( "." );
	}

	// List the meta data of the specified files in the provided arguments. 
	else {
		for (int i = 1; i < argc; i++){
			open_file(argv[i]);
		}
	}
	return 0;
 }