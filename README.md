# Playground

A small, experimental, data-driven, entity-component-based, Lua-scriptable game engine. See below for a small programming example.

----------

## Compiling

The source code uses GCC pragmas and C++14.

#### Linux
On Linux, once the dependencies have been installed, just run `make` to build the program and run the tests. The result will appear in the `./Build` folder.

#### Windows
Compiling on Windows isn't a great experience at the moment. Your locations of the dependencies should be entered into the `*_COMP` and `*_LINK` fields, at the beginning of the Makefile. Once that has been done, the build is done the same way as on Linux.

## Usage

For more detailed information, see the README files in the module folders within `src/`.

The scene is described in a data file. The data is written in the JSON format. Each entity is listed as an object in a list. Each entity is composed of components, each of whom have their own object notation.

Currently the data file has to be written by hand, but in the future it will be generated by an editor.

A small look at the available components and their JSON notation follows.

#### Transform

This component describes the position, orientation, and scale of an entity. It's JSON notation is:

```json
"transform": {
  "position": [ NUMBER, NUMBER, NUMBER ],
  "rotation": [ NUMBER, NUMBER, NUMBER, NUMBER ],
  "scale": [ NUMBER, NUMBER, NUMBER ]
}
```

`position` is a 3d vector containing the world coordinates of the entity. `rotation` is a quaternion, and it's components should be normalized. `scale` is also a 3d vector, and its components should be non-zero.

#### Script

Entities can be scripted in Lua. The script component JSON notation simply contains the location of the script file.

```json
"script": STRING
```

#### Renderable

This component will be rendered with OpenGL. The component consists of a 3d model, and a shader. Currently, only the specular shader should be used at the moment.


*Component dependency: transform*

```json
"renderable": {
  "model": STRING,
  "specular": {	
    "shininess": NUMBER,
    "specularColor": [ NUMBER, NUMBER, NUMBER ],
    "ambientColor": [ NUMBER, NUMBER, NUMBER ]
  }
}

```

`"model"` is the just the file path where the 3d model is located.

More about the shading system later.

#### Camera

The camera component is used for rendering the renderable components.

*Component dependency: transform*

```lua
"camera": {
  "fov": NUMBER,
  "nearPlane": NUMBER,
  "farPlane": NUMBER 
}
```

#### A small scene

Here is a small example of how to build a simple scene.

`data/scene.json`:

```json

[
    {
        "script": "data/template.lua",
        "transform": {
            "position": [ 0.0, 0.0, 8.0 ],
            "rotation": [ 0.0, 0.0, 0.0, 1.0 ],
            "scale": [ 1.0, 1.0, 1.0 ]
        },
        "camera": {
            "fov": 1.32,
            "nearPlane": 0.1,
            "farPlane": 10000
        }
    },
    {
        "transform": {
            "position": [ 0.0, 0.0, 0.0 ],
            "rotation": [ 0.0, 0.0, 0.0, 1.0 ],
            "scale": [ 0.001, 0.01, 0.001 ]
        },
        "renderable": {
            "model": "data/cube.dae",
            "specular": {
                "shininess": 80.0,
                "specularColor": [ 1.0, 1.0, 1.0 ],
                "ambientColor": [ 0.941, 0.455, 0.804 ]
            }
        }
    }
]

```

`data/template.lua`:

```lua
-- this gets called just after the component is assigned
function activate()
end

-- accumulated time
t = 0.0
-- angular velocity
av = 0.5
r = 8.0

-- this gets called in the update loop
function update( dt )
    if entity:hasTransform() then
        t = t + dt
        entity.transform.position = pg.Vector3f( r*math.cos( t*av ), 0.0, r*math.sin( t*av ) )
    end
end

--  this gets called just before the component is removed
function deactivate()
    print("Script gonna hang up now!")
end
```

`config.lua`:

```lua
targetFrameRate = 60.0

window = {
    width = 800,
    height = 600,
    name = "Playground engine",
    opengl = {
        major = 3,
        minor = 3,
        stencil_bits = 8,
        depth_bits = 24,
        multisample_buffers = 1,
        multisample_samples = 4
    }
}
```

## Scripting interface

**TODO: documentation**

## TODO
* Convert config file format to JSON
* Add scene parser, serialization/deserialization should go there
* Reconsider the assertions in MeshManager and ShaderManager.
* Write tests for Bundle
* Implement specular shaders
  * Add array of lights to specular shader
  * Add Light count
  * Read orientation from DirectionalLight component
  * add gamma correction to specular shader
* Material should use a union of material type structs, get rid of the verbosity of per-float uniform names.
  * I think this is justified, because the Render system is the one which provides the rendering service; materials are a part of that process.
  * Drawback: makes extension more difficult. Not a problem for now.
* Implement free-look camera script
* Normalize resource names in MeshManager and ShaderManager using r-lyeh's Unify lib.
* Rendering matrices use Matrix4f
* Implement Transform component & rendering system using my math module
* Figure out a mechanism to make `componentPointer<C>` private/not part of Entity API.
* Bundle has iterators
* Add R'lyeh's profit lib to profile execution times of critical methods
* Integrate ImGui into a system
  * I need to figure out how to render textured panels isometrically using modern OpenGL
    * This is explained in `imgui/examples/opengl3_example/imgui_impl_glfw_gl3::CreateDeviceObjects()`
  * Add event handling functions for the Ui system:
  * Add handler for text input event
  * add handler for keyup input event
* `EntityManager` needs to expose the pool size in its public API.
Here's a great way to handle developer errors: if, for instance, a mesh isn't found, use an "error" mesh instead which will be easy to spot. There is no need to halt execution because of an erroneous path to a resource.

Remember to distinguish between hard and soft errors. Hard errors occur in places like `Bundle` where an off-by one error leads to corrupted state. Soft errors occur e.g. when a user supplies an incorrect file name. The program still works correctly, it just has been given incorrect information.

## Dependencies
### SDL2
zlib license
Feel free to do anything you want with it, so long as you don't misrepresent who wrote the original software, license must be included
### glm
Happy bunny license, MIT license
Feel free to use and redistribute, but include the license
### glew
MIT license
### Lua 5.2
MIT license
### Assimp
3-clause BSD license, do what you want, but include the license text.
### LuaBridge
MIT license

