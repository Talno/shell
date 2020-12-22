/* Do Not Modify This File */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"

/* Outputs the Help: All the Built-in Commands */
void log_help() {
  printf("Welcome to GMSH (GMU Shell)!\n");
  printf("Built-in Commands: fg, bg, jobs, kill, quit, help.\n");
  printf("\tkill -SIGNAL PID\n");
  printf("\tfg [JOBID]\n");
  printf("\tbg JOBID\n");
}

/* Outputs the message after running quit */
void log_quit(){
  printf("Thanks for using GMSH! Good-bye!\n");
}

/* Outputs the prompt */
void log_prompt() {
  // fprintf to stderr to display without buffering
  fprintf(stderr, "GMSH>> ");
}

/* Output when the string cannot be parsed properly */
void log_parse_error(char *line) {
  printf("Error: Cannot parse the string \"%s\"\n", line);
}

/* Output when the command is not found
 * eg. User typed in ls instead of /bin/ls and exec returns an error
 */ 
void log_command_error(char *line) {
  printf("%s: Command cannot load\n", line);
}

/* Output when starting a background process */
void log_start_bg(int pid, char *cmd) {
  printf("Background Job %d:%s Started\n", pid, cmd);
} 

/* Output when failing to send a signal to a process */
void log_kill_error(int pid, int sig) {
  printf("Failed: kill(%d, %d)\n", pid, sig);
}

/* Output when using bg on a process */
void log_job_bg(int pid, char *cmd) {
  printf("Command bg applied to %d:%s\n", pid, cmd);
}

/* Output when there are no jobs to bg */
void log_no_bg_error() {
  printf("No background job available!\n");
}

/* Output when the user fails to specify a job for bg */
void log_job_bg_error() {
  printf("bg: Need to specify a job\n");
}

/* Output when the given job is not found for bg */
void log_bg_notfound_error(int job_id) {
  printf("bg %d: job not found!\n", job_id);
}

/* Output when the given job is not found for bg */
void log_fg_notfound_error(int job_id) {
  printf("fg %d: job not found!\n", job_id);
}

/* Output when the job is moved to the foreground with fg */
void log_job_fg(int pid, char *cmd) {
  printf("Background Job %d:%s Moved to Foreground\n", pid, cmd);
}

/* Output when bg fails */
void log_job_bg_fail(int pid, char *cmd) {
  printf("bg: Fail to Continue %d:%s\n", pid, cmd);
}

/* Output when fg fails */
void log_job_fg_fail(int pid, char *cmd) {
  printf("fg: Fail to Continue %d:%s\n", pid, cmd);
}

/* Output when a foreground job terminated normally.
 * (Signal Handler Safe Outputting)
 */
void log_job_fg_term(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Foreground Job %d:%s Terminated Normally\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a foreground job terminated due to a signal
 * (Signal Handler Safe Outputting)
 */
void log_job_fg_term_sig(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Foreground Job %d:%s Terminated by Signal\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a foreground job was continued.
 * (Signal Handler Safe Outputting)
 */
void log_job_fg_cont(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Foreground Job %d:%s Continued\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a background job terminated normally.
 * (Signal Handler Safe Outputting)
 */
void log_job_bg_term(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Background Job %d:%s Terminated Normally\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a background job terminated by a signal.
 * (Signal Handler Safe Outputting)
 */
void log_job_bg_term_sig(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Background Job %d:%s Terminated by Signal\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a background job was continued.
 * (Signal Handler Safe Outputting)
 */
void log_job_bg_cont(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Background Job %d:%s Continued\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a foreground job was stopped.
 * (Signal Handler Safe Outputting)
 */
void log_job_fg_stopped(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Foreground Job %d:%s Stopped\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output when a background job was stopped.
 * (Signal Handler Safe Outputting)
 */
void log_job_bg_stopped(int pid, char *cmd) {
  char buffer[255] = {0};
  sprintf(buffer,"Background Job %d:%s Stopped\n", pid, cmd);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/* Output on file open errors */
void log_file_open_error(char *file_name) {
  printf("Cannot open file %s!\n", file_name);
}

/* Output to list the job counts */
void log_job_number(int num_jobs){
  printf("=====%d Jobs=====\n", num_jobs);
}

/* Output to detail a single job */
void log_job_details(int job_id, int pid, char *state, char *cmd){
  printf("%d: %d: %s %s\n", job_id, pid, state, cmd);
}
