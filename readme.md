# neighbor editor

This is (the still incomplete) level editor for (the also incomplete) neighbor engine, which is a Vulkan based engine.

Currently, this editor allows a rather limited work with convex brushes. With it it's possible to add, delete, translate, scale, rotate and extrude faces. Work on edge and vertex manipulation is still ongoing. The objective of this editor is to be as simple as possible.

It works based on contexts, where each context implements a different funcionality of the editor. Currently, there's a single context implemented, which is the world context. It implements general object manipulation functionality (including brushes).

Contexts operates on states, and each context can be in a single state at the time. This allows the context to simply migrate to states that implement the next step of some thing it needs to do. 

The world context, when no input is being given, is in the idle state. In this state, it constantly checks mouse input (and other keypresses). Once it detects one of those, it'll transition to the appropriate state. If, for example, it detects a left mouse button click, it'll transition to the state that handles left mouse button clicks starting from the idle state. In this next state, at first it tests to see what's underneath the cursor, and stores that. In the next tick, it then tests a few other combinations of input. If the left mouse is being held down, and the mouse is then dragged, it transitions to the brush creation stage, where it keeps track of the box selection made by the user. If, however, the mouse is let go, it transitions to the object selection state, where it properly appends the object to the selection list. And so on, and so on.