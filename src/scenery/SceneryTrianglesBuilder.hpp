#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {
    class SceneryTrianglesBuilder : public RefCounted {
            GDCLASS(SceneryTrianglesBuilder, RefCounted);

        protected:
            static void _bind_methods();

        public:
            static Array build_chunks(const Array &p_triangles, double p_chunk_size_m);
    };
} // namespace godot
