# Synergies

When certain items are equipped together, they can synergize and become stronger. When a synergy is active, the callbacks in each item are are not called; instead the synergy's callback is called. The synergy can access the item callbacks in its callback, so you can still call them if you want.

## JSON Format

```
"synergy": {
    "display_name": "Synergy Name",
    "tooltip": "Description of Synergy",
    "items": [
        "item1",
        "item2",
        "item3",
        ...
    ],
    "primary": "synergy_primary",
    "secondary": "synergy_secondary",
    "cast": "synergy_cast",
    "use": "synergy_use",
    "control": "synergy_control"
},
```
