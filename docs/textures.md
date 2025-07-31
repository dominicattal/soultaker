# Textures

Textures are described in config/textures.json and assigned an id at runtime. Each texture's id can be queried at runtime using the name in the config file with ```texture_get_id```. There may not be duplicate names.

## JSON Format

```
"tex_name_1": "assets/textures/tex_name_1.png",
"tex_name_2": {
    "path": "assets/textures/tex_name_2.png",
    "pivot_x": 0.375,
    "pivot_y": 0.5,
    "stretch_x": 1.375,
    "stretch_y": 1.0
},
"tex_name_3_spritesheet": {
    "spritesheet": true,
    "path": "assets/textures/tex_name_3_spritesheet.png",
    "textures" {
        "tex_name_3_1": [x, y, w, h],
        "tex_name_3_2": [x, y, w, h],
        "tex_name_3_3": [x, y, w, h],
        "tex_name_3_4": {
            "location": [x, y, w, h],
            "pivot_x": 0.375,
            "pivot_y": 0.5,
            "stretch_x": 1.375,
            "stretch_y": 1.0
        }
    }
}
```

There are 3 ways to describe textures in the config file.

The first way is shown with "tex_name_1", where the value is only the path to the image. The whole image is assumed to be the texture.

The second way is shown with "tex_name_2", where the properties of the texture may be modified. Currently, the only properties are pivot and stretch. To explain pivot and stretch, it helps to think about each texture rendering to a 16x16 (in pixels) square by default.

- Pivot describes how much the image should move from the origin. In this example, "tex_name_2" would move 37.5% of its width right, so 6 pixels right, and 50% of its height up, so 8 pixels up.
- Stretch describes how big the square its rendering to should be. In this example, "tex_name_2" would become 37.5% bigger horizontally, so the square its rendering to would become 22x16 instead of 16x16.

Pivot and stretch are essential for creating animations that have several different texture sizes. For example, entities in the game are always rendered as a square, so the stretch and pivot can be used to alter how the game renders the entities.

The third way is shown with "tex_name_3_spritesheet", where each texture is located somewhere in the provided image. "tex_name_3_spritesheet" is not the name of a texture, but rather a name used to identify a group of textures in the config file. The textures that may be used in the program are in the "textures" field, and they have the same properties as the texture in the second example except for the "location" field replacing the "path" field, where the field "location" has an array value containing 4 integers describing where in the image the texture should load from. In order to correctly load textures this way, the "spritesheet" field must be set to true. Otherwise, it will (likely fail to) load the texture the second way.
