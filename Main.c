#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<fcntl.h>
#include"helpReadDirectory.h"

//Protype Definitions Of header file helpReadDirectory.h
int parsePath(char **dirs);
void lookupPath(struct command_t* command, char **dir);
int parsePipe(char* str, char** strpiped); 		// function for finding pipe
void parseSpace(char* str, struct command_t* command);		//parse By Space
void freeDirectories(char**pathv); 	//free Read Directories From Environment getevn()

void printPrompt() {
/* Build the prompt string to have the machine name, current directory, or other desired information.*/
  char promptString[1000] ="uahmad>";
  printf("%s ", promptString);
  
}

int readCommand(char* buffer) {
  gets(buffer);
  if(strcmp(buffer,"exit")==0)
	  return 0;
  return 1;
} 


//Simple Command do fork() but Piped Command don't fork().
void execArgs(struct command_t* command,bool piped)
{
	int pid=0;
	if(!piped)
	pid=fork();
	if (pid == 0)
	{      
	    int fd0,fd1,i,in=0,out=0;
	    char input[64],output[64];

	    for(i=0;command->argv[i]!='\0'&& i<command->argc;i++)
	    {
	        if(strcmp(command->argv[i],"<")==0)
	        {        
	            command->argv[i]=NULL;
	            strcpy(input,command->argv[i+1]);
	            in=2;           
	        }
			else if(strcmp(command->argv[i],">")==0)
	        {   
				out=2;
				command->argv[i]=NULL;
	            strcpy(output,command->argv[i+1]);	
	        }         
			else if(strcmp(command->argv[i],">>")==0)
			{
				out=1;
				command->argv[i]=NULL;
	            strcpy(output,command->argv[i+1]);
			}
			
		}	
	    //if '<' char was found in string inputted by user
	    if(in)
	    {   
		    int fd0;
	        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
	            perror("Couldn't open input file");
	            exit(0);
	        }           
	        // dup2() copies content of fdo in input of preceeding file
	        dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 
	        close(fd0); // necessary
	    }
	    //if '>' char was found in string inputted by user 
	    if (out)
	    {
	        int fd1 ;
			if(out==2)
			{
				if ((fd1 = creat(output , 0644)) < 0) {
	        	    perror("Couldn't open the output file");
	        	    exit(0);
	        	}	
			}
			else
			{
				if ((fd1 = open(output , O_WRONLY|O_APPEND)) < 0) {
	        	    perror("Couldn't open the output file");
	        	    exit(0);
	        	}
			}           
	        dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
	        close(fd1);
	    }
	    //execvp(command->argv[i-1], command->argv);
		//char*q[4]={"ls","-l","sample",NULL};
	    execvp(command->argv[0],command->argv);
		//execv("/bin/ls",q);
		perror("execvp");
	    _exit(1);

	    // another syntax
	    /*      if (!(execvp(*argv, argv) >= 0)) {     // execute the command  
	            printf("*** ERROR: exec failed\n");
	            exit(1);
	     */ 
	 }
     else if((pid) < 0)
     {     
         printf("fork() failed!\n");
         exit(1);
     }
     else {                                  /* for the parent:      */
        wait(NULL);
     }
}	

	// if(command->name!=NULL)
	// {
	// 	int pid=fork();
	// 	if(pid==0)
	// 	{
	// 		//command->argv[command->argc]=NULL;
	// 		printf("\t\t*****OUTPUT:>*****\n");
	// 		execv(command->name,command->argv);
	// 		printf("Execution Failed\n");
	// 	}
	// 	wait(NULL);
	// }

