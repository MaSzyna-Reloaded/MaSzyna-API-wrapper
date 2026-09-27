#pragma once
#include "VehicleComponentModel.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {
    /* A parsed vehicle: its own configuration and the components it is made of.
     *
     * What E3DModel is to an .e3d, this is to a .fiz - the parse result, cached on disk and
     * instanced as often as a scenery needs it. What is built from it is the instancer's business:
     * the vehicle's own objects today, proxy nodes for the editor alongside them later, the same
     * way E3DModel feeds two backends. */
    class VehicleModel : public Resource {
            GDCLASS(VehicleModel, Resource)

        private:
            Dictionary properties;
            TypedArray<VehicleComponentModel> components;

        protected:
            static void _bind_methods();

        public:
            /* Bumped whenever what is stored here changes shape, so the disk cache written by an
             * older parser is not read back - E3DModel::FORMAT_VERSION's own reason. */
            static const int FORMAT_VERSION = 1;

            void set_properties(const Dictionary &p_properties);
            Dictionary get_properties() const;

            void set_components(const TypedArray<VehicleComponentModel> &p_components);
            TypedArray<VehicleComponentModel> get_components() const;

            /* Everything an object carries that is worth storing - its authored configuration.
             * Live state is read-only and never has STORAGE usage, so it stays out by
             * construction. */
            static Dictionary capture(Object *p_object);
            static void apply(Object *p_object, const Dictionary &p_properties);
    };
} // namespace godot
