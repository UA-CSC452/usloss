#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <termios.h>
#include <sgtty.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

char termname[1000];
char infile[1000];
int ttyno;
void reset(int sig);
void ignore(int sig);
int kidpid;
FILE *dummy;
struct termios bar;
struct termios foo;
int fd;
void echo_out(void);

int
main(argc, argv)
  char argc;
  char **argv;
{
  char c, cr = '\r',n;
  if (argc != 2) {
    fprintf(stderr, "Usage: pterm terminal\n");
    exit(1);
  }
  sscanf(argv[1], "%d", &ttyno);
  sprintf(infile, "term%d.out", ttyno);
  signal(SIGINT, reset);
  signal(SIGTSTP, reset);
  sprintf(termname, "term%d.in", ttyno);
  if((dummy = fopen(termname, "r")) != NULL)
  {
    fclose(dummy);
    printf("overwrite %s ?", termname);
    fflush(stdout);
    if(getchar() != 'y') exit(0);
    unlink(termname);
  }
  fd = open(termname, O_CREAT + O_WRONLY, 00770);
  #ifdef NOTDEF
  ioctl(0, TCGETA, &foo);
  ioctl(0, TCGETA, &bar);
  #endif

  tcgetattr(0, &foo);
  tcgetattr(0, &bar);
 
  /* We may need this later
   * foo.sg_flags = CBREAK;
   * foo.c_iflag |= ICRNL | IXON;
   * foo.c_oflag = 0;
   * foo.c_lflag = ISIG; 
   * foo.c_iflag = 0;
   * foo.c_lflag &= ~ICANON;
   * foo.c_iflag |= BRKINT; */

#ifdef NOTDEF
  ioctl(0, TCSETA, &foo);
#endif
  tcsetattr(0, TCSANOW, &foo);
  printf("hit break (^C) to exit\r\n");
  echo_out();
  for(;;)
  {
    read(1, &c, 1);
    if(c == cr) 
      n = '\n';
    else 
      n = c;
    if(n == '\n') 
      write(1, &cr, 1);
    //write(1, &n, 1);
    write(fd, &n, 1);
  }
  return 0;
}

void reset(int sig)
{
  close(fd);
  kill(kidpid, 9);
#ifdef NOTDEF
  ioctl(0, TCSETA, &bar);
#endif
  tcsetattr(0, TCSANOW, &bar);
  printf("\ntty reset\n");
  exit(0);
}

void echo_out(void)
{
  int inp;
  char c;
  char cr = '\r';

  if((kidpid = fork()) != 0) 
    return;
  signal(SIGINT, ignore);
  signal(SIGTSTP, ignore);
  do
  {
    inp = open(infile, O_RDONLY);
  }
  while(inp < 1);
  for(;;)
  {
    while(read(inp, &c, 1) == 0);
    if(c == '\n')
      write(1, &cr, 1);
    write(1, &c, 1);
  }
}

void
ignore(int sig)
{
}
