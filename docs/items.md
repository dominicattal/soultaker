# Items

There are four types of items:
- Weapons
    - Used for inflicting damage
    - May have primary or secondary attacks
- Armor
    - Used just for stats and passive abilities. 
    - Can only have one of each type equipped.
- Accessories (called equipment right now)
    - Items that have no primary or secondary attack and grant some stats
- Material
    - Items that are used to upgrade items

Items can be bought with souls and sold for souls. They can also drop from entities and found in chests.

Weapons have primary and secondary attacks. If a player has more than one weapon equipped, they will use every weapons' primary and secondary attacks. However, what if the player wants the secondary attack of one weapon without the primary attack? What if the weapon doesn't have a secondary attack? These can be configured in the inventory as well. In addition to being equipped, a weapon may also be enabled for primary or secondary attacks. This way, the player can control which effects are used.

Accesories may have bonus effects. For example, an accessory might spawn aoes around the player. This differs from the attacks used by weapons. Each item will have to register a control if they want to do something with user input. 

Items may have synergies if certain items are equipped alongside it. For example, a complete set of armor might grant +100 hp.

## JSON Format

```
"item_name": {
    "tooltip": "Description of item",
    "attack": "item_name_attack",
    "texture": "item_name_tex"
}
```

```tooltip``` is the text displayed next to the weapon in the gui.

```texture``` is the texture used in the gui.

```attack``` is the procedure to be called when the player attacks. This function is where the weapon's logic is implemented.
