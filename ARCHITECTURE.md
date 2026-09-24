# Minimal 3D Modeler - Architecture Overview

## What We Just Built

A foundational CAD modeler architecture in C + raylib with these core systems:

### 1. **Scene Graph System** (core/scene.c/h)
- Hierarchical node tree for organizing geometry and features
- Node types: ROOT, SOLID, SKETCH, FEATURE, ASSEMBLY, REFERENCE_GEOMETRY
- Parent-child relationships with dynamic array management
- Selection management (multi-select support)
- Document state tracking (modified, needs_rebuild flags)
- Prepared for undo/redo history (stubbed)

### 2. **Sketch System** (sketch/sketch.c/h)
- 2D sketch entities: Points, Lines, Circles, Arcs
- Constraint system: Distance, Angle, Perpendicular, Parallel, etc.
- Sketch planes (XY, XZ, YZ)
- Constraint solver interface (basic validation for now)
- Closed loop detection for extrusion profiles

### 3. **Feature System** (eatures/extrude.c/h)
- First real CAD operation: Extrude
- Extrude parameters: depth, symmetric, taper angle
- Integration with sketch system
- Conversion to SDF representation for rendering
- Extensible design for future features (Revolve, Fillet, Chamfer, Boolean ops, etc.)

### 4. **SDF (Signed Distance Field) System** (enderer/sdf.c/h)
- Primitive SDFs: Sphere, Box, Cylinder, Cone, Torus, Plane
- Boolean operations: Union, Subtraction, Intersection
- Smooth CSG operations (smooth_union, smooth_subtract, smooth_intersect)
- Transform support (local space evaluation via inverse transforms)
- Tree evaluation (recursive SDF evaluation)
- Normal estimation for lighting

### 5. **Raymarching Renderer** (enderer/raymarch_renderer.c/h)
- Bridges scene graph → SDF tree → GPU rendering
- Fullscreen quad raymarching with GLSL shader
- Camera management (position, target, FOV)
- Extensible for future rendering modes

### 6. **Math Foundation** (core/math_utils.c/h)
- Vec2, Vec3, Mat4 operations
- Matrix transforms (translate, rotate, scale, multiply)
- Point/vector transformations
- Inline vector operations for performance

---

## Current Architecture Diagram

`
Application (main.c)
    ↓
SceneDocument
    ├─ SceneNode (Root)
    │  ├─ SceneNode (Extrude Feature)
    │  │  └─ data: ExtrudeFeature
    │  │      └─ Sketch reference
    │  └─ ... other nodes
    │
    ├─ Camera state
    ├─ Selection (multi-select)
    └─ Flags (modified, needs_rebuild)

         ↓
    Sketch System
    └─ Entities (lines, circles, etc.)
    └─ Constraints
    └─ Solver state

         ↓
    Feature Evaluation
    └─ Extrude → SDF Box (placeholder)
    └─ Future: Revolve, Fillet, Hole → SDF equivalents

         ↓
    SDF Tree Builder
    └─ Union/Subtract/Intersect operations
    └─ Final SDF: Composite → GPU

         ↓
    Raymarching Renderer
    └─ Fullscreen quad
    └─ Shader: raymarch.fs
    └─ Output: Shaded 3D view
`

---

## Workflow (Feature-Based, Like SolidWorks)

1. **Create Sketch** on a plane (XY, XZ, YZ)
2. **Draw Profile** (lines, circles, arcs)
3. **Constrain Sketch** (distances, angles, perpendicular, etc.)
4. **Extrude** sketch profile → 3D solid
5. **Future**: Add more features (Revolve, Fillet, Hole, Boolean ops)
6. **Render**: SDF tree → Raymarching → Real-time preview

---

## What's Ready to Use

- ✅ Scene graph creation and manipulation
- ✅ Sketch creation with entities
- ✅ Extrude feature creation
- ✅ SDF tree building from features
- ✅ Raymarching rendering pipeline
- ✅ Camera controls (right-drag to rotate, scroll to zoom)
- ✅ UI panels (Feature tree, Properties, Toolbar)

---

## Next Steps to Expand

### Short term (1-2 features):
1. **Sketch Constraints Solver** - Proper iterative constraint solving (currently just validates)
2. **Revolve Feature** - Rotate sketch profile around axis
3. **Boolean Operations** - Union, Subtract, Intersect between features
4. **Fillet/Chamfer** - Edge rounding (via SDF operations)

### Medium term:
1. **Hole Feature** - Parametric hole from sketch circle
2. **Linear/Circular Pattern** - Duplicate features with transforms
3. **Shell Feature** - Hollow out solids
4. **Rib/Boss** - Add ribs or bosses

### Long term:
1. **Mesh Export** (STL, STEP)
2. **Assembly System** - Multiple parts
3. **Simulation Hooks** - FEA integration
4. **Full STEP Import** - Read real CAD files

---

## Build Command

`ash
gcc -Wall -Wextra -std=c17 -O2 \
  -IC:/w64devkit/include \
  -IC:/Users/cocog/projects/minimal-3Dmodeler \
  -o main.exe src/main.c \
  core/scene.o core/math_utils.o \
  sketch/sketch.o \
  renderer/sdf.o renderer/raymarch_renderer.o \
  features/extrude.o \
  -LC:/w64devkit/lib -lraylib -lopengl32 -lgdi32 -lwinmm
`

Or use the Makefile (Unix-style, but Windows-compatible with make.exe from w64devkit).

---

## File Structure

`
minimal-3Dmodeler/
├── core/              # Core data structures & math
│   ├── types.h        # Vec2, Vec3, Mat4, enums
│   ├── math_utils.h/c # Vector & matrix operations
│   ├── scene.h/c      # Scene graph (nodes, documents, selection)
│   └── scene.h        # Updated with all function declarations
│
├── sketch/            # 2D Sketching system
│   ├── sketch.h/c     # Entities, constraints, solver stubs
│
├── features/          # 3D Features (Extrude, Revolve, etc.)
│   ├── extrude.h/c    # First feature: Extrude
│
├── renderer/          # Rendering backend
│   ├── sdf.h/c        # SDF primitives & operations
│   ├── raymarch_renderer.h/c  # GPU raymarching integration
│   └── raymarch.fs    # Existing shader (unchanged)
│
├── src/
│   ├── main.c         # Application entry point
│   ├── main.exe       # Compiled binary
│   └── raymarch.fs    # Fragment shader
│
└── build/             # Build artifacts (object files)
`

