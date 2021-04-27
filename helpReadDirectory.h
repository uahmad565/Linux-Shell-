#include"command_t.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void lookupPath(struct command_t* command, char **dir) 
{
  	char *result=(char*)malloc(MAX_PATH_LEN*sizeof(char));
//	 Check to see if file name is already an absolute path
  	int x;
  	if ((*command).name[0]== '/') {
			x=access((*command).name,F_OK);
			if(x==0){
				return;
			}
			else
			{
				free((*command).name);
				(*command).name=NULL;
				printf("Return Null on Looking Directories\n");
				return;
			}
  	}
	// Look in PATH directories.
	// Use access() to see if the file is in a dir.
  	x=-1;
  	int i;
  	for (i = 0; i  <  MAX_PATHS && dir[i]!='\0' && x!=0; i++) {
		strcpy(result,dir[i]);
		strcat(result,"/");
		strcat(result,(*command).name);
		x=access(result,F_OK);
 	}
  	if(x==0)
  	{
  		strcpy((*command).name,result);
		free(result);
		return;
  	}
  	free((*command).name);
  	(*command).name=NULL;
  	printf("Return Null on Looking Directories\n");
  	return;
}

int parsePath(char **dirs) {
  	char *pathEnvVar;
  	char *thePath;
	
  	pathEnvVar = (char *) getenv("PATH");
  	thePath = (char *) malloc((strlen(pathEnvVar) + 1)*sizeof(char));
  	strcpy(thePath, pathEnvVar);
  	char*pch = strtok (thePath," :");
  	int i=0;
  	while (pch != NULL && i<MAX_PATHS)
  	{
		  strcpy(dirs[i],pch);
		 // free(pch);
		  pch = strtok (NULL, " :");
		  i++;
  	}
  	dirs[i]='\0';
  	free(thePath);
  	return 0;  
}

// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
	int i;
	for (i = 0; i < MAXPIPES; i++) {
		strpiped[i] = strsep(&str, "|");
		if (strpiped[i] == NULL)
			{
				break;
			}
	}
	if (strpiped[1] == NULL)
		return 0; // returns zero if no pipe is found.
	else {
		return 1;
	}
}

//parse By Space
void parseSpace(char* str, struct command_t* command)
{
	int i;
	for (i = 0; i < MAXCOMMANDS; i++) {
		command->argv[i] = strsep(&str, " ");
		if (command->argv[i] == NULL)
			break;
		if (strlen(command->argv[i]) == 0)
			i--;
	}
	command->argc=i;
}


//free Read Directories From Environment getevn()
void freeDirectories(char**pathv)
{
	int i;
	for(i=0;i<MAX_PATHS;i++)
		free(pathv[i]);
	free(pathv);
}