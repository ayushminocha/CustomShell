/*Author : Ayush Minocha
 ** International Institute of Information Technology, Hyderabad **/

/** Custom Bash Shell
 * A custom made Linux shell that can run bash commands like in Terminal
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<errno.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>

typedef struct st
{
	int procid;
	char *procname;
	int flag; //to know whether process is currently running(1) or not(0)
	int bg;
	int cmd_proc; //cmd_proc=1 for process and 0 for cmds by default its value is 1
	char *intake;
}all;
all hist[200];

int checkbg(char *s)
{
	char t;
	int len=strlen(s);
	t=s[len-1];
	if(t=='&')
		return 1;
	else
		return 0;
}

void bg_handler(int sig)
{
	pid_t pid;
	int t;

	pid = wait(NULL);
	for(t=0;t<200;t++)
	{
		if(hist[t].procid==pid)
		{
			hist[t].flag=0;
		}
	}
	printf("Pid %d exited.\n", pid);
}

void fg_handler(int sig){
}

/* types of input :-
   0 - normal
   1 - i/p redirection  y < z
   2 - o/p redirection  y > z
   3 - i/p + o/p redirection y < x > z             y has to be a cmd and x,z are files
 */

int check_type(char checkchar,int t)
{
	if((checkchar=='<')&&(t==0))
		return 1;
	else if((checkchar=='>')&&(t==0))
		return 2;
	else if((checkchar=='>')&&(t==1))
		return 3;

	return 0;
}

