![banner-mono](https://github.com/user-attachments/assets/3548f3e1-35e4-4a53-ae5b-6527f2a02fb6)

Snake game
---

A typical snake game: a snake, an apple, walls. Eat - you grow. Crash into a wall or try to eat yourself - you lose.
Leave no free space on the field - your victory.

Everything is tied to maintaining a balance between speed and hardware load (TODO),
which allows storing a huge number of cells in small amount of RAM.

![2025-01-18-160550_1920x1080_scrot](https://github.com/user-attachments/assets/130e9915-a959-4dc8-a50b-ca3d3c39a2e1)  
*Snake game runnning on emulator of Commodore 64 (VICE).*

---

#### Cells storage
Snake cells are stored as follows:

* the head is classically represented by two coordinates, 8 bits per coordinate;
* the coordinates of other cells are represented as directions relative to the previous cells, starting from the head;
* all directions are stored in one array with 4 directions per byte;

#### Apple generation
Game field is subdivided according to some algorithm until one cell remains, the coordinates of which will be the coordinates of the apple.

First, the field is divided horizontally until one vertical line remains. Then this line is divided until one point remains.

Which region (left/right in the case of horizontal division, top/bottom in the case of vertical) will be used for the next subdivision
is determined by a function that counts the number of snake cells in each of the subdivisions.

![snake-count](https://github.com/user-attachments/assets/2866446f-8170-4bcd-af32-1b0f3d268c5c)  
*Couting cells.*

* if both divisions are occupied by snake cells, then the game ends in victory as the maximum snake length has been reached;
* if only one of the divisions is occupied by snake cells, then another division is selected;
* if both divisions have at least one free cell, then the solution is determined randomly.

This algorithm generates random coordinates with a uniform distribution (and not a normal, as it may seem).

![apple-generator](https://github.com/user-attachments/assets/99439d4f-0594-44c8-a101-b8d878cb4998)  
*Generating apples.*
