Written in C

<h1>--------------- Hardware ------------------</h1>
This engine is designed for the hackclub sprig. It has 8 buttons, a 1.8" TFT adafruit screen with SD card, and a speaker. It is powered
by two double-A batteries and controlled by a raspberry PI pico W.

<h1>-------------- Interpreter ---------------------</h1>
Custom made interpreter uses shunting yard algorithm to parse lines and call functions.

Structure:

EngineUnion (holds a value; int, string, etc..)
  |
  V
EngineVar (Contains an EngineUnion as well as stores the current type and a name)
EngineFunction (Contains a function and its start and end points within a script)
  |
  V
ScriptData (Contains the current state of script execution)
EngineObject (Contains object data and the scripts attached to it)
  |
  V
EngineScene (Contains objects)

EngineScript (Contains the content of a script and its name)
EngineSprite (Contains the data for a sprite)



<h1>-------------- Smart Renderer ------------------</h1>

1. Normal
This mode is used for all the editor UI. It bipasses the smart renderer and manually draws rectangles and text onto the screen,
clearing only when required. Great for UI, but not for quick refresh rates.

3. Fast but Flicker
This mode clears the entire screen and redraws it every frame. Light for the CPU, but causes flickers and refresh delay

4. On Change
This mode draws only the pixels that have changed between frames. It is the recomended mode as it is the quickest overall.

5. Slow but Smooth
First this mode scans for pixels that have changed, and creates a pallete using the colors on the screen.
Then an algorithm takes each color and converts it into the least number of rectangles. While very efficient and smooth,
it requires more CPU time making it slower than "On Change".

<h1>------------ Customizability -------------------</h1>
Custom scripts, layouts, and sprites can be created using the UI. Everything is saved to the
SD card on the TFT display.
