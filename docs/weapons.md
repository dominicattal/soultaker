# Weapons

TBD

## JSON Format

```
"weapon_name": {
    "tooltip": "Description of weapon",
    "attack": "weapon_name_attack",
    "texture": "weapon_name_tex"
}
```

```tooltip``` is the text displayed next to the weapon in the gui.

```texture``` is the texture used in the gui.

```attack``` is the procedure to be called when the player attacks. This function is where the weapon's logic is implemented.
