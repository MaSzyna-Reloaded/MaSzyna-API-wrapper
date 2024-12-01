# MaSzyna-API-wrapper
## About the project
This C++ GDExtension for Godot Engine 4.3 offers a simplified interface to the MaSzyna train simulator API. Tailored for the MaSzyna Reloaded project, it enables developers to construct train components using Godot's Node-based system, providing a visual and intuitive approach to train creation and management. Key features include:

- Custom Node-based Classes: Create train elements using our premade and integrated classes based on Godot's familiar Node system.
- Parameter Customization: Fine-tune train parameters directly within the Godot editor.
- Simplified Integration: Seamlessly integrate the MaSzyna simulator physics into your Godot projects.
- Enhanced Debugging: Streamline the debugging process for your train simulations.
## Code style
### General
1. Function naming: `snake_case`
2. Variable naming: `snake_case`
3. Class naming: `PascalCase`
4. Parameter naming: `snake_case` with `p_` as an prefix
5. Braces at the end of line (K & R style)
6. As less nesting as possible
7. Indentation: 4 spaces
```cpp
void set_door_open(bool p_state) {
    if (condition) {
        door_state = p_state;
        return;
    }
    //Do something else
    return;
}
```
### Classes
1. Explicit privacy declarations
