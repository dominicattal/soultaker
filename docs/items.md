# Items

There are five types of items:
- Weapons
    - Used for inflicting damage
    - May have primary and/or secondary attacks
- Abilities
    - Used for buffing the player
- Armor
    - Used just for stats and passive abilities. 
    - Can only have one of each type equipped.
- Accessories (called equipment right now)
    - Used for stats and passive abilities
- Material
    - Items that are used to upgrade items

Items can be bought with souls and sold for souls. They can also drop from entities and found in chests.

Weapons have primary and secondary attacks. If a player has more than one weapon equipped, they will use every weapons' primary and secondary attacks. However, what if the player wants the secondary attack of one weapon without the primary attack? What if the weapon doesn't have a secondary attack? These can be configured in the inventory as well. In addition to being equipped, a weapon may also be enabled for primary or secondary attacks. This way, the player can control which effects are used.

The main difference between weapons and abilities are that abilities take up different slots and don't have primary/secondary attacks.

Accesories may have bonus effects. For example, an accessory might spawn aoes around the player. This differs from the attacks used by weapons. Each item will have to register a control if they want to do something with user input. 

Items may have synergies if certain items are equipped alongside it (see `synergies.md`). For example, a complete set of armor might grant +100 hp.

## JSON Format

```
"item_name": {
    "type": "type",
    "display_name": "Item Name",
    "tooltip": "Description of item",
    "texture": "item_name_tex"
},
"weapon_name": {
    "type": "weapon",
    "display_name": "Weapon Name",
    "tooltip": "Description of Weapon",
    "texture": "weapon_name_tex"
    "primary": "weapon_name_primary",
    "secondary": "weapon_name_secondary",
},
"ability_name": {
    "type": "ability",
    "display_name": "Ability Name",
    "texture": "ability_name_tex"
    "tooltip": "Description of Ability",
    "cast": "ability_name_cast"
},
"armor_name": {
    "type": "armor",
    "display_name": "Armor Name",
    "texture": "armor_name_tex"
    "tooltip": "Description of Armor",
    "use": "armor_name_use",
    "contrl": "control"
},
"accessory_name": {
    "type": "accessory",
    "display_name": "Accessory Name",
    "texture": "accessory_name_tex"
    "tooltip": "Description of Accessory",
    "use": "accessory_name_use",
    "contrl": "control"
},
"material_name": {
    "type": "material",
    "display_name": "Material Name",
    "texture": "material_name_tex"
    "tooltip": "Description of Material",
}
```

`type` is either `weapon`, `ability`, `armor`, `accessory`, or `material` \
`tooltip` is the text displayed next to the weapon in the gui. \
`texture` is the texture used in the gui. \
`attack` is the procedure to be called when the player attacks. This function is where the weapon's logic is implemented.

More fields may be specified dependening on the type of the weapon:
- `weapon`
    - `primary` the function to call on primary attack
    - `secondary` the function to call on secondary attack
- `ability`
    - `cast` the function to call on ability cast
- `armor`
    - `use` the function to call on armor use
    - `control` the control to use to active the use function
- `accessory`
    - `use` the function to call on armor use
    - `control` the control to use to active the use function

