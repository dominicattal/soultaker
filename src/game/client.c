#include "../game.h"

Client* client_create(void)
{
    Client* client = st_calloc(1, sizeof(Client));
    client->uid = game_map_uid(client, GAME_OBJ_CLIENT);
    camera_set_defaults(&client->camera);
    return client;
}

void client_update(Client* client, f32 dt)
{
    camera_update(&client->camera, dt);
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
