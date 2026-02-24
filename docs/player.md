# Player

The player and the player's entity are distinct. The player's entity lives inside of the map just like any other entity. The player represents the state that is carried between maps. The player's entity, just like any other entity, is loaded from the config file. However, the entity loaded must have the `idle`, `walking`, and `shooting` states.

The player has an inventory. The inventory is where items are configured (see `items.md`). The player can have 1 helmet, 1 chestplate, and 1 boots equipped. The player can have as many weapons as the Weapon Slot stat allows. Likewise, the player can have as many abilities as the Ability Slot stat allows. Every item costs some amount of souls to equip and must not exceed the amount of Soul Power the player has.

The player has stats. Each item scales differently with different stats, and each item may grant the player additional stats. This increase can be either additive or mulitplicative, with additive increases being applied before multiplicative increases.
- (Max) Health
- (Max) Mana
- Soul Power
- Attack Damage
- Magic Damage
- Attack Penetration
- Magic Penetration
    - Both penetrations apply additively
- Crit Damage
- Crit Chance
    - Crit chance beyond 100% is converted to crit damage
    - True damage cannot crit
- Attack Speed
    - Affects cooldown of primary attacks
- Ability Haste
    - Affects cooldown of secondary attacks
- Magic Resistance
- Armor
- Speed
- Vitality (HP Regen)
- Wisdom (MP Regen)
- Lifesteal
- Weapon Slots
    - Determines how many weapons you may have equipped
- Ability Slots
    - Determines how many abilities you may have equipped

Debuffs will also affect player stats, but that isn't implemented yet
