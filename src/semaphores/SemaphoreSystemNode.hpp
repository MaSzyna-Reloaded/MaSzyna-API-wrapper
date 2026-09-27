#pragma once
#include "SemaphoreServer.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    /// A SemaphoreServer system in a scene: creates it on entering the tree with the delegate
    /// set here and groups the semaphores listed by name (also those registered later). A scenery
    /// creates its systems on the server directly; this node is for hand-built scenes.
    class SemaphoreSystemNode : public Node {
            GDCLASS(SemaphoreSystemNode, Node)

        private:
            Ref<SemaphoreSystemDelegate> delegate;
            PackedStringArray semaphore_names;
            RID system;

            void _on_semaphore_registered(const RID &p_semaphore, const StringName &p_name);

        protected:
            static void _bind_methods();
            void _notification(int p_what);

        public:
            void set_delegate(const Ref<SemaphoreSystemDelegate> &p_delegate);
            Ref<SemaphoreSystemDelegate> get_delegate() const;
            void set_semaphore_names(const PackedStringArray &p_names);
            PackedStringArray get_semaphore_names() const;

            RID get_system() const;
            void send_event(const StringName &p_event, const Dictionary &p_arguments);
    };
} // namespace godot
