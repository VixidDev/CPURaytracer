# CPU Raytracer

University assignment to implement a raytracer using the CPU by manually calculating each ray per pixel and recursively calculating any shadows, reflection and refraction through the scene, also implements Monte Carlo sampling with variable anti aliasing sampling to reduce noise, along with russian roulette to randomly terminate some secondary rays. Uses OpenGL to render the base scene on the left side and also to render the output of the raytracer on the right as the image generates.

### Examples

| Scene | Settings | Result |
|--- |--- |--- |
| Cornell box | Phong, Shadows, Monte Carlo (10x anti alias samples) | <img width="480" alt="10x_aa" src="https://github.com/user-attachments/assets/f4b936cc-28a8-4619-b787-479b938d9bdc" /> |
| Cornell box | Phong, Shadows, Reflection, Refraction, Fresnel | <img width="480" alt="refraction_w_fresnel" src="https://github.com/user-attachments/assets/02fb00e1-66c4-4f48-a384-bde4d6450ca8" /> |
| Cornell Suzanne | Phong, Shadows, Reflection, Refraction | <img width="480" alt="suzanne_reflection_refraction" src="https://github.com/user-attachments/assets/6c3e53c3-72dd-429b-8879-e484f77803c9" /> |

#### Keybinds

- `1` - Enable interpolation rendering
    - Renders the normals of each fragment in scene. Overwrites any other setting.
- `2` - Enable phong rendering
- `3` - Enable shadows
- `4` - Enable reflections
- `5` - Enable refraction
- `6` - Enable fresnel
- `7` - Enable Monte Carlo sampling (is slow)
- `R` - Start raytrace rendering
- `P` - Toggle orthographic projection

Typically you enable 2, 3, 4, 5 and then press R to get a typical raytraced scene in a reasonable time. Enabling Monte Carlo does result in slower raytracing and it is recommended to increase change the `ANTI_ALIAS_SAMPLES` define in `Raytracer.cpp` to get better Monte Carlo results with less noise but obviously much much slower.

## Usage

To compile on Linux:

```bash
premake5 gmake2
make -j
```

To compile on Windows:

```bash
premake5 vs2022
```

Then open up .sln file in Visual Studio.

### Running

Some example scenes are provided in the `objects` directory which include an `.obj` and `.mtl` file which must be passed in as program arguments with the `.obj` first and `.mtl` file second. The `.mtl` files can be altered to add mirror or transparency to some parts of a scene.

This is easy from the terminal but I recommend [Smart Command Line Arguments VS2022](https://marketplace.visualstudio.com/items?itemName=MBulli.SmartCommandlineArguments2022) extension for Visual Studio to be able to quickly make and switch the program arguments the program runs with when pressing the run button in Visual Studio.
