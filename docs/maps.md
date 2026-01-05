# Maps

Maps contain rooms that are stitched together at runtime.
There are several types a map room can be: an enemy room, a treasure
room, an event room, a boss room, etc. These are described in the ```map``` config
file. Each map has a corresponding image file that stores all of its rooms.
Each component in the image file describes the shape that it will be, where the
colors in the image file translates to tiles in the game. These are described for
the entire map in the ```palette``` field. The image file is read in as 3 channels, 
so the alpha value is ignored. There are 3 reserved colors:

- white (```0xFFFFFF```) is empty space
- gray (```0x808080```) is a male connector
- black (```0x000000```) is a female connector

When trying to build a map, male connectors will try to find a valid female connector
to connect with. A female connector is valid if it satifies two criteria:

- that room does not overlap with any already placed tiles
- that room is the correct room type

If a valid connector is not found, it will backtrack and try a different combination
of components. If no combination of components results in a valid map, then the
world will end.

There is no hard limit to the size of a map.

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
            "spawn_point": [x, y],
            "location": [x, y, w, h],
            "create": "map_name_spawn_room_create",
            "enter": "map_name_spawn_room_enter",
            "exit": "map_name_spawn_room_exit",
            "type": "spawn",
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
The ```path``` field contains the path to the image file used for loading the rooms.

The ```generate_init``` field is a function name that is called before generating the map. It returns a void pointer.
The ```generate_cleanup``` field is a function name that is called after generating the map.
The first ```generate``` field is a function name that determines whether a room should be loaded or not. You can use the data in the ```LocalMapGenerationSettings``` struct and the data in your own struct passed from ```generate_init``` to make this decision.
These fields are all optional.

The ```palette``` field contains zero or more colors (formatted as shown above) that correspond to a unit on the map, where a unit is a tile or a wall. Tiles require the ```tex``` field and walls require the ```side_tex``` and ```top_tex``` fields, where the values in these fields are texture names described in the textures.json config file. The ```collide``` field is optional and contains a collision function for that tile. The ```create``` field is optional and contains a create function for when the tile is first made, which is how you set flags for the tile. The ```height``` specifies the height if the tile is a wall; otherwise, this value will be ignored. When ```color``` is "000000", that tile will replace every unfilled tile with that tile.

The ```create``` field described in each room are function names that are called when that room is loaded in, which is used to populate rooms with enemies, obstacles, or more walls. This field is optional.

The ```enter``` field is the name of the function to call when a player enters the room.

The ```exit``` field is the name of the function to call when a player exits the room.

The ```location``` field described in each room are UV coordinates that encompasses the room in the image file described in the "path" field.

The ```type``` field described in each room specifies how the generation should treat that room. For example, the generation algorithm might not put 3 enemy rooms next to each other, or it might not make more than one treasure room in a whole map, or it might make the boss room 10 rooms away from the spawn room. Each map requires a spawn room, but every other room is optional. In this way, the room's type is more like a suggestion for the generation algorithm; there is nothing stopping a spawn room from containing a boss or some treasure. There may be only one spawn room, and spawn rooms must contain a ```spawn_point``` field that specifies where the player should spawn relative to the bottom-left corner of the bounding box described in ```location```.
