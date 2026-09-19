@tool
extends RefCounted

func import(p: MaszynaParser, context: MaszynaImporterContext):
    var tokens = p.get_tokens_until("end")
    tokens.pop_back()
    var filename = resolve_filename(tokens.pop_front())
    var final_path = UserSettings.get_maszyna_game_dir().path_join("scenery").path_join(filename)
    var file = FileAccess.open(final_path, FileAccess.READ)
    var parameters = {}
    for i in range(tokens.size()):
        parameters["p%s" % (i+1)] = tokens[i]
    if file:
        context.push_state()
        context.include_depth += 1
        if context.defer_includes:
            # SceneryInstancer continues in the included file and pops the state when it ends
            context.pending_include_parser = SceneryInstancer.open_parser(filename, parameters, context)
            if context.pending_include_parser:
                context.pending_include_filename = filename
                p.interrupt()
            else:
                context.pop_state()
            return []
        var objects = SceneryInstancer.parse_file(filename, parameters, context)
        context.pop_state()
        return objects
    else:
        context.cacheable = false
        push_error("Cannot load include file: " + final_path)
        return []


## Include path relative to scenery/, as it exists on disk. Original assets assume Windows'
## case-insensitive filesystem (e.g. .scm files referencing "EST-bramka770.inc" when the file on
## disk is "est-bramka770.inc") - falls back to a case-insensitive lookup; returns filename
## unchanged when nothing matches. Also used by SceneryInstancer's include prescan.
func resolve_filename(filename: String) -> String:
    var scenery_dir: String = UserSettings.get_maszyna_game_dir().path_join("scenery")
    if FileAccess.file_exists(scenery_dir.path_join(filename)):
        return filename
    var resolved_path: String = _find_case_insensitive(scenery_dir, filename)
    return resolved_path if resolved_path else filename


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
