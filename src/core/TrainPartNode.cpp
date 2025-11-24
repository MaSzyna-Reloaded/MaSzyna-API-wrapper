#include "TrainPartNode.hpp"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void TrainPartNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_train_part", "train_part"), &TrainPartNode::set_train_part);
    ClassDB::bind_method(D_METHOD("get_train_part"), &TrainPartNode::get_train_part);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "train_part", PROPERTY_HINT_RESOURCE_TYPE, "TrainPart"),
                 "set_train_part", "get_train_part");

    ClassDB::bind_method(D_METHOD("get_train_node"), &TrainPartNode::get_train_node);
    ClassDB::bind_method(D_METHOD("get_train_state"), &TrainPartNode::get_train_state);
    ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &TrainPartNode::log);
    ClassDB::bind_method(D_METHOD("log_debug", "line"), &TrainPartNode::log_debug);
    ClassDB::bind_method(D_METHOD("log_info", "line"), &TrainPartNode::log_info);
    ClassDB::bind_method(D_METHOD("log_warning", "line"), &TrainPartNode::log_warning);
    ClassDB::bind_method(D_METHOD("log_error", "line"), &TrainPartNode::log_error);
}

void TrainPartNode::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY:
            _attach_to_train();
            break;
        case NOTIFICATION_EXIT_TREE:
            added_to_entity = false;
            break;
        default:
            break;
    }
}

void TrainPartNode::_attach_to_train() {
    if (train_part.is_null()) {
        UtilityFunctions::push_warning("TrainPartNode has no TrainPart assigned");
        return;
    }

    TrainNode *train_node = _find_train_node();
    if (train_node == nullptr) {
        UtilityFunctions::push_warning("TrainPartNode could not find parent TrainNode");
        return;
    }

    train_part->set_train_node(train_node);
    // add component to entity
    train_node->add_component(train_part);

    // update mover/config once on attach
    train_part->update_mover();
    added_to_entity = true;
}

TrainNode *TrainPartNode::_find_train_node() const {
    Node *p = get_parent();
    while (p != nullptr) {
        if (auto *tn = Object::cast_to<TrainNode>(p)) {
            return tn;
        }
        p = p->get_parent();
    }
    return nullptr;
}

void TrainPartNode::set_train_part(const Ref<TrainPart> &p_part) {
    train_part = p_part;
}

Ref<TrainPart> TrainPartNode::get_train_part() const {
    return train_part;
}

TrainNode *TrainPartNode::get_train_node() const {
    return _find_train_node();
}

Dictionary TrainPartNode::get_train_state() const {
    if (train_part.is_null()) { return {}; }
    return train_part->get_mover_state();
}

void TrainPartNode::log(const LogSystem::LogLevel level, const String &line) const {
    if (train_part.is_null()) { return; }
    train_part->log(level, line);
}

void TrainPartNode::log_debug(const String &line) const { log(LogSystem::LogLevel::DEBUG, line); }
void TrainPartNode::log_info(const String &line) const { log(LogSystem::LogLevel::INFO, line); }
void TrainPartNode::log_warning(const String &line) const { log(LogSystem::LogLevel::WARNING, line); }
void TrainPartNode::log_error(const String &line) const { log(LogSystem::LogLevel::ERROR, line); }

} // namespace godot
