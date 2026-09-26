extends SemaphoreSystemDelegate

## Demo automatic block: every `next` event moves each semaphore of the system to the next aspect
## its kind declares (sbl_3.tres: S1, S5, S2, S3), and back to the first after the last.

const NEXT_EVENT: StringName = &"next"


func _handle_event(system: RID, event: StringName, _arguments: Dictionary) -> void:
    if not event == NEXT_EVENT:
        return
    for semaphore: RID in SemaphoreServer.system_get_semaphores(system):
        var aspects: PackedStringArray = SemaphoreServer.semaphore_get_aspects(semaphore)
        if not aspects:
            continue
        var current: int = aspects.find(SemaphoreServer.semaphore_get_aspect(semaphore))
        SemaphoreServer.semaphore_set_aspect(semaphore, aspects[(current + 1) % aspects.size()])
