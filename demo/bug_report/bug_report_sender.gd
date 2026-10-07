class_name BugReportSender
extends HTTPRequest

## Sends a problem report to where Project Setting maszyna/bugtracking/endpoint points.
##
## An http:// or https:// endpoint opens the GitHub issue - report API version API_VERSION: one
## multipart/form-data POST of text fields and one archive.
##   fields      "api_version", then the report's own: "title", "description", "build",
##               "scenery", "vehicle" (UTF-8 text; a field may be empty)
##   attachments ATTACHMENTS_FILE (application/zip): SNAPSHOT_FILE, and SCREENSHOT_FILE and the logs
##               (the session's app.log, the game's log files - BugReport) when there are any
## It answers JSON with the issue's "issue_url". The fields are enough to open the issue; the
## archive is the one file the issue links to.
##
## Anything else - user://, file:// or a plain path - is a directory: a new directory of the
## report's own in it gets REPORT_FILE (the fields as JSON) and the same ATTACHMENTS_FILE.
## Empty - the default, until the endpoint exists - is no reporting at all (is_available()).
##
## What leaves the machine names nobody: the snapshot and the logs go with the player's home
## directory and system user name masked (mask_personal()) - the logs write paths under the home,
## and a player with no nick is named by the user name. The report's own fields are what the player
## typed and go as they are.

signal report_sent(issue_url: String)
## The report was saved into a directory, not sent
signal report_saved(directory: String)
signal report_failed(message: String)

## Project Setting: the endpoint that opens the issues, or the directory reports are saved in
const ENDPOINT_SETTING: String = "maszyna/bugtracking/endpoint"
const FILE_SCHEME: String = "file://"
## What the endpoint is told it is sent - raised whenever the fields or the archive change
const API_VERSION: int = 1
const REPORT_FILE: String = "report.json"
const ATTACHMENTS_FILE: String = "report.zip"
const SNAPSHOT_FILE: String = "snapshot.json"
const SCREENSHOT_FILE: String = "screenshot.jpg"
## The archive of a report that is sent is made here, and gone once it is read
const PACKING_DIRECTORY: String = "user://bug_reports_outgoing"
const BOUNDARY_PREFIX: String = "MaSzynaReport"
const HTTP_OK_FIRST: int = 200
const HTTP_OK_LAST: int = 299
## The home directory: HOME on Linux and macOS, USERPROFILE on Windows
const HOME_VARIABLES: PackedStringArray = ["HOME", "USERPROFILE"]
## The system user name: USER on Linux and macOS, USERNAME on Windows
const USER_VARIABLES: PackedStringArray = ["USER", "USERNAME"]
const HOME_MASK: String = "~"
const USER_MASK: String = "<user>"
## What a regular expression reads as its own, escaped in a user name - the backslash first
const REGEX_SPECIAL_CHARACTERS: String = "\\.^$|?*+()[]{}-"


## Whether a report has anywhere to go
static func is_available() -> bool:
    return not ProjectSettings.get_setting(ENDPOINT_SETTING, "") == ""


## An empty screenshot or log is left out of the archive; the logs by their file names
func send(
        report: Dictionary, snapshot: String, screenshot: PackedByteArray,
        logs: Dictionary[String, PackedByteArray]) -> void:
    var endpoint: String = ProjectSettings.get_setting(ENDPOINT_SETTING)
    var files: Dictionary[String, PackedByteArray] = {
        SNAPSHOT_FILE: mask_personal(snapshot).to_utf8_buffer()
    }
    if screenshot:
        files[SCREENSHOT_FILE] = screenshot
    for log_file: String in logs:
        if logs[log_file]:
            files[log_file] = mask_personal(logs[log_file].get_string_from_utf8()).to_utf8_buffer()
    var fields: Dictionary = fields_of(report)
    if not endpoint.begins_with("http://") and not endpoint.begins_with("https://"):
        # a directory per report, named by when it was made
        var directory: String = endpoint.trim_prefix(FILE_SCHEME).path_join(
                Time.get_datetime_string_from_system().replace(":", "-"))
        var made: Error = DirAccess.make_dir_recursive_absolute(directory)
        if made == OK:
            made = pack(files, directory.path_join(ATTACHMENTS_FILE))
        if made == OK:
            var file: FileAccess = FileAccess.open(directory.path_join(REPORT_FILE), FileAccess.WRITE)
            if file:
                file.store_string(JSON.stringify(fields, "\t"))
            else:
                made = FileAccess.get_open_error()
        if not made == OK:
            report_failed.emit(error_string(made))
            return
        report_saved.emit(ProjectSettings.globalize_path(directory))
        return
    var archive_path: String = PACKING_DIRECTORY.path_join(ATTACHMENTS_FILE)
    var packed: Error = DirAccess.make_dir_recursive_absolute(PACKING_DIRECTORY)
    if packed == OK:
        packed = pack(files, archive_path)
    if not packed == OK:
        report_failed.emit(error_string(packed))
        return
    var archive: PackedByteArray = FileAccess.get_file_as_bytes(archive_path)
    DirAccess.remove_absolute(archive_path)
    var boundary: String = "%s%d" % [BOUNDARY_PREFIX, randi()]
    var error: Error = request_raw(
            endpoint, PackedStringArray(["Content-Type: multipart/form-data; boundary=" + boundary]),
            HTTPClient.METHOD_POST, build_body(boundary, fields, archive))
    if not error == OK:
        report_failed.emit(error_string(error))


