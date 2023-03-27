# Blaster Game (working title)

This is a multiplayer third-person shooter developed in Unreal Engine 5.

## Changes to Base Content

This project makes use of the following marketplace content:

- Animation Starter Pack
- Military Weapons Silver
- Unreal Learning Kit: Games

Changes to characters and animations were done by first duplicating the asset, so changes to these assets are tracked in version control.

Changes to weapons have not yet been replicated in version control. Instead, the following changes were made directly to the assets:

- Added left hand socket to Weapon skeletons for hand placement via FABRIK IK
- Rotated AmmoEject sockets on Weapon skeltons to correctly orient ejected casings
- Added a Emissive Color parameter to the M_WeaponMaster_01 material (used by material instances)
- Changed material of `AssaultRifleA_Ammo` to our own emissive material

TODO: Duplicate above assets to get these changes into source control.