// Yasmeen Al-Harbi & Jordyn Espenant
// NSIDs: yaa300 & jde842
// CMPT 332 Section 1
// Assignment 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
Structure to hold a simple command 
with its output redirection file.
*/
typedef struct single_command {
	char **command;
	char *outfile;
} Single_Command;


/*
Prints the shell prompt and returns the users command.
Commands must be written from right to left.
*/
char* prompt() {
	size_t command_size = 0;
	char* command = (char*) malloc(sizeof(char) * command_size);

	printf("wrdsh> ");

	// Stores the entire line of input into command.
	size_t num_chars = getline(&command, &command_size, stdin);

	// Use length of input to replace the last character of the input to null.
	command[num_chars-1] = '\0';
	
	return command;
}


/*
Parses the line by a space delimeter.
Returns a pointer to a pointer containing the split strings.
@param input: a pointer to a string input taken from the user.
*/
char** parse_input(char* input) 
{
	int index = 0;

	// Split the first string.
	char* token = strtok(input, " ");
	char** token_list = (char**) malloc(sizeof(char*) * BUFSIZ);

	// Keep splitting strings if delimeter is encountered.
	while (token != NULL) 
	{
		token_list[index] = token;
		index++;

		// Keep tokenizing the same string by passing NULL.
		token = strtok(NULL, " ");
	}
	
	return token_list;
}


/*
Takes a parsed input array and puts the individual commands
inside of Single_Command struct. Returns a pointer to that struct.
@param parsed: a pointer to a string array.
*/
Single_Command* create_commands(char** parsed) 
{
	// Allocates space for an array of individual commands.
	Single_Command *command_list = (Single_Command*) malloc(sizeof(Single_Command*) * BUFSIZ);
	
	int p_index = 0; // Parsed index.
	int c_index = 0; // Command index.
	int a_index = 0; // Argument index.
	
	while (parsed[p_index]) 
	{
		if (p_index == 0 && parsed[1] && strcmp(parsed[1], "<") == 0) // Check if there will be redirection.
		{
			// Do nothing since outfile will be saved on the next iteration of while loop.
		}
		else if (strcmp(parsed[p_index], "<") != 0 && strcmp(parsed[p_index], "|") != 0) // Check for any pipes and redirection.
		{
			// Allocate each command struct.
			if (command_list[c_index].command == NULL) 
			{
				command_list[c_index].command = (char**) malloc(sizeof(char*) * BUFSIZ);
			}

			// Allocate each string for the arguments in the command.
			command_list[c_index].command[a_index] = (char*) malloc(sizeof(char) * (strlen(parsed[p_index]) + 1));
			strcpy(command_list[c_index].command[a_index], parsed[p_index]);
			a_index++; // Move to make space for the next argument in the command.
		}
		else 
		{
			// Saves the outfile if redirection is found.
			if (strcmp(parsed[p_index], "<") == 0) 
			{
				command_list[c_index].outfile = (char*) malloc(sizeof(char) * (strlen(parsed[p_index-1]) + 1));
				strcpy(command_list[c_index].outfile, parsed[p_index-1]);
			}
			// If a pipe is found, move to next command in the structure.
			else if (strcmp(parsed[p_index], "|") == 0)
			{
				c_index++; // Move to next command_list index for new command.
				a_index = 0; // Reset argument counter for next command.
			}
		}

		p_index++;
	}

	// Allocates space for an array of individual commands in reverse order.
	Single_Command *r_command_list = (Single_Command*) malloc(sizeof(Single_Command*) * BUFSIZ);
	int r_index = 0; // Command index in the reverse list.
	int i = c_index;

	// Copy the contents of command_list into r_command_list in reverse order.
	while(c_index != -1)
	{
		r_command_list[r_index].command = command_list[c_index].command; // Set the current item in command_list to the current item in r_command_list.
		
		if (command_list[c_index].outfile) // If there exists an outfile for the current command, copy it in the reverse list's current command.
		{
			r_command_list[r_index].outfile = (char*) malloc(sizeof(char) * (strlen(command_list[c_index].outfile) + 1)); // Allocate the outfile.
			strcpy(r_command_list[r_index].outfile, command_list[c_index].outfile); // Copy the outfile.
		}

		c_index = c_index - 1;
		r_index++;
	}

	// Free the allocated space for the outfile in command_list.
	while(i != -1)
	{
		if (command_list[i].outfile)
		{
			free(command_list[i].outfile);
		}
		i = i - 1;
	}

	return r_command_list;
}


