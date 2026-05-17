# UE5 AI Project / FalseOrders

Small Unreal Engine 5 C++ prototype focused on enemy AI, character movement, weapon handling, and animation integration.

[Video](https://youtu.be/ESm8_Zni0ZI)

## Current Features

- C++ enemy character based on a shared character base class
- AI Controller with perception support (only perception for now)
- Blackboard / Behavior Tree integration
- Patrol point selection using Unreal navigation
- Alert states: Passive, Suspicious, Alerted, Searching, Combat
- Character movement state tracking for animation
- Aiming state support
- Weapon equip, fire, and reload hooks
- Replicated base character state prepared for future co-op/multiplayer work
- Animation Blueprint integration through character state instead of directly reading Blackboard values

## Project Structure

    Config/                  Unreal Engine project configuration
    Source/FalseOrders/      Main C++ source code
      AI/                    Enemy AI, AI controller, behavior tree tasks, AI types
      Character/             Base character logic, movement, life state, animation state
      Combat/                Weapon base classes and weapon animation types
    FalseOrders.uproject     Unreal Engine project file

## AI Architecture

The project uses a clean separation between AI decision-making and animation:

    Behavior Tree / Blackboard
            ↓
    Enemy AI Controller
            ↓
    Enemy Character / Character Base state
            ↓
    Animation Blueprint

The Animation Blueprint reads character state such as speed, aiming, stance, movement state, life state, and equipped weapon type.

## Important Notes

This repository is currently code-focused. Large Unreal Engine generated folders and asset folders are intentionally not included. (too big)

## Development Status

Early prototype.

Current development focus:

- Enemy locomotion animation
- AI aiming toward target
- Combat behavior
- Weapon animation hooks
- Clean C++ architecture for future tactical co-op gameplay
