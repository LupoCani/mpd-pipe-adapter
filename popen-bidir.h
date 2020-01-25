/*
Simple reimplementation of popen() that allows bidirectional commication using
several pipes.
*/

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/types.h>
#include <signal.h>

typedef struct pipe_bd_s{
	pid_t pid;
	int in;
	int out;
	int err;
} pipe_bd;

void pclose2(pipe_bd handle)
{
	close(handle.in);
	close(handle.out);
	
	if (handle.err > 0)
		close(handle.err);
}

pipe_bd popen2(char *argv[], int route_stderr)
{
	int pipe_detect_fail[2];
	int ch_stdin[2], ch_stdout[2], ch_stderr[2];
	
	pipe(pipe_detect_fail);
	fcntl(pipe_detect_fail[1], F_SETFD, FD_CLOEXEC);
	
	pipe(ch_stdin);
	pipe(ch_stdout);
	if (route_stderr)
		pipe(ch_stderr);
	else
		ch_stderr[0] = -1;
	
	
	pipe_bd handle;
	char one_byte = 0;
	
	handle.in = ch_stdin[1];
	handle.out = ch_stdout[0];
	handle.err = ch_stderr[0];
	
	handle.pid = fork();
	
	if (handle.pid != 0) {
		close(ch_stdin[0]);
		close(ch_stdout[1]);
		if (route_stderr)
			close(ch_stderr[1]);
		
		close(pipe_detect_fail[1]);
		
		if (handle.pid < 0)
			pclose2(handle);
		else if (read(pipe_detect_fail[0], &one_byte, 1) != 0) {
			pclose2(handle);
			handle.pid = -1;
		}
		
		close(pipe_detect_fail[0]);
		return handle;
	}
	close(ch_stdin[1]);
	close(ch_stdout[0]);
	
	dup2(ch_stdin[0],  STDIN_FILENO);
	dup2(ch_stdout[1], STDOUT_FILENO);
	if (route_stderr)
		dup2(ch_stderr[1], STDERR_FILENO);
		
	execv(argv[0], argv);
	
	write(pipe_detect_fail[1], &one_byte, 1);
	close(pipe_detect_fail[1]);
}
