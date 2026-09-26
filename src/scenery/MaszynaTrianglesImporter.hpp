#pragma once

#include "parsers/maszyna_parser.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    class MaszynaTrianglesImporter : public RefCounted {
            GDCLASS(MaszynaTrianglesImporter, RefCounted);

        protected:
            static void _bind_methods();

        public:
            static Array import_triangles(MaszynaParser *p_parser, const Vector3 &p_rotate, const Vector3 &p_origin);
    };
} // namespace godot
