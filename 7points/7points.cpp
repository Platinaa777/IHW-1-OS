#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <iostream>

#define BUFFER_SIZE 5000
#define FAIL 1
#define SUCCESS 0

const char *PIPE1 = "pipe1";
const char *PIPE2 = "pipe2";

char* countSequence(char *buffer, ssize_t bytes_read, int n) {
    int left_ptr = 0;
    int right_ptr = 0;
    int len = 1;

    for (int i = 1; i < bytes_read; ++i) {
        if (buffer[i - 1] > buffer[i]) {
            len++;
        } else {
            len = 1;
        }
        
        if (len == n) {
            left_ptr = i - len + 1;
            right_ptr = i;
            break;
        }
    }

    if (right_ptr - left_ptr + 1 == n) {
        char* sequence = new char[n];

        for (int i = 0; i < len; ++i) {
            sequence[i] = buffer[left_ptr + i];
        }

        return sequence;
    }

    char* empty = new char[n];

    for (int i = 0; i < n; ++i) {
        empty[i] = ' ';
    }

    return empty;
}

void DoReadAndWrite(const char *input_file_name, const char *output_file_name) {
    int fd1 = open(PIPE1, O_WRONLY);
    if (fd1 < 0) {
        fprintf(stderr, "Can't open pipe");
        exit(FAIL);
    }

    int fd2 = open(PIPE2, O_RDONLY);
    if (fd1 < 0) {
        fprintf(stderr, "Can't open pipe");
        exit(FAIL);
    }

    char buffer[BUFFER_SIZE];

    int fd_input_file = open(input_file_name, O_RDONLY);
    if (fd_input_file < 0) {
        fprintf(stderr, "Can't open input file");
        exit(FAIL);
    }

    int size_r = read(fd_input_file, buffer, BUFFER_SIZE);
    if (size_r < 0) {
        fprintf(stderr, "Can't read from file");
        exit(FAIL);
    }

    int size_w = write(fd1, buffer, size_r);
    if (size_w < 0) {
        fprintf(stderr, "pipe for writing is prohibited");
        exit(FAIL);
    }

    int pipe_size = read(fd2, buffer, size_w);
    if (pipe_size < 0) {
        fprintf(stderr, "error with reading from pipe\n");
        exit(FAIL);
    }

    int fd_out = open(output_file_name, O_WRONLY);
    if (fd_out < 0) {
        fprintf(stderr, "error with output file\n");
        exit(FAIL);
    }

    int size_w2 = write(fd_out, buffer, pipe_size);
    if (size_w2 < 0) {
        fprintf(stderr, "error with writing to output file\n");
        exit(FAIL);
    }

    if (close(fd1) < 0 || close(fd2)) {
        printf("error with closing pipes\n");
        exit(FAIL);
    }

    exit(SUCCESS);
}

void DoMainWork(int n) {
    int fd1 = open(PIPE1, O_RDONLY);
    if (fd1 < 0) {
        fprintf(stderr, "error with opening pipe to read.\n");
        exit(FAIL);
    }

    int fd2 = open(PIPE2, O_WRONLY);
    if (fd2 < 0) {
        fprintf(stderr, "error with opening pipe to write.\n");
        exit(FAIL);
    }

    char buffer[BUFFER_SIZE];

    int size_r = read(fd1, buffer, BUFFER_SIZE);
    if(size_r < 0) {
        fprintf(stderr, "error with reading from pipe.\n");
        exit(FAIL);
    }
 
    auto sequence = countSequence(buffer, size_r, n);

    int size_w = write(fd2, sequence, n);
    if (size_w < 0) {
        fprintf(stderr, "error with writing to pipe.\n");
        exit(FAIL);
    }

    if (close(fd1) < 0 || close(fd2) < 0) {
        printf("error with closing pipes\n");
        exit(FAIL);
    }

    exit(SUCCESS);
}


int main(const int argc, const char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <n>\n", argv[0]);
        exit(FAIL);
    }

    mknod(PIPE1, S_IFIFO | 0666, 0);
    mknod(PIPE2, S_IFIFO | 0666, 0);
    int n = std::stoi(argv[3]);

    int pid_2 = fork();
    if (pid_2 < 0) {
        fprintf(stderr, "2 pid was not started\n");
        exit(FAIL);
    }

    if (pid_2 != 0) {
        DoReadAndWrite(argv[1], argv[2]);
    }
    
    DoMainWork(n);
}