/*
Executes a command using execvp().
@param command: array of strings forming a command.
@param flag: 1 if there is output to the user, 0 otherwise.
*/
int execute_command(char** command, int flag)
{
	// Create a pipe for duplicating cmpt letters.
	int fd[2];
	char buffer[1];

	if (flag) // If there is output to the user, create pipe.
	{
		if (pipe(fd) < 0) // Check if pipe failed.
		{
			fprintf(stderr, "Pipe failed.\n");
			exit(1);
		}
	}

	// Fork to create a new process.
	int rc = fork();
	if (rc < 0) // Fork failed, exit immediately.
	{
		fprintf(stderr, "Fork failed.\n");
		exit(1);
	}
	else if (rc == 0) // Child (new) process.
	{
		if (flag) // Redirect output if there is output to the user.
		{
			// Close the read end of the pipe, we only want to write here.
			close(fd[0]);

			// Redirect the output of the command to the write end of the pipe.
			dup2(fd[1], STDOUT_FILENO);
		}

		// Prints error message if command does not run.
		if (execvp(command[0], command) == -1) 
		{
			perror("Error in command");

			// Exit out of the child process.
			exit(1);
		}

		if (flag)
		{
			close(fd[1]); // Done writing, close write end of pipe.
		}
	}
	else // Parent process.
	{ 
		if (flag) // If there is output to the user, duplicate the letters 'c', 'm', 'p', 't' whenever they are encountered.
		{
			close(fd[1]); // Close the write end of the pipe, we only want to read here.

			while (read(fd[0], buffer, sizeof(buffer)) != 0)
			{
				if (buffer[0] == 'c')
				{
					printf("c%s", buffer);
				}
				else if (buffer[0] == 'm')
				{
					printf("m%s", buffer);
				}
				else if (buffer[0] == 'p')
				{
					printf("p%s", buffer);
				}
				else if (buffer[0] == 't')
				{
					printf("t%s", buffer);
				}
				else {
					printf("%s", buffer);
				}
			}

			close(fd[0]); // Close the read end of the pipe, reading finished here.
		}

		// Make sure the commands finish by making the parent process wait for the child process.
		wait(NULL);
	}

	return 0;
}


/*
Initializes pipes and redirection if needed.
@param commands: command struct array of all commands from input.
*/
void set_up_execution(Single_Command* commands)
{
	int flag = 0; // If there is output to the user, this will be set to 1.

	// Saves the original input and output file descriptors.
	int init_in = dup(STDIN_FILENO);
	int init_out = dup(STDOUT_FILENO);

	// File descriptors to be modified for each process.
	int infile = dup(init_in);
	int outfile;

	// Sets up the file descriptors for each command.
	for (int i = 0; commands[i].command; i++) 
	{
		dup2(infile, STDIN_FILENO);	// Redirect the input from the last pipe.
		close(infile);

		if (commands[i+1].command == NULL) // Check to see if on the last command.
		{
			flag = 1;

			if (commands[i].outfile) // Check to see if there was redirection for output.
			{
				flag = 0;
				// If so, open the outfile specified in the command struct.
				outfile = open(commands[i].outfile, O_WRONLY | O_TRUNC | O_CREAT, 0600);
			}
			else 
			{
				// Otherwise, use the original output.
				outfile = dup(init_out);
			}
		}
		else // Not on last command in the struct array.
		{
			// Create a pipe.
			int fd[2];
			if (pipe(fd) < 0) // Check if pipe failed.
			{
				fprintf(stderr, "Pipe failed.\n");
				exit(1);
			}

			outfile = fd[1];
			infile = fd[0];
		}

		// Redirect the output for the next pipe.
		dup2(outfile, STDOUT_FILENO);
		close(outfile);

		// Execute the command given now that redirection is set up.
		if (flag) // If there is output to the user.
		{
			execute_command(commands[i].command, flag);
		}
		else // Otherwise, execute normally.
		{
			execute_command(commands[i].command, flag);
		}
	}

	// Set I/O back to the originals once all commands are executed.
	dup2(init_in, 0);
	dup2(init_out, 1);
	close(init_in);
	close(init_out);
}


/*
Returns 1 if a "<" is used incorrectly in the command, 0 otherwise.
@param parsed: a pointer to a string array.
*/
int check_redirection(char** parsed)
{
	// Make sure the redirection operator is in the right place here, if not, ask for input again.
	int p_index = 0;
	int flag = 0;

	while (parsed[p_index])
	{
		// If a redirection operator is encountered and it is not at the first index, then it was used incorrectly.
		if (strcmp(parsed[p_index], "<") == 0 && p_index != 1)
		{
			flag = 1;
		}

		p_index++;
	}

	return flag;
}


/*
Launches and exits the shell.
*/
int main() 
{
	// Get a command from the user.
	char* input = prompt();
	char exit_code[] = "exit";
	Single_Command* args;

	// Parse the given line into its command/arguments.
	char** parsed = parse_input(input);

	// Keep asking for user input if redirection operator is used incorrectly.
	while (check_redirection(parsed))
	{
		printf("Incorrect use of redirection operator. Please try again.\n");
		input = prompt();
		parsed = parse_input(input);
	}

	while (strcmp(input, exit_code) != 0) // Keep looping until the user types exit.
	{
		// Create array of commands to be executed.
		args = create_commands(parsed);

		// Prepare necessary redirection and execute commands.
		set_up_execution(args);
		
		// Get user input again.
		input = prompt();
    	parsed = parse_input(input);

		// Keep asking for user input if redirection operator is used incorrectly.
		while (check_redirection(parsed))
		{
			printf("Incorrect use of redirection operator. Please try again.\n");
			input = prompt();
			parsed = parse_input(input);
		}
	}

	int i = 0; // Command index.
	int a = 0; // Argument index.
	while (args[i].command)
	{
		// Free all the arguments for the current command list.
		while (args[i].command[a])
		{
			free(args[i].command[a]);
			a++;
		}
		// Free the outfile for the current command list.
		if (args[i].outfile)
		{
			free(args[i].outfile);
		}

		// Free the command list.
		free(args[i].command);
		i++;
	}

	// Free the array of commands.
	free(args);
	// Free the list of parsed commands/arguments.
	free(parsed); 
	// Free the input.
	free(input);
	return 0;
}
		