# Entity

Entities are described in config/entities.json and assigned an id at runtime. Each texture's id can be queried at runtime using the name in the config file with ```entity_get_id```. There cannot be duplicate names.

## JSON Format

```
"entity_1": {
    "create": "entity_1_create",
    "update": "entity_1_update",
    "destroy": "entity_1_destroy",
    "states": {
        "idle": {
            "update": "entity_1_idle",
            "frames": 1,
            "left": [ "entity_1_left_1" ],
            "right": [ "entity_1_right_1" ],
            "up": [ "entity_1_up_1" ],
            "down": [ "entity_1_up_1" ]
        }
        "walk": {
            "update": "entity_1_walk",
            "frames": 2,
            "timers": [ 0.1, 0.2 ],
            "left": [
                "entity_1_left_1",
                "entity_1_left_2"
            ],
            "right": [
                "entity_1_right_1",
                "entity_1_right_2"
            ],
            "up": [
                "entity_1_up_1",
                "entity_1_up_2"
            ],
            "down": [
                "entity_1_down_1",
                "entity_1_down_2"
            ]
        }
    }
},
"entity_2": {
    "bidirectional": true,
    "states": {
        "idle": {
            "frames": 1,
            "left": [ "entity_2_left" ],
            "right": [ "entity_2_right" ]
        }
    }
}
```

Each entity stores its name and related information as key-value pairs. The entity can be referenced at runtime by using this name with the ```entity_get_id``` function. Each entity has five fields:
- ```create``` has the name of the procedure that should be called when an entity of this type is first created. This is useful for giving entities unique data.
- ```update``` has the name of the procedure that should be called every game update. It is called after the general entity update processes, such as movement and collision.
- ```destroy``` has the name of the procedure that should be called when the entity is destroyed. All allocations made in the create procedure must be deallocated.
- ```bidirectional``` specifies whether the entity has four sets of animation frames (left, right, up, and down) or two sets (left and right). The program will automatically pick one of these sets based on the camera's orientation and the direction the entity is facing. An entity will only be bidirectional if this field exists and is true.
- ```states``` describes the states an entity may be in, which is useful for creating unique entity behavior. This field contains one or more states, and each state can be referenced at runtime with the ```entity_get_state_id``` function by using the specified state name. Each state can be customized with more fields:
    - ```update``` has the name of the procedure that should be called every game update when the entity is in this state. It is called after the method from the other ```update``` field.
    - ```frames``` specifies the number of frames the state has, which is used to corroborate the number of textures specified in the ```left```, ```right```, ```up```, and ```down``` fields. An error occurs if this number does not match one of these fields or if it is less than 1.
    - ```left```, ```right```, ```up```, and ```down``` store an array of textures for the entity when it is in this state, where the textures are names described in the ```texture``` config file. The program will automatically cycle between these textures.
    - ```timers``` specifies how many seconds each frame should last for. For example, in the example, ```entity_1_left_1``` in the ```walk``` state would last 0.1 seconds, while ```entity_1_left_2``` would last 0.2 seconds. The speed at which the frames cycle may be adjusted per entity as well. This field is optional, and the default value is 0.5 seconds for every frame.
