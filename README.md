
Demo  link: https://youtu.be/d34lxe9QW-U

<h1>------------- Optimization ----------------</h1>

- I used space partitioning to reduce the amount of CPU usage when checking collision between objects
- I used a load balancer to make sure both cores were being used efficiently
- I used a string memory pool to reduce memory exhaustion
- I used linked lists for dynamic arrays and efficient look up
- I created a smart renderer that compares frames to prevent redundant updates

<h1>--------------- Hardware ------------------</h1>
This engine is designed for the hackclub sprig. It has 8 buttons, a 1.8" TFT adafruit screen with SD card, and a speaker. It is powered
by two double-A batteries and controlled by a raspberry PI pico W.

<h1>--------------- Software ------------------</h1>

The engine was written entirely in C by me.

<h2> Interpreter </h2>
Custom made interpreter uses shunting yard algorithm to parse lines. It has a call stack to run functions and branch code.

<h2> Smart Renderer </h2>

The renderer uses a buffer that stores the state of the pixels on the screen and updates the screen depending on the mode:

1. Normal
This mode is used for all the editor UI. It bipasses the smart renderer and manually draws rectangles and text onto the screen,
clearing only when required. Great for UI, but not for quick refresh rates.

3. Fast but Flicker
This mode clears the entire screen and redraws it every frame. Light for the CPU, but causes flickers and refresh delay

4. On Change (default)
This mode draws only the pixels that have changed between frames. It is the recomended mode as it is the quickest overall.

5. Slow but Smooth
First this mode scans for pixels that have changed, and creates a pallete using the colors on the screen.
Then an algorithm takes each color and converts it into the least number of rectangles. While very smooth,
it requires more CPU time making it slower than "On Change".

<h2> Collision </h2>

The collision engine first parses space into "cells" that completely cover all of the colliders of the objects. Then each cell is iterated over to check for collisions using Seperating Axis Theorem (SAT), which allows for collision detection while the objects are rotated.

<h2> Load Balancer </h2>

The Raspberry Pi Pico W has 2 cores. To maximize efficiency, tasks are assigned to the cores dynamically based on load. This ensures that neither core is idle during runtime. The primary purpose of core 0 is object manipulation, script execution, and load balancing. Core 1's primary purpose is rendering. If core 1 finishes early, it will start processing collisions, reducing idle time and allowing core 0 to catch up. The cores communicate with eachother using FIFO stacks.

<h1> Runtime Manager </h1>

All of these processes combine together to run the user programs. This flowchart shows the interaction and ordering of the processes.
<br>
<img width="2124" height="2294" alt="FlowChart" src="https://github.com/user-attachments/assets/92cea3b1-bd1e-433e-b949-52e1817b627a" />

