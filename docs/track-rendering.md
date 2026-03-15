# Track Rendering System

## Overview
The track rendering system in `libmaszyna` is designed for high performance and scalability, capable of rendering extensive railway networks with minimal CPU overhead. It utilizes a hybrid architecture combining C++ GDExtension, direct `RenderingServer` calls, and GPU-based vertex deformation.

## Architecture

### 1. High-Level Logic: `MaszynaTrack3D` (GDScript)
This node acts as the user interface and data provider for the rendering system.
- **Curve Management**: Extends `Path3D` to leverage Godot's native curve editing tools.
- **Data Preparation**: Samples the `Curve3D` at regular intervals to create arrays of positions and quaternions (orientations).
- **Auto-Smoothing**: Implements a "3-point logic" that automatically calculates smooth tangents and banking (tilt) based on curve radius and track parameters.
- **Template Generation**: Dynamically generates a 2D cross-section of a standard rail (Vignoles profile) and extrudes it into a template mesh for the GPU.

### 2. Performance Core: `TrackRenderer` (C++)
A GDExtension class optimized for low-level interaction with Godot's `RenderingServer`.
- **Direct Server Access**: By using `RenderingServer` RIDs directly, it bypasses the SceneTree overhead for geometry instances. This reduces the cost of transform propagation and Node lifecycle management.
- **Instance Management**:
    - **Rails**: Manages a single mesh instance that is deformed in real-time on the GPU.
    - **Sleepers**: Manages a `MultiMesh` instance, ensuring thousands of sleepers are rendered in a single draw call.
- **Optimization**: 
    - Implements manual **AABB management** to ensure precise culling.
    - Listens for `NOTIFICATION_TRANSFORM_CHANGED` and `NOTIFICATION_VISIBILITY_CHANGED` to update the render state only when necessary.

### 3. GPU Shaders
The heavy lifting of geometry placement is performed on the GPU.
- **Rail Deformation (`track_path_deform.gdshader`)**: Deforms the vertices of the template rail mesh along the path. It uses `instance uniform` variables for length and point count to allow many unique track segments to share the same material.
- **Sleeper Placement (`sleeper_path_deform.gdshader`)**: Uses the `INSTANCE_ID` to calculate the exact position and orientation of each sleeper along the curve, ensuring they remain perfectly anchored to the path.

### 4. Server-Side Rendering (`RenderingServer`)
To achieve high performance, `libmaszyna` bypasses Godot's high-level `SceneTree` and communicates directly with the `RenderingServer` using low-level **RIDs** (Resource IDs).

- **Bypassing the SceneTree**: Standard nodes (like `MeshInstance3D`) carry overhead from lifecycle events and transform propagation. By using `RenderingServer::instance_create()`, we create visual instances that exist purely in the renderer, drastically reducing CPU usage when thousands of tracks are present.
- **Geometry Management**: The server is told to link these instances to specific meshes (the rail template or sleeper MultiMesh). This separation allows for efficient geometry sharing across different track segments.
- **Direct Uniform Injection**: Instead of modifying shared `ShaderMaterial` resources, `TrackRenderer` uses `instance_geometry_set_shader_parameter()` to inject path data (points, quaternions) directly into the instance's GPU memory.
- **Manual Culling**: Because these instances aren't nodes, the system must manually handle visibility. `TrackRenderer` calculates the **AABB (Axis-Aligned Bounding Box)** for each track segment and updates it via `instance_set_custom_aabb()`, allowing the engine to correctly cull off-screen tracks.

## Collision System
The system supports low-poly, dynamically generated collisions for the rails using Godot's `PhysicsServer3D`.
- **Server-Side Physics**: Like the rendering, collisions are managed via RIDs (`physics_body` and `physics_shape`), bypassing the overhead of `CollisionShape3D` nodes.
- **Simplified Geometry**: Instead of the high-poly visual rail profile, the system generates a simple 4-point box shape for each rail segment. This provides a balance between physical accuracy (for walking or wheel interaction) and performance.
- **Dynamic Updates**: Collision shapes are automatically regenerated whenever the track curve changes, ensuring the physical representation always matches the visual one.

