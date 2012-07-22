/*
** Copyright 2012, The CyanogenMod Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include <cutils/properties.h>
#include <cutils/log.h>

/* reads a file, making sure it is terminated with \n \0 */
char* read_file(const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
    int fd;

    data = 0;
    fd = open(fn, O_RDONLY);
    if(fd < 0) return 0;

    sz = lseek(fd, 0, SEEK_END);
    if(sz < 0) goto oops;

    if(lseek(fd, 0, SEEK_SET) != 0) goto oops;

    data = (char*) malloc(sz + 2);
    if(data == 0) goto oops;

    if(read(fd, data, sz) != sz) goto oops;
    close(fd);
    data[sz] = '\n';
    data[sz+1] = 0;
    if(_sz) *_sz = sz;
    return data;

oops:
    close(fd);
    if(data != 0) free(data);
    return 0;
}

int get_property(const char *data, char *found, const char *searchkey, const char *not_found)
{
    char *key, *value, *eol, *sol, *tmp;
    if (data == NULL) goto defval;
    int matched = 0;
    sol = strdup(data);
    while((eol = strchr(sol, '\n'))) {
        key = sol;
        *eol++ = 0;
        sol = eol;

        value = strchr(key, '=');
        if(value == 0) continue;
        *value++ = 0;

        while(isspace(*key)) key++;
        if(*key == '#') continue;
        tmp = value - 2;
        while((tmp > key) && isspace(*tmp)) *tmp-- = 0;

        while(isspace(*value)) value++;
        tmp = eol - 2;
        while((tmp > value) && isspace(*tmp)) *tmp-- = 0;

        if (strncmp(searchkey, key, strlen(searchkey)) == 0) {
            matched = 1;
            break;
        }
    }
    int len;
    if (matched) {
        len = strlen(value);
        if (len >= PROPERTY_VALUE_MAX)
            return -1;
        memcpy(found, value, len + 1);
    } else goto defval;
    return len;

defval:
    len = strlen(not_found);
    memcpy(found, not_found, len + 1);
    return len;
}

/*
 * The following is based off of bionic/libc/unistd/system.c with
 *  modifications to avoid calling /system/bin/sh -c
 */
extern char **environ;
int system_nosh(const char *command)
{
    pid_t pid;
    sig_t intsave, quitsave;
    sigset_t mask, omask;
    int pstat;
    char buffer[255];
    char *argp[32];
    char *next = buffer;
    char *tmp;
    int i = 0;

    if (!command)           /* just checking... */
        return(1);

    /*
     * The command to argp splitting is from code that was
     * reverted in Change: 11b4e9b2
     */
    if (strnlen(command, sizeof(buffer) - 1) == sizeof(buffer) - 1) {
        ALOGE("command line too long while processing: %s", command);
        errno = E2BIG;
        return -1;
    }
    strcpy(buffer, command); // Command len is already checked.
    while ((tmp = strsep(&next, " "))) {
        argp[i++] = tmp;
        if (i == 32) {
            ALOGE("argument overflow while processing: %s", command);
            errno = E2BIG;
            return -1;
        }
    }
    argp[i] = NULL;


    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &omask);
    switch (pid = vfork()) {
    case -1:                        /* error */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        return(-1);
    case 0:                                 /* child */
        sigprocmask(SIG_SETMASK, &omask, NULL);
        execve(argp[0], argp, environ);
        _exit(127);
    }

    intsave = (sig_t)  bsd_signal(SIGINT, SIG_IGN);
    quitsave = (sig_t) bsd_signal(SIGQUIT, SIG_IGN);
    pid = waitpid(pid, (int *)&pstat, 0);
    sigprocmask(SIG_SETMASK, &omask, NULL);
    (void)bsd_signal(SIGINT, intsave);
    (void)bsd_signal(SIGQUIT, quitsave);
    return (pid == -1 ? -1 : pstat);
}

