#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "LogSystem.hpp"
#include "TrainNode.hpp"
#include "TrainPart.hpp"

namespace godot {

class TrainPartNode : public Node {
        GDCLASS(TrainPartNode, Node)

    public:
        TrainPartNode() = default;
        ~TrainPartNode() override = default;

        void set_train_part(const Ref<TrainPart> &p_part);
        Ref<TrainPart> get_train_part() const;

        TrainNode *get_train_node() const;
        Dictionary get_train_state() const;
        void log(LogSystem::LogLevel level, const String &line) const;
        void log_debug(const String &line) const;
        void log_info(const String &line) const;
        void log_warning(const String &line) const;
        void log_error(const String &line) const;

    protected:
        static void _bind_methods();
        void _notification(int p_what);

    private:
        Ref<TrainPart> train_part;
        bool added_to_entity{false};

        void _attach_to_train();
        TrainNode *_find_train_node() const;
};

} // namespace godot
