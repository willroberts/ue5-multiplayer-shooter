# Blaster Game (working title)

This is a multiplayer third-person shooter developed in Unreal Engine 5.

## Marketplace Content

This project makes use of the following marketplace content:

- Animation Starter Pack
- Military Weapons Silver
- Paragon: Dekker (for respawn FX)
- Shooter Sample Game (for additional weapon audio)
- Unreal Learning Kit: Games (for character skeleton/mesh)

The following changes were made directly to Weapon assets:

- Added left hand socket to Weapon skeletons for hand placement via FABRIK IK
- Rotated AmmoEject sockets on Weapon skeltons to correctly orient ejected casings
- Added a Emissive Color parameter to the M_WeaponMaster_01 material (used by material instances)
- Changed material of `AssaultRifleA_Ammo` to our own emissive material

TODO: Duplicate above assets to get these changes into source control.

## Credits

- Additional animations from www.Mixamo.com
- Additional sounds effects from www.ZapSplat.com
