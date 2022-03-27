#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main()
{
    //daemon rules

    unsigned int pid1;
    int fd0, fd1, fd2;

     if (chdir("/etc") != 0)
     {
         exit(0);
     }
    if ((pid1 = fork()) != 0)
        exit(0);
    if (setsid() < 0)
        exit(0);
    umask(0);

    close(0);
    close(2);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = open("/dev/null", O_RDWR);
    fd2 = open("/dev/null", O_RDWR);

    //getline
    char *cmd;
    char cmds[100][100];
    int min[100];
    int hr[100];
    size_t size = 100;
    cmd = (char *)malloc(size * sizeof(char));
    char hr_str[3];
    char min_str[3];

    FILE *fp = fopen("/etc/simplecrontab", "r");
    int index = 0;
    while (1)
    {
        if (fscanf(fp, "%s %s", min_str, hr_str) == EOF)
            break;

        if (min_str[0] != '*')
        {
            min[index] = atoi(min_str);
        }
        else
        {
            min[index] = -1;
        }
        if (hr_str[0] != '*')
        {
            hr[index] = atoi(hr_str);
        }
        else
        {
            hr[index] = -1;
        }
        getline(&cmd, &size, fp);
        strcpy(cmds[index], cmd);
        index++;
    }

    int k = 1;
    int pid;
    int pids[100];
    int pidIndex = 0;
    int piTemp = 0;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    int diffmin;
    int br = 0;
    for (int j = 0; j < index; j++)
    {
        if (k != 0)
        {
            k = fork();
            if (k == 0)
            {
                char *args[] = {"bash", "-c", cmds[j], NULL};
                while (1)
                {
                    time(&t);
                    tm = localtime(&t);
                    if (hr[j] != -1 && min[j] != -1) //d d
                    {
                        diffmin = (hr[j] * 60 + min[j]) - (tm->tm_hour * 60 + tm->tm_min);
                        if (diffmin < 0)
                            diffmin += 1440;
                        sleep(diffmin * 60);

                        pid = fork();
                        if (pid == 0)
                        {
                            execv("/bin/bash", args);
                            return 0;
                        }
                        sleep(60);
                    }
                    else if (hr[j] != -1 && min[j] == -1) //* d
                    {
                        if (hr[j] == tm->tm_hour)
                            ;
                        else
                        {
                            diffmin = (hr[j] * 60) - (tm->tm_hour * 60 + tm->tm_min);
                            if (diffmin < 0)
                                diffmin += 1440;
                            sleep(diffmin * 60);
                        }

                        pid = fork();
                        if (pid == 0)
                        {
                            execv("/bin/bash", args);
                            return 0;
                        }
                        sleep(60);
                    }
                    else if (hr[j] == -1 && min[j] != -1) //d *
                    {
                        diffmin = min[j] - tm->tm_min;
                        if (diffmin < 0)
                        {
                            diffmin += 60;
                        }
                        sleep(diffmin * 60);

                        pid = fork();
                        if (pid == 0)
                        {
                            execv("/bin/bash", args);
                            return 0;
                        }
                        sleep(60);
                    }
                    else //* *
                    {
                        pid = fork();
                        if (pid == 0)
                        {
                            execv("/bin/bash", args);
                            return 0;
                        }
                        sleep(60);
                    }

                    //wait
                    int status;
                    piTemp = 0;
                    for (int i = 0; i < pidIndex; i++)
                    {
                        if (waitpid(pids[i], &status, WNOHANG) == 0)
                        {
                            pids[piTemp] = pids[i];
                            piTemp++;
                        }
                    }
                    if (waitpid(pid, &status, WNOHANG) == 0)
                    {
                        if (piTemp < 100)
                        {
                            pids[piTemp] = pid;
                            piTemp++;
                        }
                    }
                    pidIndex = piTemp;
                }
            }
        }
    }

    return 0;
}
