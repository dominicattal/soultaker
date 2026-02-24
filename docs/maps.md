# Maps

Maps contain rooms that are stitched together at runtime. There are several types a map room can be: an enemy room, a treasure room, an event room, a boss room, etc. These are described in the `map` config file. Each map has a corresponding image file that stores all of its rooms. Each component in the image file describes the shape that it will be, where the colors in the image file translates to tiles in the game. These are described for the entire map in the `palette` field. The image file is read in as 3 channels, so the alpha value is ignored. There are 3 reserved colors:

- white (`0xFFFFFF`) is empty space
- gray (`0x808080`) is a male connector
- black (`0x000000`) is a female connector

When trying to build a map, male connectors will try to find a valid female connector to connect with. A female connector is valid if it satifies two criteria:

- that room does not overlap with any already placed tiles
- that room is the correct room type

If a valid connector is not found, it will backtrack and try a different combination of components. If no combination of components results in a valid map, map generation will fail.

All of the game objects live inside of a map. When a map is loaded into memory, so are all of its objects. Likewise, when a map is unloaded from memory, so are all of its objects.

Every game object created is a part of a map. They are exposed with functions that look like `map_create_entity` and `room_create_entity` in the api. If the object was created using a function like `map_create_entity`, then the object will not be associated with a room. Since each room has its own data, objects created with this function will be unable to access any room's data. Information about the map can still be queried using `map_get_data`. If the object was created using a function like `room_create_entity`, then the object will store both the map and room it was created in. When inside of an object's callback function (`create`, `update`, etc), the map will use the object's room if functions like `room_create_entity` are called in the callback. If the object was made using a function like `map_create_entity`, the function will do nothing, return NULL, and produce a warning. 

It is important to remember functions like `map_create_entity` and `room_create_entity` can fail and return NULL. This happens when a function like `room_create_entity` is called when the object doesn't know its room or if the map is cleaning up and doesn't create any new objects. This is only a concern if you don't know if the object knows its room or if you're calling `create` from a `destroy` function.

## JSON Format

```
"map_name" {
    "path": "assets/maps/map_name.png",
    "generate": "map_name_generate",
    "cleanup": "map_name_cleanup",
    "branch": "map_name_branch",
    "init": "map_name_init",
    "cleanup": "map_name_cleanup",
    "palette": {
        "clear_tile": {
            "color": "000000",
            "type": "tile",
            "tex": "clear_tile_tex"
        },
        "tile_name": {
            "color": "FF0000",
            "type": "tile",
            "tex": "tile_tex",
            "create": "tile_create",
            "collide": "tile_collide"
        },
        "wall_name": {
            "color": "00FF00",
            "type": "wall",
            "side_tex": "wall_side_tex",
            "top_tex": "wall_top_tex",
            "height": 3.0
        }
    },
    "rooms" {
        "spawn_room": {
            "bounding_box": [u1, v1, u2, v2],
            "create": "map_name_spawn_room_create",
            "enter": "map_name_spawn_room_enter",
            "exit": "map_name_spawn_room_exit",
            "type": "spawn",
            "alternates": [
                {
                    "type": "female",
                    "default": "tile_name",
                    "location": [u, v],
                    "bounding_box": [u1, v1, u2, v2],
                    "origin": [u, v]
                },
                {
                    "type": "male",
                    "location": [u, v],
                    "bounding_box": [u1, v1, u2, v2],
                    "origin": [u, v]
                }
            ]
        },
        "enemy_room_1": {
            "location": [x, y, w, h],
            "create": "map_name_enemy_room_1_create",
            "type": "enemy"
        },
        "enemy_room_2": {
            "location": [x, y, w, h],
            "create": "map_name_enemy_room_2_create",
            "type": "enemy"
        },
        "boss_room": {
            "location": [x, y, w, h],
            "create": "map_name_boss_room_create",
            "type": "boss"
        },
        "mini_boss_room" {
            "location": [x, y, w, h],
            "create": "map_name_mini_boss_room_create",
            "type": "mini_boss"
        }
    }
}
```
- The `path` field contains the path to the image file used for loading the rooms.
- The `init` field is a function name that is called before generating the map.
- The `cleanup` field is a function name that is called after generating the map.
- The `generate` field is a function name that determines whether a room should be loaded or not. You can use the data in the `LocalMapGenerationSettings` struct and the data in your own struct passed from `init` to make this decision.

- The `palette` field contains zero or more colors (formatted as shown above) that correspond to a unit on the map, where a unit is a tile or a wall. Tiles require the `tex` field and walls require the `side_tex` and `top_tex` fields, where the values in these fields are texture names. The `collide` field is optional and contains a collision function for that tile. The `create` field is optional and contains a create function for when the tile is first made, which is how you set flags for the tile. The `height` specifies the height if the tile is a wall; otherwise, this value will be ignored. When `color` is "000000", that tile will replace every unfilled tile with that tile.

- The `rooms` field contains one or more rooms with the room name as a key
    - The `create` field described in each room are function names that are called when that room is loaded in, which is used to populate rooms with enemies, obstacles, or more walls.
    - The `enter` field is the name of the function to call when a player enters the room.
    - The `exit` field is the name of the function to call when a player exits the room.
    - The `bounding_box` field described in each room are UV coordinates that encompasses the room in the image file described in the "path" field.
    - The `type` field described in each room specifies how the generation should treat that room. For example, the generation algorithm might not put 3 enemy rooms next to each other, or it might not make more than one treasure room in a whole map, or it might make the boss room 10 rooms away from the spawn room. Each map requires a spawn room, but every other room is optional. In this way, the room's type is more like a suggestion for the generation algorithm; there is nothing stopping a spawn room from containing a boss or some treasure. There may be only one spawn room, and the player spawn point is the location of the female alternate.

    - The `alternates` field are the location in UV coordinates of the alternates. Map generation will fail if these are inaccurate.
        - `type` is either `male` or `female`
        - `default` is the tile (specified in `palette`) that should be placed if this alternate isn't used
        - `location` is the location of the alternate in the image file
        - `bounding_box` is the area in the image file that represents the alternate
        - `origin` is the pivot point of the bounding box. If the `bounding_box` is 3x3 and the `origin` is the UV coordinate in the center of the `bounding_box`, then the center of the `bounding_box` will be equivalent to the alternate's position
