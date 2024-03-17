#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <cstring>

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

    close(pipe1[0]); // Close read end of pipe1
    close(pipe2[1]); // Close write end of pipe2
    readFromFile(input_filename, pipe1[1]);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        std::cerr << "Error: Failed to open output file for writing.\n";
        return EXIT_FAILURE;
    }
    
    while ((bytes_read = read(pipe2[0], buffer, BUFFER_SIZE)) <= 0) {
        sleep(1);
    }

    if (bytes_read > 0) {
        write(output_fd, buffer, bytes_read);
    }

    close(output_fd);
    close(pipe2[0]);
    return EXIT_SUCCESS;
    
}
