#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define SOCKET_PATH "/var/run/dpdk/tmp/dpdk_telemetry.v2:"TOSTRING(SHRT_MAX)
#define SOCKET_PATH_NEW "/var/run/dpdk/rte/dpdk_telemetry.v2"
#define BUFFER_SIZE 1024
#define PIPE_READ 0
#define PIPE_WRITE 1

pid_t pid;

void create_socket(const char *path, int pipe_fd) {
    int sockfd, client_sockfd;
    struct sockaddr_un addr;
    char response[256];

    sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    unlink(path);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 2) == -1) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Notify parent process that server is ready
    printf("Server listening on %s\n", path);
    write(pipe_fd, "READY", 5);

    while (1) {
        // Accept incoming connection
        client_sockfd = accept(sockfd, NULL, NULL);
        if (client_sockfd == -1) {
            perror("accept");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Construct the JSON response
        snprintf(response, sizeof(response), "{\"pid\": %d}", pid);

        // Send the JSON response
        if (send(client_sockfd, response, strlen(response), 0) == -1) {
            perror("send");
            close(client_sockfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Close the client socket
        close(client_sockfd);
    }

    close(sockfd);
}

ssize_t read_from_socket(const char *socket_path, char *buffer, size_t buffer_size) {
    int sockfd;
    struct sockaddr_un addr;
    ssize_t num_read;

    sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) != 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    num_read = read(sockfd, buffer, buffer_size - 1);
    if (num_read == -1) {
        perror("read");
        close(sockfd);
        return -1;
    }

    buffer[num_read] = '\0';

    close(sockfd);

    return num_read;
}

int create_link(char *path)
{
	int sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (sockfd < 0) {
		printf("Error with socket creation, %s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
	printf("Attempting socket bind to path '%s'\n", path);

	if (link(SOCKET_PATH, path) < 0) {
		printf("Initial create link to socket '%s' failed.\n", path);

		/* check if current socket is active */
		if (connect(sockfd, (void *)&addr, sizeof(addr)) == 0) {
			close(sockfd);
			return -EADDRINUSE;
		}

		/* socket is not active, delete and attempt create link */
		printf("Attempting unlink and create link\n");
		unlink(path);
		if (link(SOCKET_PATH, path) < 0) {
			printf("Error create socket link: %s\n", strerror(errno));
			close(sockfd);
			return -errno; /* if unlink failed, this will be -EADDRINUSE as above */
		}
	}

	if (connect(sockfd, (void *)&addr, sizeof(addr)) != 0) {
        printf("Error calling listen for socket: %s\n", strerror(errno));
        unlink(path);
        close(sockfd);
        return -EADDRINUSE;
	}
	printf("Socket link creation ok\n");

	return sockfd;
}

int find_and_create_link() {
    short suffix = 0;
    char *path = SOCKET_PATH_NEW;

	int ret = create_link(path);
	while (ret < 0) {
		if (suffix < 0) {
			printf("suffix max %d\n", suffix);
			return -1;
		}
		if (snprintf(path, sizeof(path), "%s:%d",
				SOCKET_PATH_NEW, ++suffix) >= (int)sizeof(path)) {
			printf("Error with socket binding, path too long\n");
			return -1;
		}
		ret = create_link(path);
	}

    return ret;
}

int main() {
    unlink(SOCKET_PATH);
    unlink(SOCKET_PATH_NEW);
    int pipefd[2];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    pid = getpid();

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Start the server in a separate process or thread
    if (fork() == 0) {
        // Child process runs the server
        close(pipefd[PIPE_READ]); // Close unused read end
        create_socket(SOCKET_PATH, pipefd[PIPE_WRITE]);
        exit(EXIT_SUCCESS); // Ensure child process exits after starting server
    }

    // Parent process
    close(pipefd[PIPE_WRITE]); // Close unused write end

    // Give the server some time to start
    char pipe_buffer[10];
    if (read(pipefd[PIPE_READ], pipe_buffer, sizeof(pipe_buffer)) > 0) {
        if (strcmp(pipe_buffer, "READY") == 0) {
            printf("Server is ready\n");
        } else {
            fprintf(stderr, "Unexpected message from server: %s\n", pipe_buffer);
        }
    } else {
        fprintf(stderr, "Failed to read readiness signal from server\n");
    }

    bytes_read = read_from_socket(SOCKET_PATH, buffer, sizeof(buffer));
    if (bytes_read >= 0) {
        printf("Received: %s\n", buffer);
    } else {
        fprintf(stderr, "Failed to read from the socket %s\n", SOCKET_PATH);
    }

    int ret = find_and_create_link();

    bytes_read = read_from_socket(SOCKET_PATH_NEW, buffer, sizeof(buffer));
    if (bytes_read >= 0) {
        printf("Received: %s\n", buffer);
    } else {
        fprintf(stderr, "Failed to read from the socket %s\n", SOCKET_PATH_NEW);
    }

    close(pipefd[PIPE_READ]);

    return 0;
}
