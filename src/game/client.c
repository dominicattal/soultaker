#include "../game.h"

Client* client_create(void)
{
    Client* client = st_calloc(1, sizeof(Client));
    client->uid = 0;
    return client;
}

void client_set_username(Client* client, char* username)
{
    string_free(client->username);
    client->username = username;
}

void client_destroy(Client* client)
{
    game_free_uid(client->uid);
    string_free(client->username);
    st_free(client);
    
}
