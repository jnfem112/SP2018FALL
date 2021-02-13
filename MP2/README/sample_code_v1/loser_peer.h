#include <limits.h>

/* This header provides example code to handle configurations.
 * The config struct stores whole socket path instead of names.
 * You are free to modifiy it to fit your needs.
 */

struct configuration
{
    int n_peers;
    char *host_socket;
    char *peer_sockets[];
};

/* Initialize the config struct from path */
int init_config(struct configuration *conf, char *path);

/* Destroy the config struct */
void destroy_config(struct configuration *conf);
