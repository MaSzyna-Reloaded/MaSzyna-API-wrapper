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
exactly anchored to the path.

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

## Performance Summary
- **Draw Calls**: Only 2 draw calls per track segment (1 for rails, 1 for sleepers).
- **CPU Usage**: Minimal; CPU only handles path sampling and uniform updates.
- **Scalability**: Designed to handle 10,000+ track segments with negligible impact on framerate.
