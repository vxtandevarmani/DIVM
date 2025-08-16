#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return 1; // handle error

    const char *text = "Hello, Linux syscall!\n";
    write(fd, text, strlen(text));

    close(fd);
    return 0;
}