## Key Features & Properties
- **Sleeper Spacing**: Configurable distance between sleepers (typically 0.6m).
- **Sleeper Height**: Adjusts the vertical offset of the rails relative to the sleepers without affecting the sleeper's anchor to the terrain.
- **Banking Intensity**: Controls the degree of track tilting in curves, facilitating realistic physics and visuals.
- **Collision Enabled**: Toggleable physics for the rails.
- **GPU-Centric**: Vertex positions are calculated in the shader, meaning track updates (like moving a curve point) have zero CPU cost for geometry regeneration (visual only).

## Step-by-Step: From Node to Screen

### 1. Track Generation & Rendering
A single track segment is processed through the following lifecycle:

1.  **Curve Definition**: The user edits a `MaszynaTrack3D` (Path3D) node in the editor.
2.  **Data Sampling**: When the curve changes, `MaszynaTrack3D` samples the `Curve3D` at regular intervals (up to 256 points). It extracts positions, orientations (quaternions), and tilt data.
3.  **C++ Synchronization & Server Instance**: The sampled data is passed to the `TrackRenderer` C++ class. Internally, this class creates raw **RIDs** via `RenderingServer::instance_create()`. This bypasses the SceneTree completely, ensuring the track exists only as a lightweight visual object in the renderer's memory.
4.  **Geometry & Shader Binding**: The `TrackRenderer` tells the `RenderingServer` to use the rail template mesh (or sleeper MultiMesh) as the "base" for these instances via `rs->instance_set_base()`. It then assigns the appropriate `ShaderMaterial` to the instance RID.
5.  **Direct Uniform Upload**: Instead of updating a high-level material property, `TrackRenderer` calls `rs->instance_geometry_set_shader_parameter()`. This "injects" the point and quaternion arrays directly into the instance's own GPU memory, allowing thousands of tracks to share the same material resource while having unique paths.
6.  **GPU Deformation & Drawing**: The `RenderingServer` sends the geometry to the GPU.
    -   **Rails**: The vertex shader reads the injected path data and bends the template mesh in real-time.
    -   **Sleepers**: The server handles the `MultiMesh` drawing, while the shader uses `INSTANCE_ID` to anchor each sleeper to its specific coordinate on the path.
7.  **Collision Server Sync**: Simultaneously, the system bypasses `CollisionShape3D` nodes by calling `PhysicsServer3D` directly. It generates raw collision faces (convex/concave shapes) and updates the physical RID, ensuring the "physics world" stays in sync with the "visual world" with zero Node overhead.

### 2. Train Movement & Physics
The `RailVehicle3D` node manages how a train interacts with the track:

1.  **Longitudinal Movement**: The `TrainController` calculates the current velocity. `RailVehicle3D` then updates its `distance_on_track` property: `distance += velocity * delta`.
2.  **Track Transitions**: If `distance` exceeds the current track's length (or goes below zero), the vehicle queries the `next_track` or `previous_track` properties to seamlessly transition to the next segment.
3.  **Bogie Snapping**:
    -   The vehicle identifies its bogies (usually two).
    -   It calculates the exact 3D position for each bogie by sampling the track at `distance + bogie_offset`.
4.  **Car Body Alignment**:
    -   The main car body position is calculated as the midpoint between the two bogies.
    -   The car body's rotation is derived from the vector pointing from one bogie to the other, ensuring it "leans" into curves correctly.
5.  **Wheel Animation**: Based on the `wheel_rotation` (RPM) provided by the controller, the wheel nodes are rotated around their local X-axis to match the ground speed.
6.  **Kinematic Sync**: The final calculated transform is pushed to the `PhysicsServer3D` as a kinematic body, allowing the train to interact with the environment (e.g., triggering signals or colliding with buffers).

## Performance Summary
- **Draw Calls**: Only 2 draw calls per track segment (1 for rails, 1 for sleepers).
- **CPU Usage**: Minimal; CPU only handles path sampling and uniform updates.
- **Scalability**: Designed to handle 10,000+ track segments with negligible impact on framerate.
