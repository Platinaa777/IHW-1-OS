#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

#define BUFFER_SIZE 5000

void readFromFile(const char* filename, int pipe_fd) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        std::cerr << "Error: Failed to open file for reading.\n";
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

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <n>\n";
        return EXIT_FAILURE;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int n = std::stoi(argv[3]);

    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        std::cerr << "Error: Pipe creation failed.\n";
        return EXIT_FAILURE;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        std::cerr << "Error: Fork failed.\n";
        return EXIT_FAILURE;
    } else if (pid1 == 0) {
        // Child process 1
        close(pipe1[0]); // Close read end of pipe1
        readFromFile(input_filename, pipe1[1]);
        return EXIT_SUCCESS;
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        std::cerr << "Error: Fork failed.\n";
        return EXIT_FAILURE;
    } else if (pid2 == 0) {
        // Child process 2
        close(pipe1[1]); // Close write end of pipe1
        countSequence(pipe1[0], pipe2[1], n);
        return EXIT_SUCCESS;
    }

    close(pipe1[0]); // Close read end of pipe1
    close(pipe1[1]); // Close write end of pipe1
    close(pipe2[1]); // Close write end of pipe2

    int output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        std::cerr << "Error: Failed to open output file for writing.\n";
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(pipe2[0], buffer, BUFFER_SIZE)) > 0) {
        write(output_fd, buffer, bytes_read);
    }

    close(output_fd);
    close(pipe2[0]);

    // Wait for child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return EXIT_SUCCESS;
}
