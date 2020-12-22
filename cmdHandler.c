/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Corey Wong
 * GNumber: 01073746
 */
//handle kill signals
//handle kill -18
//handle kill -19
//handle sleep not stopping properly
#include "logging.h"
#include "shell.h"
#define MAX_ARGS 50
#define BUILT_IN_COUNT 6
#define MAX_PROCESS 100

struct Process 
{
  int jid;//job id
  int pid;
  int fg;//0 indicates bg process, 1 indicates fg
  char* state;//"Running" or "Stopped"
  char cmdline[MAXLINE];
  struct Process* next;//Linked list data struct
};


void blocksig_chld();
void blocksigs();
void unblocksigs();
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void fgHandler(int child_status, struct Process* fg);
int parseCmd(char** cmd, char* cmdline);
void addNewBG(struct Process* newbg);
struct Process* remBG(int pid);
void addBG(struct Process* bg);
int getMaxJid();
struct Process* getJob(int pid);
struct Process* getTail();
void getArgv(char** cmd, char** argv, int arg_count);
int isBuiltIn(char* cmd);
void execBuiltIn(char** cmd, int cmd_i);
void execJobs();
void execFG(char** cmd, int cmd_i);
void waitFG(struct Process* fg);
void waitsig();
void execBG(char** cmd, int cmd_i);
void execKill(char** cmd, int cmd_i);
int callKill(int pid, int sig);
struct Process* getProcess(int jid);

int handleFiles(char** cmd, int cmd_i, int* fd1, int* fd2);

