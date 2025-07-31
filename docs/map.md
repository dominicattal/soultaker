# Maps

Maps contain rooms that are stitched together at runtime.
There are several types a map room can be: an enemy room, a treasure
room, an event room, a boss room, etc. These are described in the ```map``` config
file. Each map has a corresponding image file that stores all of its rooms.
Each component in the image file describes the shape that it will be, where the
colors in the image file translates to tiles in the game. These are described for
the entire map in the ```colors``` field. The image file is read in as 3 channels, 
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
    "create": "map_name_create",
    "colors": {
        "FF0000": {
            "type": "tile",
            "tex": "tile_tex",
            "collision": "tile_collide"
        },
        "00FF00": {
            "type": "wall",
            "side_tex": "wall_side_tex",
            "top_tex": "wall_top_tex"
        }
    },
    "rooms" {
        "spawn_room": {
            "location": [x, y, w, h],
            "create": "map_name_spawn_room_create",
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

The first ```create``` field is a function name that is called when the map is loaded in, which can be used to tinker with the generation settings. This field is optional.

The ```colors``` field contains zero or more colors (formatted as shown above) that correspond to a unit on the map, where a unit is a tile or a wall. Tiles require the ```tex``` field and walls require the ```side_tex``` and ```top_tex``` fields, where the values in these fields are texture names described in the textures.json config file. The ```collision``` field is optional and contains a collision function for that tile.

The ```create``` field described in each room are function names that are called when that room is loaded in, which is used to populate rooms with enemies, obstacles, or more walls. These fields are optional.

The ```location``` field described in each room are UV coordinates that encompasses the room in the image file described in the "path" field.

The ```type``` field described in each room specifies how the generation should treat that room. For example, the generation algorithm might not put 3 enemy rooms next to each other, or it might not make more than one treasure room in a whole map, or it might make the boss room 10 rooms away from the spawn room. Each map requires a spawn room, but every other room is optional. In this way, the room's type is more like a suggestion for the generation algorithm; there is nothing stopping a spawn room from containing a boss or some treasure.

