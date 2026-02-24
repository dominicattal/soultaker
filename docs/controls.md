# Controls

Keys are case-insensitive when taking input from the window. That means if you're holding shift and press the `5` key, glfw will send the keycode for `5` and add shift to the modifier instead of passing a value for `%`. To keep things consistent, all letters are their lowercase variant since the numbers are used instead of their symbol counterpart, among other characters. For the most part, the controls should be intuitive. Will eventually make a list of every key.

Keys can be combined with modifiers to make more complicated controls.
- `S-...` is shift
- `C-...` is control
- `M-...` is alt
- `D-...` is super key
- `L-...` is when caps lock is enabled
- `N-...` is when num lock is enabled

Some examples: 
- `C_ESC` when control is held down when `ESC` is pressed
- `S_C_d` when shift and control are held down when the `d` key is pressed.
- `S_C_M_D_k` when shift, control, alt, and super key are held down while the `k` key is pressed.
- `F11` when `F11` is pressed
- `S_[` when shift is held when `[` is pressed
- `S_C_M_D_L_N_=` when every modifier is held down and the `=` key is pressed

## JSON Format

```
"control1": "S_C_d",
"control2": "C_[",
"control3": "k"
```

Only uses name of control as key and control string as value.