int bg_count = 0;//number of bg jobs
struct Process* head;//dummy head
struct Process* tail;//real tail
struct Process* newbg;//the current process running, or the process to be added to background
volatile pid_t pid;//pid received by sigchld, used to flag sigsuspend calls
sigset_t old;//default masking
int main() 
{
    char cmdline[MAXLINE]; /* Command line */
    head = (struct Process*)malloc(sizeof(struct Process));//dummy head
    newbg = (struct Process*)malloc(sizeof(struct Process));
    newbg->fg = 0;
    //tail = (struct Process*)malloc(sizeof(struct Process));//real tail 
    
    //get default handler mask
    sigprocmask(SIG_SETMASK, NULL, &old);
    //install SIGCHLD handler
    struct sigaction new_sigchld;
    new_sigchld.sa_handler = sigchld_handler;
    new_sigchld.sa_flags = 0;
    sigaction(SIGCHLD, &new_sigchld, NULL);

    //install SIGINT handler
    struct sigaction new_sigint;
    new_sigint.sa_handler = sigint_handler;
    new_sigint.sa_flags = 0;
    sigaction(SIGINT, &new_sigint, NULL);

    //install SIGTSTP handler
    struct sigaction new_sigtstp;
    new_sigtstp.sa_handler = sigtstp_handler;
    new_sigtstp.sa_flags = 0;
    sigaction(SIGTSTP, &new_sigtstp, NULL);
    
    while (1) {
      /* Print prompt */
      log_prompt();
      /* Read a line */
      // note: fgets will keep the ending '\n'
      if (fgets(cmdline, MAXLINE, stdin)==NULL)
	{
	  if (errno == EINTR)
	    continue;
	  exit(-1); 
	}
      
      if (feof(stdin)) {
	exit(0);
      }
      /* Parse command line and evaluate */
      char** cmd = malloc(sizeof(char*) * MAX_ARGS);
      int cmd_i = 0;
      
      //remove '\n' char
      cmdline[strlen(cmdline) - 1] = '\0';
      //make new process
      newbg = (struct Process*)malloc(sizeof(struct Process));
      strcpy(newbg->cmdline, cmdline);
      newbg->state = "Running";
      
      cmd_i = parseCmd(cmd, cmdline);
      /* Parse command for execution */
      int arg_count = cmd_i;      
      int bg = 0;//flag for command being a bg process
      int builtIn = isBuiltIn(cmd[0]);//determines built in command or not
      if(strcmp(cmd[cmd_i - 1], "&") == 0)
	{
	  arg_count--;
	  bg = 1;
	}

      //execute command, handle different types of commands
      if(!builtIn)
	{      
    	  if((newbg->pid = fork()) == 0)
	    {
	      //child, perform program execution
	      
	      //handle file redirection
	      int fd1 = -1;
	      int fd2 = -1;
	      
	      int i = cmd_i - 6;
	      for(; i < cmd_i; i++)
		{
		  if(i < 1)
		    continue;
		  //file input
		  if(strcmp(cmd[i], "<") == 0)
		    {
		      arg_count -= 2;
		      fd1 = open(cmd[i + 1], O_RDONLY);
		      if(fd1 == -1)
			log_file_open_error(cmd[i + 1]);
		      else
			dup2(fd1, STDIN_FILENO);
		      
		    }
		  //file output
		  else if(strcmp(cmd[i], ">") == 0)
		    {
		      arg_count -= 2;
		      fd2 = open(cmd[i + 1], O_WRONLY | O_CREAT, 0600);
		      if(fd2 == -1)
			log_file_open_error(cmd[i + 1]);
		      else
			dup2(fd2, STDOUT_FILENO);
		    }
		  else if(strcmp(cmd[i], ">>") == 0)
		    {
		      arg_count -= 2;
		      fd2 = open(cmd[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0600);
		      if(fd2 == -1)
			log_file_open_error(cmd[i + 1]);
		      else
			dup2(fd2, STDOUT_FILENO);
		    }
		}
	      
	      
	      //create list of args
	      char** argv = malloc(sizeof(char*) * (arg_count + 1));
	      getArgv(cmd, argv, arg_count);
	      
	      unblocksigs();
	      int err = 0;
	      //execute command
	      if((err = execve(cmd[0], argv, 0)) == -1)
		{
		  //may need to handle them
		  log_command_error(cmdline);
		}
	      
	      if(fd1 != -1)
		  close(fd1);
	      if(fd2 != -1)
		  close(fd2);	      
	      exit(err);
	    }	
	  else
	    {
	      //parent, check for bg/fg process and handle waiting appropriately
	      setpgid(newbg->pid, 0);
	      if(bg == 1)
		{
		  blocksig_chld();
		  log_start_bg(newbg->pid, newbg->cmdline);
		  newbg->fg = 0;
		  addBG(newbg);
		}
	      else
		{
		  blocksigs();
		  newbg->jid = 0;
		  newbg->fg = 1;
		  waitFG(newbg);
		}	      
	    }
	}
      else//built in command
	{
	  execBuiltIn(cmd, cmd_i);
	}
      unblocksigs();
      //free mallocd data
      free(cmd);
    }
} 
/*
 * Blocks sigtstp, sigint, sigchld
 */
void blocksigs()
{
  sigset_t sig_key;
  sigemptyset(&sig_key);
  sigaddset(&sig_key, SIGINT);
  sigaddset(&sig_key, SIGTSTP);
  sigaddset(&sig_key, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sig_key, NULL);
}
/*
 * Blocks sigchld
 */
void blocksig_chld()
{
  sigset_t sig_chld;
  sigemptyset(&sig_chld);
  sigaddset(&sig_chld, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sig_chld, NULL);
}
/*
 * Returns blocked signals to default
 */
void unblocksigs()
{
  sigprocmask(SIG_SETMASK, &old, NULL);
}
/*
 * Parses string into arguments separated by spaces
 * Returns number of arguments
 */
int parseCmd(char** cmd, char* cmdline)
{
  char* s = strtok(cmdline, " ");
  int arg_count = 0;
  while(s != NULL)
    {
      //no argument between this and last space
      if(strlen(s) == 0)
	continue;
      cmd[arg_count++] = s;
      s = strtok(NULL, " ");
    }
  return arg_count;
}
/*
 * Handles termination of foreground process
 */
void fgHandler(int child_status, struct Process* fg)
{
  if(WIFEXITED(child_status) && child_status == 0)
    {
      log_job_fg_term(fg->pid, fg->cmdline);
      free(fg);
    }
  else if(WIFSTOPPED(child_status))
    log_job_fg_stopped(fg->pid, fg->cmdline);
  else if(WIFCONTINUED(child_status))
    log_job_fg_cont(fg->pid, fg->cmdline);
  else if(WIFSIGNALED(child_status))
    {
      log_job_fg_term_sig(fg->pid, fg->cmdline);
      free(fg);
    }
  fg->fg = 0;
}
void sigchld_handler(int sig)
{
  //recieved state change signal from child, handle different cases
  int child_status;
  pid_t prev = -1;
  printf("chidl\n");
  while((pid = waitpid(-1, &child_status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
      //check for fg/bg
      printf("found pid\n");
      prev = pid;
      struct Process* child = getJob(pid);
      newbg = child;//unnecessary?
      if(child->fg == 1)
	{
	  fgHandler(child_status, child);
	}
      else
	{
	  remBG(pid);
	  if(WIFEXITED(child_status) && child_status == 0)
	    {
	      log_job_bg_term(pid, child->cmdline);
	      free(remBG(pid));
	    }
	  else if(WIFSIGNALED(child_status))
	    {
	      log_job_bg_term_sig(pid, child->cmdline);
	      free(remBG(pid));
	    }
	  else if(WIFCONTINUED(child_status))
	    {
	      log_job_bg_cont(pid, child->cmdline);
	      //child->state = "Running";
	    }
	  else if(WIFSTOPPED(child_status))
	    {
	      log_job_bg_stopped(pid, child->cmdline);
	      //	      child->state = "Stopped";
	    }
	}
    }
  //mark pid as previous pid
  printf("prev: %d\n", prev);
  pid = prev;
}
void sigint_handler(int sig)
{
  printf("int\n");
  blocksig_chld();
  if(newbg->fg == 1)//fg process running, sigint process
    {
      callKill(newbg->pid, sig);
    }
  unblocksigs();
}
void sigtstp_handler(int sig)
{
  blocksig_chld();
  if(newbg->fg == 1)
    {
      if(callKill(newbg->pid, SIGSTOP) != -1)
	{
	  newbg->fg = 0;
	  newbg->state = "Stopped";
	  addBG(newbg);
	}
    }
  unblocksigs();
}
/*
 * Adds process to bg list
 * Handles potential processes that have been assigned jids
 */
void addBG(struct Process* bg)
{
  bg_count++;
  if(bg->jid == 0)//new process, assign nonnegative jid
      addNewBG(bg);
  else//old process, simply place in proper position in list
    {
      struct Process* temp = head;
      while(temp->next != NULL && temp->next->jid < bg->jid)
	temp = temp->next;
      bg->next = temp->next;
      temp->next = bg;
    }
  tail = getTail();
  bg->fg = 0;
}
/*
 * Adds new process to bg list
 */
void addNewBG(struct Process* newbg)
{
  if(head->next == NULL)
    {
      newbg->jid = 1;
      head->next = newbg;
    }
  else
    {
      newbg->jid = tail->jid + 1;
      tail->next = newbg;
    }
  tail = newbg;  
}
/*
 * Removes the process with pid if found in bg list
 */
struct Process* remBG(int pid)
{
  struct Process* temp = head->next;
  struct Process* prev = head;
  
  while(temp != NULL && temp->pid != pid)
    {
      prev = temp;
      temp = temp->next;
    }
  //dont remove if NULL(no pid found)
  if(temp != NULL)
    {
      prev->next = temp->next;
      bg_count--;
    }
  return temp;
}

/*
 * Gets process based on jid
 * Returns null if none found
 */
struct Process* getProcess(int jid)
{
  struct Process* temp = head->next;
  while(temp != NULL)
    {
      if(temp->jid == jid)
	return temp;
      temp = temp->next;
    }
  return NULL;
}
int getMaxJid()
{
  return tail->jid;
}


struct Process* getTail()
{
  return tail;
}

struct Process* getJob(int pid)
{
  struct Process* temp = head->next;
  while(temp != NULL && temp->pid != pid)
      temp = temp->next;

  if(temp != NULL && temp->pid == pid)
    return temp;
  else//not in bg list, must be fg
    return newbg;

}
void getArgv(char** cmd, char** argv, int arg_count)
{
  int i = 0;
  for(i = 0; i < arg_count; i++)
    {
      argv[i] = cmd[i];
    }
  argv[i + 1] = (char *)0;
}

//checks if a command is built in or not
//returns 1 for built in, 0 for not

int isBuiltIn(char* cmd)
{
  char *built_in[BUILT_IN_COUNT] = {"fg", "bg", "jobs", "kill", "quit", "help"};

  int i = 0;
  for(; i < BUILT_IN_COUNT; i++)
    {
      if(strcmp(built_in[i], cmd) == 0)
	return 1;
    }
  return 0;
}

void execBuiltIn(char** cmd, int cmd_i)
{
  blocksigs();
  char* user_cmd = cmd[0];
  if(strcmp(user_cmd, "fg") == 0)
    execFG(cmd, cmd_i);
  else if(strcmp(user_cmd, "bg") == 0)
    execBG(cmd, cmd_i);
  else if(strcmp(user_cmd, "kill") == 0)
    execKill(cmd, cmd_i);
  else if(strcmp(user_cmd, "help") == 0)
    log_help();
  else if(strcmp(user_cmd, "jobs") == 0)
    execJobs();
  else if(strcmp(user_cmd, "quit") == 0)
    {
      log_quit();
      exit(0);
    }
  unblocksigs();
}
/*
 * Executes bg with cmd args
 */
void execBG(char** cmd, int cmd_i)
{
  //check for proper input and bg jobs
  if(cmd_i < 2)
    {
      log_job_bg_error();
      return;
    }
  else if(bg_count == 0)
    {
      log_no_bg_error();
      return;
    }
  //check for existing job
  int job_id = atoi(cmd[1]);
  struct Process* bg = getProcess(job_id);
  if(bg != NULL)
    {
      //exists, execute bg cmd
      if(strcmp(bg->state, "Stopped") == 0)
	{
	  log_job_bg(bg->pid, bg->cmdline);	
	  bg->state = "Running";
	  if(callKill(bg->pid, SIGCONT) == -1)
	    { 
	      log_job_bg_fail(bg->pid, bg->cmdline);
	    }
	}
    }
  else//no bg with job_id found
    log_bg_notfound_error(job_id);
}
void execFG(char** cmd, int cmd_i)
{
  //check for no bg jobs
  if(bg_count == 0)
    {
      log_no_bg_error();
      return;
    }

  //get appropriate job id
  int job_id;
  if(cmd_i > 1)
    job_id = atoi(cmd[1]);
  else
    job_id = tail->jid;

  //check for valid job id
  struct Process* fg = getProcess(job_id);
  if(fg != NULL)
    {
      //valid, process fg cmd
      newbg = fg;
      newbg->fg = 1;
      //check if stopped or running
      if(strcmp(newbg->state, "Running") == 0)
	log_job_fg(newbg->pid, newbg->cmdline);
      else if(callKill(newbg->pid, SIGCONT) != -1)
	{
	  newbg->state = "Running";
	}
      else//resume failed, report back
	{
	  log_job_fg_fail(newbg->pid, newbg->cmdline);
	  return;
	}
      remBG(newbg->pid);
      //wait for termination of fg process
      printf("waiting\n");
      waitFG(newbg);
      printf("done\n");
    }
  else//no process found
    log_fg_notfound_error(job_id);
}
/*
 * Waits for foreground process to terminate
 */
void waitFG(struct Process* fg)
{
  int child_status;
  fg->fg = 1;//UNNCESSARY?
  pid_t pidres = 0;//response from waitpid
  pid_t pidfg = fg->pid;
  pid = 0;
  //continue until fg process terminates normally(through waitpid) or handler deals with it
  printf("FGPID: %d\n", pidfg);
  while(pidfg != pidres && pidfg != pid)
    {
      //wait for signal
      printf("waitfg pid: %d fgpid: %d\n", pid, pidfg);
      waitsig();
      //wait for fg process to terminate normally or handler to acces info
      pidres = waitpid(pidfg, &child_status, WNOHANG);
    }
  
  //check for regular termination, handlers handle other cases
  if(pidfg > 0)
    fgHandler(child_status, fg);
}
/*
 * Waits until a signal is received
 */
void waitsig()
{
  pid = 0;
  while(!pid)
    sigsuspend(&old);
}
/*
 * Executes jobs with the given cmd arguments
 */
void execJobs()
{
  log_job_number(bg_count);
  struct Process* temp = head->next;
  //print each job in list
  while(temp != NULL)
    {
      log_job_details(temp->jid, temp->pid, temp->state, temp->cmdline);
      temp = temp->next;
    }
}
/*
 * Executes kill with the given cmd arguments
 */
void execKill(char** cmd, int cmd_i)
{
  int sig = atoi(strtok(cmd[1], "-"));
  int pid_kill = atoi(cmd[2]);
  callKill(pid_kill, sig);
}

/*
 * Calls kill() and handles logging errors
 */
int callKill(int pid, int sig)
{
  int ret = kill(pid, sig);
  //wait for kill to return
  waitsig();
  if(ret == -1)
    log_kill_error(pid, sig);
  return ret;
}
