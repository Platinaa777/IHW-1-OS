#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <cstring>

#define BUFFER_SIZE 5000

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
        write(pipe_write_fd, "", 1); // send empty string
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

    char pipe1_name[] = "pipe1";
    char pipe2_name[] = "pipe2";

    // Create named pipes
    mknod(pipe1_name, 0666, 0);
    mknod(pipe2_name, 0666, 0);

    int pipe1[2];
    int pipe2[2];

    pipe1[0] = open(pipe1_name, O_RDONLY); // Read end of pipe1
    pipe1[1] = open(pipe1_name, O_WRONLY); // Write end of pipe1

    pipe2[0] = open(pipe2_name, O_RDONLY); // Read end of pipe2
    pipe2[1] = open(pipe2_name, O_WRONLY); // Write end of pipe2


    // Child process
    close(pipe1[1]); // Close write end of pipe1
    close(pipe2[0]); // Close read end of pipe2
    countSequence(pipe1[0], pipe2[1], n);
    return EXIT_SUCCESS;
}
