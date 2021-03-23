#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h> 

int REDIRECT_FLAG = 0;
char *SHELL_NAME = "tux";  	// Name of the shell. 
int QUIT = 0;  				// Determines whether the shell terminates or not. 
size_t BUFFER_SIZE = 1024;  // Maximum number of characters that the user can enter into the shell. 
int NO_OF_COMMANDS = 3;  	// No. of commands in the global COMMANDS array.
char *COMMANDS[] = {	 	// Contains the commands supported by the shell. 
  "cp",
  "pwd\n",
  ">",
};


int cp_command(char **args){
	/*
		Desc: Uses the system syscall to execute the cp command. 

		Params: char **args - An array containing the command the user entered. 
							  The array was produced by the parseLine() command.

	*/	
	char character;
	char *source_name = args[1];
	char *destination_name = args[2];
	char *command; 
	FILE *source_file;
	FILE *destination_file;

	// Attempt to open the source file 
	source_file = fopen(source_name, "r");
	if(source_file == NULL)
	{
		printf("Error opening source file!\n");
		return -1;
	}

	// Attempt to open the destination file
	destination_file = fopen(destination_name, "w");
	if(destination_file == NULL)
	{
		printf("Error opening destination file!\n");
		return -1;
	}

	// Copy the contents of the source file to the destination character by character until EOF
	while ((character = fgetc(source_file)) != EOF ){
		fputc(character, destination_file);
	}

	// Close both the source and destination files 
   	fclose(source_file);
   	fclose(destination_file);
	printf("File successfully copied!\n");
}


int pwd_command(char **args){
	/*
		Desc: Uses the system syscall to execute the pwd command. 

		Params: char **args - An array containing the command the user entered. 
							  The array was produced by the parseLine() command.
	*/	
	char cwd[1024];

	// Get the current working directory
	getcwd(cwd, sizeof(cwd));

	// Display the current working directory
	printf("%s\n", cwd);
}


int io_redirect_command(char **args){
	/*
		Desc: Uses the system syscall to execute the io_redirect command. 

		Params: char **args - An array containing the command the user entered. 
							  The array was produced by the parseLine() command.
	*/		
	char character;
	char *source = args[0];
	char *destination = args[2];
	char dataToBeCopied[1024];
	int fd;
	int pid; 
	FILE *destination_file;

	// Attempt to open the destination file.
	destination_file = fopen(destination, "w");
	if(destination_file == NULL)
	{
		printf("Error opening destination file!\n");
		return -1;
	}

	// Gets the file descriptor of the desintation file.
	fd = fileno(destination_file);

	// Copies the data to be copied to another variable.
	strcpy(dataToBeCopied, source);

	// Check if the user entered the "pwd" command.
	if (strcmp(source, "pwd") == 0){
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		strcpy(dataToBeCopied, cwd);
	}

	// Fork a child process to redirect the output to the destination file.
	pid = fork();
	if (pid == 0) {
		dup2(fd, 1);
		puts(dataToBeCopied);
		exit(1);
		fclose(destination_file);
	}
	printf("I/O redirection copied!\n"); 
}


int exit_command(){
	/*
		Desc: Exits the shell by setting the global QUIT variable to 1. 

		Params: None.
	*/		
	printf("Exiting shell!\n");
	QUIT = 1;
}


int (*COMMAND_FUNC[]) (char **) = {&cp_command, &pwd_command, &io_redirect_command, &exit_command};  // Array of the command functions. 


char *readLine(){
	/*
		Desc: Reads the command inputted by the user. 

		Params: None. 
	*/	
	char *buffer;
	size_t line;
	buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

	// Check that memory can be allocated to the buffer. 
	if (buffer == NULL)
	{
		perror("Unable to allocate buffer");
		exit(1);
	}

	// Gets the command inputted by the user into the buffer. 
	getline(&buffer, &BUFFER_SIZE, stdin);
	return buffer;
}


char **parseLine(char *line)
{
	/*
		Desc: Parses the command inputted by the user and splits it into tokens. The delimeter is whitespace. 

		Params: char *line - The command that the user entered. 
	*/	
	char delim[] = " ";
	int position = 0;
	char *token = strtok(line, delim);
	char **tokens = malloc(BUFFER_SIZE * sizeof(char*));

	// Splits the command by using the whitespace as delimeter. 
	while(token != NULL)
	{
		tokens[position] = token;
		position++;
		token = strtok(NULL, delim);
	}

	// Insert NULL as the last element into the tokens array. 
	tokens[position] = NULL;
	return tokens;
}


int executeLine(char **args){
	/*
		Desc: Executes the command inputted by the user by passing

		Params: char **args - An array containing the commands inputted by the user. The command was splitted into tokens in the parseLine function.
	*/		
	// Count the number of arguments that are in the command. 
	int argCount = 0;
	while (args[argCount] != NULL){
		argCount++;
	}

	// Check whether the command entered by the user is "exit".
	if (strcmp(args[0], "exit\n") == 0){
		exit_command();
	}
	
	// Checks whether the command entered by the user exists in the global COMMANDS array. 
	for (int i = 0; i < NO_OF_COMMANDS; i++){
		// For single commands such as "pwd".
		if (argCount == 1){
			if(strcmp(args[0], COMMANDS[i]) == 0){
				return (*COMMAND_FUNC[i])(args);
			}
		}

		// For commands that require multiple parameters such as "cp" and ">".
		else {
			if(strcmp(args[0], COMMANDS[i]) == 0 || strcmp(args[1], COMMANDS[i]) == 0){
				return (*COMMAND_FUNC[i])(args);
			}
		}
	}		
}


int main(){
	/*
		Desc: Main function.

		Params: The shell does three things mainly. 
		- It reads the command inputted by the user.
		- It parses the command inputted by the user.
		- It executes the command inputted by the user. 
	*/	
	char *line;
	char **args;

	// Continously loop the shell if the user did not specify to quit. 
	while(QUIT == 0)
	{
		printf("%s:$ ", SHELL_NAME);
		// Reads the command inputted by the user.
		line = readLine();

		// Parses the command inputted by the user.
		args = parseLine(line);

		// Executes the command inputted by the user. 
		executeLine(args);
	}
	return 1;
}