## The text without the player's home directory - HOME_MASK, written with either slash - and
## system user name - USER_MASK, as a whole word wherever it stands
static func mask_personal(text: String) -> String:
    for variable: String in HOME_VARIABLES:
        var home: String = OS.get_environment(variable)
        if home:
            text = text.replace(home, HOME_MASK).replace(home.replace("\\", "/"), HOME_MASK)
    for variable: String in USER_VARIABLES:
        var user: String = OS.get_environment(variable)
        if not user:
            continue
        for special: String in REGEX_SPECIAL_CHARACTERS:
            user = user.replace(special, "\\" + special)
        text = RegEx.create_from_string("\\b%s\\b" % user).sub(text, USER_MASK, true)
    return text


## The text fields of a report: the API version, then the report's own, each as text
static func fields_of(report: Dictionary) -> Dictionary:
    var fields: Dictionary = {"api_version": str(API_VERSION)}
    for field: Variant in report:
        fields[field] = str(report[field])
    return fields


## The archive of the files at [param path]
static func pack(files: Dictionary[String, PackedByteArray], path: String) -> Error:
    var packer: ZIPPacker = ZIPPacker.new()
    var error: Error = packer.open(path)
    if not error == OK:
        return error
    for file_name: String in files:
        packer.start_file(file_name)
        packer.write_file(files[file_name])
        packer.close_file()
    return packer.close()


## The multipart/form-data body: every field as text, then the archive
static func build_body(
        boundary: String, fields: Dictionary, archive: PackedByteArray) -> PackedByteArray:
    var body: PackedByteArray = PackedByteArray()
    for field: Variant in fields:
        body.append_array(_part(
                boundary, str(field), "", "text/plain; charset=utf-8",
                str(fields[field]).to_utf8_buffer()))
    body.append_array(_part(boundary, "attachments", ATTACHMENTS_FILE, "application/zip", archive))
    body.append_array(("--%s--\r\n" % boundary).to_utf8_buffer())
    return body


static func _part(boundary: String, field: String, filename: String, content_type: String, content: PackedByteArray) -> PackedByteArray:
    var disposition: String = "form-data; name=\"%s\"" % field
    if filename:
        disposition += "; filename=\"%s\"" % filename
    var part: PackedByteArray = ("--%s\r\nContent-Disposition: %s\r\nContent-Type: %s\r\n\r\n" % [
        boundary, disposition, content_type
    ]).to_utf8_buffer()
    part.append_array(content)
    part.append_array("\r\n".to_utf8_buffer())
    return part


func _on_request_completed(result: int, response_code: int, _headers: PackedStringArray, body: PackedByteArray) -> void:
    if not result == HTTPRequest.RESULT_SUCCESS:
        report_failed.emit(tr("The report could not be sent (error %d)") % result)
        return
    if response_code < HTTP_OK_FIRST or response_code > HTTP_OK_LAST:
        report_failed.emit(tr("The report was refused (HTTP %d)") % response_code)
        return
    var response: Variant = JSON.parse_string(body.get_string_from_utf8())
    report_sent.emit(str(response.get("issue_url", "")) if response is Dictionary else "")
