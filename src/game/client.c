#include "../game.h"

static u32 current_uid;

Client* client_create(void)
{
    Client* client = st_calloc(1, sizeof(Client));
    client->uid = __atomic_fetch_add(&current_uid, 1, __ATOMIC_SEQ_CST);
    return client;
}

void client_set_username(Client* client, char* username)
{
    string_free(client->username);
    client->username = username;
}

void client_destroy(Client* client)
{
    string_free(client->username);
    st_free(client);
}
