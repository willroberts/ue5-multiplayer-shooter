# Blaster Game (working title)

This is a multiplayer third-person shooter developed in Unreal Engine 5.

## Changes to Base Content

This project makes use of the following marketplace content:

- Unreal Learning Kit: Games
- Military Weapons Silver

The following changes were made to this content:

- Added right hand socket to EpicCharacter skeleton
- Added left hand socket to Weapon skeletons

The EpicCharacter's skeleton was duplicated as `SK_Character`, and animations were retargeted. For the weapons, the skeleton was modified in-place. (TODO: Duplicate weapon skeletons to get them into source control).
