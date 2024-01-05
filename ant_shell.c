/*
  ****************************************
  ATOM_SHELL short for atom shell
  ************************************

  -I have used '&&' to add two commands in one input line
  -example:
    ls > file && cat < file
            OR
    ls && ls -l

  -I have used built in functions like cd, help, exit, and umask
  -Still working on history function 
  
 
  -to run use the makefile i have just type	
		'make all && make run'
		and you should be in business!
    
	
  -Might add more stuff later on!
  -Enjoy! ;)
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define atom_RL_BUFFSIZE 1024
#define atom_TOK_BUFSIZE 64
#define atom_TOK_DELIM " \t\r\n\a"


// to add built in linux commands with c

// step 1
int atom_cd(char **args);
int atom_help(char **args);
int atom_exit(char **args);
int atom_history(char **args);
int atom_umask(char **args);

// step 2
char *builtin_str[] = { "cd", "help", "exit", "history", "umask"};

// step 3
int (*builtin_func[]) (char **) = {&atom_cd, &atom_help, &atom_exit, &atom_history, &atom_umask};


int atom_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

// look back at umask
int atom_umask(char **args) {

  int flag = 0;

  if(args[1] == NULL) {
    system("umask");
    
  } else {
    for(int i =0; i < sizeof(args[1]); i++) {
      if(args[1] != NULL) {
        if(args[1][i] == '8') {
          flag = 1;
        } else if(args[1][i] == '9') {
          flag = 1;
        }
      }
    }
    
    if(flag == 1) {
      printf("atom_shell: octal out of length\n");
    } else {
      mode_t um = strtol(args[1], NULL, 8);
      umask(um);
    }
  }

  
  return 1;
}

int atom_cd(char **args) {
  if(args[1] == NULL) {
    fprintf(stderr, "%s\n", "atom_shell: expected argument to \"cd\"\n");
  } else {
    if(chdir(args[1]) != 0) {
      perror("atom");
    }
  }
  return 1;
}

int atom_history(char **args) {
  system("history");
  return 1;
}

int atom_help(char **args) {
    printf("%s\n", "Atom's atom shell help:");
    printf("%s\n", "----------------------------");
    printf("%s\n", "Type program names and arguments, and hit enter!");
    printf("%s\n", "The following are built in:");

    for(int i = 0; i < atom_num_builtins(); i++) {
      printf("  %s\n", builtin_str[i]);
    }

    printf("%s\n", "Use the man command for information on other programs.\n");
    return 1;
}

int atom_exit(char ** args) {
  fflush(stdout);
  printf("%s\n", "Good Bye!");
  exit(0);
  return 0;
}

int atom_launch(char **args) {
  int pipefd[2];
  pid_t pid;
  int status, out, in;

  if(args[1] == NULL) {
    // if we have only one
    if(pipe(pipefd) == -1) {
      perror("atom_shell: pipe error");
      exit(EXIT_FAILURE);
    }

    pid = fork();

    if(pid == 0) {
      close(pipefd[1]);
      // the child
      if(execvp(args[0], args) == -1) {
        perror("atom");
      }

      close(pipefd[0]);
      exit(EXIT_FAILURE);
    } else if(pid < 0) {
      perror("atom");
      exit(EXIT_FAILURE);
    } else {
      // the parent
      close(pipefd[0]);
      if(((pid) = waitpid(pid, &status, 0)) < 0 ) {
        printf("%s\n", "atom_shell: wait pid error.");
      }
      close(pipefd[1]);
    }

  } else { // more than one input
     if(strncmp(args[1], ">", 1) == 0) {
        if(args[2] == NULL) {
          printf("%s\n", "usage: #->> cmd > file-name" );
        } else {
          if(pipe(pipefd) == -1) {
            perror("atom_shell: pipe error");
            exit(EXIT_FAILURE);
          }
          pid = fork();

          if(pid == 0) {
            close(pipefd[1]);

            out = open(args[2], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

            dup2(out, 1);
            char * ls_args[] = {args[0], NULL};

            if(execvp(ls_args[0], ls_args) == -1) {
              perror("atom");
            }
            close(pipefd[0]);
            exit(EXIT_FAILURE);
          } else if(pid < 0) {
            perror("atom");
            exit(EXIT_FAILURE);
          } else {

            close(pipefd[0]);
            if(((pid) = waitpid(pid, &status, 0)) < 0) {
              printf("%s\n", "atom_shell: wait pid error.");
            }
            close(pipefd[1]);
          }
        }
    } else if(strncmp(args[1], "<", 1) == 0) {
      if(args[2] == NULL) {
        printf("%s\n", "usage: #->> cmd > file-name" );
      } else {
        if(pipe(pipefd) == -1) {
          perror("atom_shell: pipe error");
          exit(EXIT_FAILURE);
        }
        pid = fork();

        if(pid == 0) {
          close(pipefd[1]);

          in = open(args[2], O_RDONLY);
          if(in < 0) {
            perror("atom_shell:");
            exit(EXIT_FAILURE);
          }
          dup2(in, 0);
          char * ls_args[] = {args[0], NULL};

          if(execvp(ls_args[0], ls_args) == -1) {
            perror("atom");
          }
          close(pipefd[0]);
          exit(EXIT_FAILURE);
        } else if(pid < 0) {
          perror("atom");
          exit(EXIT_FAILURE);
        } else {

          close(pipefd[0]);
          if(((pid) = waitpid(pid, &status, 0)) < 0) {
            printf("%s\n", "atom_shell: wait pid error.");
          }
          close(pipefd[1]);
        }
      }
    } else {
        // its not > nor < its something like ls -l or rm file-name or etc..
        if(pipe(pipefd) == -1) {
          perror("atom_shell: pipe error");
          exit(EXIT_FAILURE);
        }

        pid = fork();

        if(pid == 0) {
          close(pipefd[1]);
          // the child
          if(execvp(args[0], args) == -1) {
            perror("atom");
          }

          close(pipefd[0]);
          exit(EXIT_FAILURE);
        } else if(pid < 0) {
          perror("atom");
          exit(EXIT_FAILURE);
        } else {
          // the parent
          close(pipefd[0]);
          if(((pid) = waitpid(pid, &status, 0)) < 0 ) {
            printf("%s\n", "atom_shell: wait pid error.");
          }
          close(pipefd[1]);
        }
    }
  }

  return -1;
}

char **atom_split_line(char *line) {
  int bufsize = atom_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if(!tokens) {
    fprintf(stderr, "%s\n", "atom_shell: allocation error.");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, atom_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if(position >= bufsize) {
      bufsize += atom_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if(!tokens) {
        fprintf(stderr, "%s\n", "atom_shell: allocation error.");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, atom_TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}

char *atom_read_line(void) {
  char *line = NULL;
  size_t buffsize = 0;
  getline(&line, &buffsize, stdin);
  return line;
}

int atom_execute(char **args) {
  
  int flag = 0;
  int at_index = -1;

  if(args[0] == NULL) {
    return 1;
  }
  // so if user enter cd, exit, or help, for loop catches this commands and executes them.
  // else goes to atom_launch
  for (int i = 0; i < atom_num_builtins(); i++) {
    if(strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  for(int i =0; i < sizeof(args); i++) {
    if(args[i] != NULL) {
      if(strncmp(args[i], "&&", 1) == 0) {
        flag = 1;
        at_index = i;
        break;
      }
    }
  }
  

  if(flag == 1) {
      char **new_args = malloc(sizeof(char) * 1024);
      char **new_args2 = malloc(sizeof(char) * 1024);
      int a = 0;
      int b = 0;

      while (a < at_index) {
        new_args[a] = args[a];
        a++;
      }

      b = at_index;

      while (b < sizeof(args)) {
        new_args2[b-at_index-1] = args[b];
        b++;
      }

      a = atom_launch(new_args);
      b = atom_launch(new_args2);

      // free(new_args2);
      // free(new_args);

      if (a < 0) {
        return a;
      } else if (b < 0) {
        return b;
      }
  }


  return atom_launch(args);
}

void atom_loop(void) {
  char *line;
  char **args;
  int status;
  // loops while status is in check
  do{
    printf("#->> "); // or $,% ^
    line = atom_read_line();
    args = atom_split_line(line);
    status = atom_execute(args);

    free(line);
    free(args);
  } while(status);
}

void atom_sig_handler(int sig_num) {
  signal(SIGINT, atom_sig_handler);
  printf("\n%s\n", "atom_shell: Caught Signal _ to exit type 'exit'! ;)");
  atom_loop();
}

int main(int argc, char ** argv) {

  signal(SIGINT, atom_sig_handler);
  atom_loop();

  return EXIT_SUCCESS;
}
