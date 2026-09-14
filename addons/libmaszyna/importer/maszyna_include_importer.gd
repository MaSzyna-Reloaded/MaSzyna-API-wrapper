@tool
extends RefCounted

func import(p: MaszynaParser, context: MaszynaImporterContext):
    var tokens = p.get_tokens_until("end")
    tokens.pop_back()
    var filename = tokens.pop_front()
    var scenery_dir = UserSettings.get_maszyna_game_dir().path_join("scenery")
    var final_path = scenery_dir.path_join(filename)

    var file = FileAccess.open(final_path, FileAccess.READ)
    if not file:
        # Original assets assume Windows' case-insensitive filesystem (e.g. .scm files
        # referencing "EST-bramka770.inc" when the file on disk is "est-bramka770.inc") - fall
        # back to a case-insensitive lookup before giving up, and keep filename/final_path in
        # sync since parse_file() below re-resolves filename against scenery_dir itself.
        var resolved_path = _find_case_insensitive(scenery_dir, filename)
        if resolved_path:
            filename = resolved_path
            final_path = scenery_dir.path_join(filename)
            file = FileAccess.open(final_path, FileAccess.READ)
    var parameters = {}
    for i in range(tokens.size()):
        parameters["p%s" % (i+1)] = tokens[i]
    if file:
        context.push_state()
        context.include_depth += 1
        var objects = SceneryInstancer.parse_file(filename, parameters, context)
        context.pop_state()
        return objects
    else:
        context.cacheable = false
        push_error("Cannot load include file: " + final_path)
        return []


## Original MaSzyna assets assume a case-insensitive filesystem. Walks p_relative_path segment by
## segment under p_base_dir, matching each against the actual directory listing case-insensitively,
## and returns the path (relative to p_base_dir) as it actually exists on disk, or "" if no match.
func _find_case_insensitive(base_dir: String, relative_path: String) -> String:
    var segments = relative_path.split("/")
    var current_dir = base_dir
    var resolved_segments: Array[String] = []
    for segment in segments:
        var dir = DirAccess.open(current_dir)
        if not dir:
            return ""
        var match_name = ""
        dir.list_dir_begin()
        var entry = dir.get_next()
        while entry:
            if entry.to_lower() == segment.to_lower():
                match_name = entry
                break
            entry = dir.get_next()
        dir.list_dir_end()
        if not match_name:
            return ""
        resolved_segments.append(match_name)
        current_dir = current_dir.path_join(match_name)
    return "/".join(resolved_segments)