// Function where the piped system commands is executed
void execArgsPiped(struct command_t*commands,int n_Pipes)
{
	int n_Commands=n_Pipes+1;
	// 0 is read end, 1 is write end
	int pipefd[MAXPIPES][2];
	for (int i = 0; i < MAXPIPES; i++)
	{
		if (pipe(pipefd[i]) < 0) {
		printf("\nPipe At %d could not be initialized",i);
		return;
		}
	}
	pid_t p1, p2;
	p1 = fork();
	if (p1 < 0) {
		printf("\nCould not fork");
		return;
	}
	if (p1 == 0) {
		//Child 1 executing..
		//It only needs to write at the write end
		close(pipefd[0][0]); 
		dup2(pipefd[0][1], STDOUT_FILENO);
		close(pipefd[0][1]);
		execArgs(&commands[0],true);
		// if (execvp(commands[0].argv[0], commands[0].argv) < 0) {
		// 	printf("\nCould not execute command 1..");
		// 	exit(0);
		// }
	}
	wait(NULL);
	for (int i = 0; i < n_Pipes; i++)
	{
		p2=fork();
		if (p2 < 0) {
		printf("\nCould not fork");
		return;
		}
		if (p2 == 0) { //Child Executing
			close(pipefd[i][1]);	//write end
			dup2(pipefd[i][0],STDIN_FILENO);//read from pipe
			close(pipefd[i][0]);
			if(i+1<n_Pipes){
				close(pipefd[i+1][0]);	//close read End of i+1 pipe
				dup2(pipefd[i+1][1], STDOUT_FILENO);
				close(pipefd[i+1][1]);
			}
			
			//char*q[5]={"tail","-5",NULL};
			execArgs(&commands[i+1],true);
			// if (execvp(commands[i+1].argv[0], commands[i+1].argv) < 0) {
			// 	printf("\nCould not execute command 2..");
			// 	exit(0);
			// }
		}
		close(pipefd[i][1]);
		wait(NULL);
		printf("\nI did Wait for %d\n",i);
	}
}

int buildCommands(char*str,struct command_t*commands,char**pathv,int*noOfPipes)
{
	char* strpiped[MAXPIPES+1]; // n pipes =n+1 commands
	int piped = 0;

	piped = parsePipe(str, strpiped);
	if (piped) {
		int n_Commands=0;
		for (int i = 0; strpiped[i]!=NULL ; i++,n_Commands++)
		{
			parseSpace(strpiped[i], &commands[i]);
		}
		*noOfPipes=n_Commands-1; //set No Of Pipes
		for (int i = 0; i < n_Commands; i++)//noOfpipes -1 = noOfCommands
		{
			commands[i].name=(char*)malloc(1000*sizeof(char));
			strcpy(commands[i].name,commands[i].argv[0]);
			lookupPath(&commands[i],pathv);
			printf("\naaa=%s\n",commands[i].name);
		}

		for (int i = 0; strpiped[i]!=NULL; i++)
		{
			for (size_t j = 0; commands[i].argv[j]!=NULL; j++)
			{
				printf("commands[%d].argv[%d]:%s \n",i,j,commands[i].argv[j]);
			}			
		}
	} else {
		parseSpace(str, &commands[0]);
		commands[0].name=(char*)malloc(1000*sizeof(char));
		strcpy(commands[0].name,commands[0].argv[0]);
		lookupPath(&commands[0],pathv);
		printf("\ncommandName=%s\n",commands[0].name);
		for (int i = 0;  commands[0].argv[i]!=NULL; i++)
		{
			printf("parsed[%d]: %s\n",i,commands[0].argv[i]);
		}
		*noOfPipes=0;
	}
	// if (ownCmdHandler(commands[0].argv[0])
	// 	return 0;
		return 1 + piped;
}


int main()
{
	struct command_t commands[MAXCOMMANDS];
	char commandLine[LINE_LEN];
	char*pathv[MAX_PATHS];
	for( int i=0; i < MAX_PATHS; i++)
		pathv[i] = (char*)malloc(MAX_PATH_LEN*sizeof(char));
	parsePath(pathv);
	int exit=-1;
	printf("Enter \"exit\" For Termination This Program\n");
	printPrompt();
	exit=readCommand(commandLine);
	int noOfPipes=0;
	while(true)
	{
		int piped=buildCommands(commandLine,commands,pathv,&noOfPipes);
		if(piped==1) //simple Command
		{
			execArgs(&commands[0],false);
		}
		else if(piped==2)//piped
		{
			execArgsPiped(commands,noOfPipes);
		}
		
		printf("\nExecution Completed.\n\n");
		printf("--Enter \"exit\" For Termination This Program\n");
		printPrompt();	
		exit=readCommand(commandLine);
	}
}

