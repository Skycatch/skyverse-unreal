# Skycatch Skyverse Plugin for Unreal Engine 5

This plugin leverages the Skycatch APIs to easily integrate Skyverse 3d models into Unreal Engine

### Version 1.0.1

## Dependencies

- Unreal Engine v5.1.1
- Cesium For Unreal Engine v1.22.0

## Installation

There are two ways to install this plugin:

### A. Engine Install

This will allow you to use the plugin in any project that uses the Engine

1. Locate the path to your local installation of the Unreal Engine

2. Copy the `SkycatchAPI`folder into the local Engine path in the `Plugins`folder

- e. g. `C:\Program Files\Epic Games\UE_5.0\Engine\Plugins\SkycatchAPI`

3. Build the plugin using the Engine solution

### B. Project Install

This will allow you to use the plugin only for a specific project

1. Copy the `SkycatchAPI`folder into the project directory `Plugins`folder

- e. g. `C:\Projects\MyUEProject\Plugins\SkycatchAPI`

2. Rebuild your project


## Enabling and Configuring the Plugin

After following instructions on the `Installation` section, in the Editor, go to the Plugins Browser. There, search for the Skycatch Skyverse Plugin and mark the checkbox to enable it. This will ask for an Editor restart, click Yes.

In the Project Settings, go to Skycatch Skyverse Section. Here you can set the SKYVERSE KEY and the ENDPOINT parameters.

## Using the Plugin

Once the plugin is enabled, the Skycatch Terrain Actor will be available to place in a scene. Make sure to have a Cesium Georeference already in the scene. Then you can follow some quick steps to test that everything is working correctly.

1. Place a `Skycatch Terrain` Actor in your level
2. In the `Skycatch Terrain` Details Panel:
2.1. In the `Skycatch Terrain Properties` Section, use the selector to set the already existing `Cesium Georeference` Actor in your scene.
2.2. In the `Skycatch Query Params` Section, set the coordinates for the dataset you want to retrieve.

3. Search for the created `Cesium3DTileset` in the World Outliner, and double click it for the viewport camera to focus the tileset.