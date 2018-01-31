# ARK Server API: ARK Game Testing Commands (Plugin)

## Introduction

This mod facilitates a lot of advanced testing of creatures, bosses, items, caves etc.

Chat
* **/health**: Prints a summary report of nearby tamed dinos health status.

Console
* **health**: Prints a summary report of nearby tamed dinos health status (via notification).
Example: `Health`

* **SpawnTemplate `<template key>`**: Spawn custom templated dinos defined in config.json.
Example: `SpawnTemplate griffin`

* **ItemTemplate `<template key>`**: Spawn custom templated items defined in config.json.
Example: `ItemTemplate flak`

* **TeleportTo `<location key>`**: Teleport to a custom location defined in config.json.
Example: `TeleportTo redobi`

* **ArkGameTestingCommands.ReloadConfig**: Reload updated templates and settings from config.json.
Example: `ArkGameTestingCommands.ReloadConfig`

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

This plugin is based on Michidu's work on Ark-Server-Plugins and ARK Server API. The basic plumbing code is copied directly from those plugins.

## Links

ARK Server API (http://arkserverapi.com)

ARK Server API [GitHub] (https://github.com/Michidu/ARK-Server-API)

Ark-Server-Plugins [GitHub] (https://github.com/Michidu/Ark-Server-Plugins)