int main()
{
	sigset_t st;
	sigaddset(&st,SIGINT);
	sigaddset(&st,SIGTSTP);
	sigprocmask(SIG_BLOCK,&st,NULL);
	int tp,i,j,cbg; //cbg for checking bg process
	char input[200];
	char input_copy[200];
	char pipe_input_copy[200];
	char *usr=getenv("USER");
	char host[200];
	char cwd[200];
	char bgstr[50];
	int tof=0;
	int filectr=0; //to check the tof input
	int stdin,stdout;
	stdin=dup(0);
	stdout=dup(1);
	int fdi,fdo;
	int nthcmd; // for !histn
	int hist_input_ctr=0;								// to be used with hist family cmds

	for(i=0;i<200;i++)
	{
		hist[i].flag=1;	
		hist[i].procname=(char*)calloc(50,sizeof(char));
		hist[i].bg=0;
		hist[i].cmd_proc=1;//by default a process
		hist[i].intake=(char*)calloc(100,sizeof(char));
	}
	int ctr=0;
	gethostname(host,200);
	char *path=getenv("HOME");
	char home[100];
	strcpy(home,path);
	char**str;
	char**file;
	char**pipe_proc;
	char**hist_input;
	
	hist_input=(char**)calloc(200,sizeof(char));					// to be used with hist family cmds
	for(i=0;i<200;i++)
		hist_input[i]=(char*)calloc(200,sizeof(char));
	
	str=(char**)calloc(50,sizeof(char*));
	for(i=0;i<50;i++)
		str[i]=(char*)calloc(50,sizeof(char));



	while(1)
	{
		dup2(stdin,0);
		dup2(stdout,1);

		printf("%s@%s:%s>",usr,host,getcwd(cwd,200));
		int pfd[50][2];
		int pipe_ctr=0;										// TO BE USED FOR PIPES
		int pipe_flag=0;									//PIPE_FLAG=1 IF PIPE PRESENT
		int proc_pos=0;										//STORING PROCS IN PIPE_PROC
		while(scanf(" %[^\n]",input)==EOF);							//taking input

label1:	
		//printf("%s\n",input);
		////////////////////////////////////
		dup2(stdin,0);
		dup2(stdout,1);

		//printf("%s@%s:%s>",usr,host,getcwd(cwd,200));
		
		 pipe_ctr=0;										// TO BE USED FOR PIPES
		 pipe_flag=0;									//PIPE_FLAG=1 IF PIPE PRESENT
		 proc_pos=0;
		///////////////////////////////////
		strcpy(hist_input[hist_input_ctr],input);
		hist_input_ctr++;
		
		for(i=0;i<200;i++)
		{
			pipe_input_copy[i]='\0';
		}
		strcpy(pipe_input_copy,input);								//INPUT COPIED FOR PIPE

		// CHECK THE INPUT FOR PIPES
		for(i=0;i<strlen(input);i++)
		{
			if(input[i]=='|')
				pipe_ctr++;								//NO OF PROCESS = PIPE_CTR+1
		}
		if(pipe_ctr>0)
			pipe_flag=1;									//PIPE DETECTED

		pipe_proc=(char**)calloc(50,sizeof(char*));						// REQUIRED FOR STORING PROCESS OF PIPES 
		for(i=0;i<50;i++)
			pipe_proc[i]=(char*)calloc(50,sizeof(char));

		//PARSING OF INPUT FOR PIPES
		char *pipetoken=strtok(pipe_input_copy,"|");
		while(pipetoken!=NULL)
		{
			strcpy(pipe_proc[proc_pos],pipetoken);
			proc_pos++;
			pipetoken=strtok(NULL,"|");
		}
		int proc_count;

		for(proc_count=0;proc_count<=pipe_ctr;proc_count++)					//THE FOR LOOP FOR PIPING
		{
			strcpy(input,pipe_proc[proc_count]);
			if(pipe_ctr)
			{	pipe(pfd[proc_count]);							//CREATING PIPE
				dup2(pfd[proc_count][1],1);
				if(proc_count==pipe_ctr)						//FOR THE LAST CMD
					dup2(stdout,1);
			}


//label1: 										// label for goto------------------------
			////////*******************************/////////////////
			
			signal(SIGCHLD, bg_handler); 									//signal
			cbg=0;
			errno=0;
			i=0;
			//tof=0;
			filectr=0;
			str=(char**)calloc(50,sizeof(char*));
			for(i=0;i<50;i++)
				str[i]=(char*)calloc(50,sizeof(char));

			file=(char**)calloc(50,sizeof(char*));					// required for redirection 
			for(i=0;i<50;i++)
				file[i]=(char*)calloc(50,sizeof(char));

			for(i=0;i<200;i++)
			{
				input_copy[i]='\0';
			}       
			////////*******************************/////////////////

			//fprintf(stderr,"%s\n",input);
			strcpy(hist[ctr].intake,input);

			strcpy(input_copy,input);
			for(i=0;i<strlen(input);i++)		// check the type of input
			{
				if((input_copy[i]=='<')||(input_copy[i]=='>'))
					tof=check_type(input_copy[i],tof);
			}
			char *filetoken=strtok(input_copy,"><");
			while(filetoken!=NULL)
			{
				strcpy(file[filectr],filetoken);
				filectr++;
				filetoken=strtok(NULL,"><");
			}
			for(i=1;i<filectr;i++)
			{
				filetoken=strtok(file[i]," \t\n");
				strcpy(file[i],filetoken);
			}
			strcpy(input,file[0]);
			if(tof==1)
			{
				fdi=open(file[1],O_RDWR|O_CREAT);
				dup2(fdi,0);
			}
			if(tof==2)
			{
				fdo=creat(file[1],S_IRUSR|S_IWUSR);
				printf("\n");
				dup2(fdo,1);
			}
			if(tof==3)
			{
				fdi=open(file[1],O_RDWR|O_CREAT);
				dup2(fdi,0);
				fdo=creat(file[2],S_IRUSR|S_IWUSR);
				printf("\n");
				dup2(fdo,1);
			}

			char *token=strtok(input," \t\n");								//*******//
			while(token!=NULL)
			{
				// the exit thing
				if(strcmp(token,"exit")==0)
					return 0;

				//implementing cd	
				else if(strcmp(token,"cd")==0)
				{
					strcpy(hist[ctr].procname,token);						//////////////////////////////////////
					hist[ctr].cmd_proc=0;
					ctr++;
					token=strtok(NULL," \t\n");
					if(token==NULL)
					{
						//printf("home=%s\n",home);
						tp=chdir(home);
						//printf("tp=%d\n",tp);
						/*if(tp==0)
						  {
						  getcwd(cwd,200);
						  printf("%s\n",cwd);
						  strcpy(path,cwd);
						  break;
						  }*/
						if(tp!=0)
						{
							printf("error\n");
						}
					}
					else
					{
						tp=chdir(token);
						if(tp!=0)
						{
							//printf("no such file or directory\n");
							perror(NULL);
							break;
						}
						else
						{
							getcwd(cwd,200);
							//printf("%s\n",cwd);
							strcpy(path,cwd);
							break;
						}

					}
					
				}
				else if(strncmp(token,"hist",4)==0)         						//history
				{
					if(strcmp(token,"hist")==0)
					{
						strcpy(hist[ctr].procname,token);						//////////////////////////////////////
						hist[ctr].cmd_proc=0;
						ctr++;
						for(j=0;j<hist_input_ctr;j++)
						{
							printf("%d > %s \n",j+1,hist_input[j]);
						}
					}
					//printf("the nth char = %c\n",token[3]);
					else if(isdigit(token[4]))
					{
						strcpy(hist[ctr].procname,token);						//////////////////////////////////////
						hist[ctr].cmd_proc=0;
						ctr++;
						//-----------------------//
						int cas;//=token[4];
						//cas=cas-48;
						cas=atoi(token+4);
						//printf("cas=%d\n",cas);
						if(cas<=hist_input_ctr)
						{
							for(j=hist_input_ctr-cas;j<hist_input_ctr;j++)
							{
								printf("%d > %s \n",j+1,hist_input[j]);
							}
						}
						else
						{
							for(j=0;j<hist_input_ctr;j++)
							{
								printf("%d > %s \n",j+1,hist_input[j]);
							}
						}
					}


					token=strtok(NULL," \t\n");
					while(token!=NULL)
						token=strtok(NULL," \t\n");

				}

				else if(strncmp(token,"!hist",5)==0)					// implementing !histn command
				{
					strcpy(hist[ctr].procname,token);						//////////////////////////////////////
					//printf("%s\n",hist[nthcmd].intake);
					hist[ctr].cmd_proc=0;
					ctr++;
					nthcmd=atoi(token+5);
					//printf("%d\n",nthcmd);
					nthcmd=nthcmd-1;
					//printf("%s\n",hist_input[nthcmd]);
					
					token=strtok(NULL," \t\n");
					while(token!=NULL)
						token=strtok(NULL," \t\n");
					//-------------------
					for(i=0;i<50;i++)
						free(str[i]);

					free(str);

					for(i=0;i<50;i++)
						free(file[i]);

					free(file);
					//-------------------
					//exit(0);
					dup2(stdin,0);
					dup2(stdout,1);
					printf("%s\n",hist_input[nthcmd]);
					strcpy(input,hist_input[nthcmd]);
					goto label1;

				}


				//implementing ls and other cmds
				else
				{
					/*-----------------*/
					if(strcmp(token,"pid")==0)								/** block of pid **/
					{
						strcpy(hist[ctr].procname,token);						//////////////////////////////////////
						hist[ctr].cmd_proc=0;
						ctr++;
						token=strtok(NULL," \t\n");
						if(token==NULL)
						{
							printf("command name:./a.out process id:%d\n",getpid());
						}
						else if(strcmp(token,"all")==0)
						{
							for(j=0;j<ctr;j++)
							{
								if(hist[j].cmd_proc==1)
								{
									printf("command name:%s process id:%d\n",hist[j].procname,hist[j].procid);
								}
							}
							token=strtok(NULL," \t\n");
						}
						else if(strcmp(token,"current")==0)
						{
							printf("List of currently executing processes spawned from this shell:\n");
							for(j=0;j<200;j++)
							{
								if(hist[j].bg==1 && hist[j].flag==1 && hist[j].cmd_proc==100)
									printf("command name:%s process id:%d\n",hist[j].procname,hist[j].procid);
							}
							token=strtok(NULL," \t\n");
						}
						else
						{
							printf("command not found\n");
							token=strtok(NULL," \t\n");
						}
					}
					else											/** other cmds block **/
					{
						strcpy(str[0],token);
						i=1;
						token=strtok(NULL," \t\n");
						if(token==NULL)
						{
							cbg=checkbg(str[0]); //checking for the background process
							if(cbg)
							{
								strncpy(bgstr,str[0],(strlen(str[0])-1));
								strcpy(str[0],bgstr);
								//printf("newstring=%s\n",str[0]);
							}
						}
						else
						{
							while(token!=NULL)
							{
								strcpy(str[i],token);
								i++;
								token=strtok(NULL," \t\n");
							}
							cbg=checkbg(str[i-1]); //checking for the background process
							if(cbg)
							{
								str[i-1]=NULL;
							}

						}

						str[i]=NULL;
						if(cbg==0)
						{
							signal(SIGCHLD,fg_handler);				//signal
						}
						int pid=fork();
						if(pid==0)
						{
							execvp(str[0],str);
							if(errno!=0)
							{
								perror(NULL);
								//exit(0);
								return 0;
							}

						}
						if(pid>0)
						{
							if(errno==0)
							{
								strcpy(hist[ctr].procname,str[0]);
								hist[ctr].flag=1;
								hist[ctr].procid=pid;
								if(cbg)
									hist[ctr].bg=1;
								ctr++;
							}
							if(cbg==0)
							{
								//signal(SIGCHLD,fg_handler);			
								waitpid(-1,NULL,0);
								signal(SIGCHLD,bg_handler);				//signal
							}
							else if(cbg)
							{
								signal(SIGCHLD,bg_handler);				//signal
							}
						}

						/********************/
					}			
				}
				if(tof)
				{
					if(tof==1)
					{
						dup2(stdin,0);
					}
					else if(tof==2)
					{
						dup2(stdout,1);
					}
					else if(tof==3)
					{
						dup2(stdin,0);
						dup2(stdout,1);
					}
					tof=0;
				}

			}
			
			if(pipe_ctr)
			{	
				dup2(pfd[proc_count][0],0);
				close(pfd[proc_count][1]);
			}
			//freeing
			for(i=0;i<50;i++)
				free(str[i]);

			free(str);

			for(i=0;i<50;i++)
				free(file[i]);

			free(file);
			
		}//------------------------------------------------------ending the for loop
		
		
	
	}
	for(i=0;i<200;i++)
	{
		free(hist[i].procname);
	}

}
