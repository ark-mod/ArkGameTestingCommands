# ARK Beyond API: ARK Game Testing Commands (Plugin)


## Notes & Requirements

For the plugin to work correctly you have to use my forked version of Beyond API from https://github.com/tsebring/ARK-Server-Beyond-API.

This plugin is not stable and not meant for use on a persistent server. It is meant for testing only.

## Introduction

This mod facilitates a lot of advanced testing of creatures, bosses, items, caves etc. It should be used on a private dedicated server.

Additionally, it serves as my testbed for features, data dumping, and analysis of server code in general.

Chat
* **/health**: Prints a summary report of nearby tamed dinos health status.

Console
* **SpawnTemplate `<template key>`**: Spawn custom templated dinos defined in config.json.
Example: `SpawnTemplate griffin`

* **ItemTemplate `<template key>`**: Spawn custom templated items defined in config.json.
Example: `ItemTemplate flak`

* **TeleportTo `<location key>`**: Teleport to a custom location defined in config.json.
Example: `TeleportTo redobi`

* **ReloadTestConfig**: Reload updated templates from config.json.
Example: `ReloadTestConfig`

## Configuration

```json
{
  "ItemTemplates": {
    "flak": {
      "items": [
        {
          "blueprints": [
          	"Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Metal/PrimalItemArmor_MetalHelmet.PrimalItemArmor_MetalHelmet'",
          	"Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Metal/PrimalItemArmor_MetalShirt.PrimalItemArmor_MetalShirt'",
          	"Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Metal/PrimalItemArmor_MetalGloves.PrimalItemArmor_MetalGloves'",
          	"Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Metal/PrimalItemArmor_MetalPants.PrimalItemArmor_MetalPants'",
          	"Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Metal/PrimalItemArmor_MetalBoots.PrimalItemArmor_MetalBoots'"
          ],
          "quality": 10.0
        }
      ]
    }
  },
  "Locations": {
     "redobi": {
 		"x": -262528.094,
 		"y": 241189.141,
 		"z": -11191.670
     }
  },
  "Templates": {
    "theriz_boss": {
      "count": 20,
      "radius": 1000.0,
      "blueprint": "Blueprint'/Game/PrimalEarth/Dinos/Therizinosaurus/Therizino_Character_BP.Therizino_Character_BP'",
      "saddleBlueprint": "Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Armor/Saddles/PrimalItemArmor_TherizinosaurusSaddle.PrimalItemArmor_TherizinosaurusSaddle'",
      "saddleArmor": 100.0,
      "imprint": 1.0,
      "ignoreAllWhistles": false,
      "ignoreAllyLook": false,
      "follow": false,
      "aggressionLevel": "passive",
      "facing": "outwards",
      "baseLevelHealth": 45,
      "baseLevelStamina": 25,
      "baseLevelOxygen": 25,
      "baseLevelFood": 25,
      "baseLevelWeight": 25,
      "baseLevelMeleeDamage": 45,
      "baseLevelMovementSpeed": 25,
      "tamedLevelHealth": 25,
      "tamedLevelStamina": 0,
      "tamedLevelOxygen": 0,
      "tamedLevelFood": 0,
      "tamedLevelWeight": 0,
      "tamedLevelMeleeDamage": 25,
      "tamedLevelMovementSpeed": 0,
      "items": [
        {
          "blueprint": "Blueprint'/Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_SweetVeggieCake.PrimalItemConsumable_SweetVeggieCake'",
          "quantity": 5,
          "count": 10
        }
      ]
    }
  }
}
```

## Spawn template options

| Option | Values | Default |
| --- | --- | --- |
| **aggressionLevel** | passive, neutral, aggressive, attackmytarget | passive |
| **facing** | forward, outwards, inwards | forward |

## Acknowledgements

This plugin is based on Michidu's work on Ark-Server-Plugins and ARK Beyond API. The basic plumbing code is copied directly from those plugins.

## Links

My ARK Beyond API Fork (https://github.com/tsebring/ARK-Server-Beyond-API)

ARK Beyond API by Michidu (https://github.com/Michidu/ARK-Server-Beyond-API)

Ark-Server-Plugins (https://github.com/Michidu/Ark-Server-Plugins)