#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "loser_peer.h"


int init_config(struct configuration *conf, char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    /* TODO parse the config file */

    close(fd);
    return 0;
}


void destroy_config(struct configuration *conf)
{
    /* TODO do something like free() */
}


int main(int argc, char *argv[])
{
    /* Make sure argv has a config path */
    assert(argc == 2);
    int ret;

    /* Load config file */
    struct configuration config;
    ret = init_config(&config, argv[1]);
    if (ret < 0)
        exit(EXIT_FAILURE);

    assert(config.host_socket != NULL);
    assert(config.peer_sockets != NULL);
    assert(config.n_peers >= 0);

    /* Create host UNIX socket */
    struct sockaddr_un sock_addr;
    int connection_socket;

    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket < 0)
        exit(EXIT_FAILURE);

    memset(&sock_addr, 0, sizeof(struct sockaddr_un));

    /* Bind socket to a name. */
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, config.host_socket, sizeof(sock_addr.sun_path) - 1);

    ret = bind(connection_socket,
               (const struct sockaddr *) &sock_addr,
               sizeof(struct sockaddr_un)
        );
    if (ret == -1)
        exit(EXIT_FAILURE);

    /* Prepare for accepting connections */
    ret = listen(connection_socket, 20);
    if (ret == -1)
        exit(EXIT_FAILURE);


    /* Enter the serving loop.
     * It calls select() to check if any file descriptor is ready.
     * You may look up the manpage select(2) for details.
     */

    int max_fd = sysconf(_SC_OPEN_MAX);

    fd_set read_set;
    fd_set write_set;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);

    FD_SET(STDIN_FILENO, &read_set);       /* check for user input */
    FD_SET(connection_socket, &read_set);  /* check for new peer connections */

    while (1)
    {
        struct timeval tv;
        fd_set working_read_set, working_write_set;

        memcpy(&working_read_set, &read_set, sizeof(working_read_set));
        memcpy(&working_write_set, &write_set, sizeof(working_write_set));

        ret = select(max_fd, &working_read_set, &working_write_set, NULL, &tv);

        if (ret < 0)             /* We assume it doesn't happen. */
            exit(EXIT_FAILURE);

        if (ret == 0)            /* No fd is ready */
            continue;

        if (FD_ISSET(STDIN_FILENO, &working_read_set))
        {
            /* TODO Handle user commands */
        }

        if (FD_ISSET(connection_socket, &working_read_set))
        {
            int peer_socket = accept(connection_socket, NULL, NULL);
            if (peer_socket < 0)
                exit(EXIT_FAILURE);

            /* TODO Store the peer fd */
        }
    }

    /* finalize */
    destroy_config(&config);
    close(connection_socket);
    return 0;
}
