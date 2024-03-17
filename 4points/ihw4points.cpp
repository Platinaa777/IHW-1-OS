#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#define BUFFER_SIZE 5000

void readFromFile(const char* filename, int pipe_fd) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        fprintf(stderr, "Error: Failed to open file for reading.\n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        write(pipe_fd, buffer, bytes_read);
    }

    close(file_fd);
    close(pipe_fd);
}

void countSequence(int pipe_read_fd, int pipe_write_fd, int n) {
    char buffer[BUFFER_SIZE];
    int left_ptr = 0;
    int right_ptr = 0;
    int len = 1;

    int bytes_read = read(pipe_read_fd, buffer, BUFFER_SIZE);
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

    close(pipe_read_fd);
    if (right_ptr - left_ptr + 1 == n) {
        char sequence[right_ptr - left_ptr + 1] = {0};
        for (int i = 0; i < len; ++i) {
            sequence[i] = buffer[left_ptr + i];
        }

        write(pipe_write_fd, sequence, sizeof(sequence));
    } else {
        write(pipe_write_fd, "", 0);
    }
    close(pipe_write_fd);
}

void writeToOutputFile(const char* filename, int pipe_fd) {
    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        fprintf(stderr, "Error: Failed to open file for writing.\n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd, buffer, BUFFER_SIZE)) > 0) {
        write(file_fd, buffer, bytes_read);
    }

    close(file_fd);
    close(pipe_fd);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int n = std::stoi(argv[3]);

    int pipe1[2], pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        fprintf(stderr, "Error: Pipe creation failed.\n");
        return EXIT_FAILURE;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        fprintf(stderr, "Error: Fork failed.\n");
        return EXIT_FAILURE;
    } else if (pid1 == 0) {
        // read from file
        close(pipe1[0]);
        readFromFile(input_filename, pipe1[1]);
        return EXIT_SUCCESS;
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        fprintf(stderr, "Error: Fork failed.\n");
        return EXIT_FAILURE;
    } else if (pid2 == 0) {
        // Child process 2
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2
        countSequence(pipe1[0], pipe2[1], n);
        return EXIT_SUCCESS;
    }

    pid_t pid3 = fork();
    if (pid3 == -1) {
        fprintf(stderr, "Error: Fork failed.\n");
        return EXIT_FAILURE;
    } else if (pid3 == 0) {
        // Child process 3
        close(pipe2[1]); // Close write end of pipe2
        writeToOutputFile(output_filename, pipe2[0]);
        return EXIT_SUCCESS;
    }

    close(pipe1[0]); // Close read end of pipe1
    close(pipe1[1]); // Close write end of pipe1
    close(pipe2[0]); // Close read end of pipe2
    close(pipe2[1]); // Close write end of pipe2

    // Wait for all child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return EXIT_SUCCESS;
}