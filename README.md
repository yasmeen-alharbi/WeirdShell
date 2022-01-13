<h1 align="center"> WeirdShell </h1>
<h3 align="center"> A simple shell that implements pipes and standard output redirection by reading input from right to left. </h3>

<h2 align="left"> To Run: </h2>

- Run the following commands to enter the shell:
    - 'make'
    - 'build/wrdsh'

<h2 align="left"> Overview: </h2>

<h3 align="left"> wrdsh.c: </h3>

- Contains a structure that keeps track of one simple command (i.e. "ls -l") and holds the outfile if there is redirection
- Once the shell has taken input from the user, we parse the input based on whitespace
- We then split the command(s) based on any redirection symbols or pipes found and put each simple command within its own struct
- Next, we reverse the order of commands so that they will be executed in proper order
- The shell then keeps running until the user enters "exit"

<h2 align="left"> Limitations: </h2>

- When allocating space for commands, we had a max size of the macro BUFSIZ (which is machine dependent), so the user is limited with the amount of characters and commands they can enter
- Memory is freed, though there may be some other memory leaks that we were unable to figure out
- The output redirection file will always be the first argument in the command line
- If the redirection symbol is found anywhere that isn't the second argument, an error message will print and the command will not be executed
- When the command 'ls' is executed, it prints each file/directory on a newline
