---
layout: page
title: "Problem Reports"
---

The demo game lets a player report a problem or a suggestion from the scenery selector (the bug
button at the right edge of the screen) and from a running scenery (the same button, or "Report a
problem or suggestion" in the top bar). The form pauses the simulation, takes a screenshot the
player can mark with red rectangles, collects a snapshot of the game's state and - once the player
has agreed to publish it - hands it to the **reporting endpoint**. The endpoint is a small server,
not part of this repository, that opens a GitHub issue from it: a Cloudflare Worker in the
`MaSzyna-Reloaded/reports` repository (`worker/`), which opens the issue in that repository and
keeps `report.zip` as an asset of the month's release (`reports-YYYY-MM`).

This page is the contract between the game (`demo/bug_report/`) and that server.

## Where reports go

Project Setting `maszyna/bugtracking/endpoint` (`demo/project.godot`, section `[maszyna]`):

| Value | What the game does |
|---|---|
| `http://...` or `https://...` | sends the report to the server - one `POST`, described below |
| anything else: `user://...`, `file:///...`, a plain path | saves the report into that directory |
| empty | opens no form: a notice says reporting is not available in this version |

The default is empty - until the server exists, the game does not pretend to send anything. For
testing, a directory (`user://bug_reports`) saves the reports locally; on Linux that is
`~/.local/share/MaSzyna-Reloaded/bug_reports/` (the project's custom user directory).

A saved report is a directory of its own, named by the local time it was made
(`2026-10-04T15-30-12/`), with what the server receives:

* `report.json` - the text fields, as one JSON object (indented)
* `report.zip` - the `attachments` part

## The request

Report API version **1**. One `POST` to the endpoint URL, `multipart/form-data`: the report's
metadata as text fields, and every file in one zip archive. Godot's `HTTPRequest` adds `Host`,
`User-Agent: GodotEngine/...` and `Content-Length`; the game adds only the content type:

```
POST /<endpoint path> HTTP/1.1
Content-Type: multipart/form-data; boundary=MaSzynaReport1849302847
```

The boundary is `MaSzynaReport` followed by a random number, new for every request. Lines of the
multipart framing end with `\r\n`. The parts come in this order:

| Part (`name`) | `filename` | `Content-Type` | Always |
|---|---|---|---|
| `api_version` | - | `text/plain; charset=utf-8` | yes - `1` |
| `title` | - | `text/plain; charset=utf-8` | yes |
| `description` | - | `text/plain; charset=utf-8` | yes |
| `build` | - | `text/plain; charset=utf-8` | yes |
| `scenery` | - | `text/plain; charset=utf-8` | yes, may be empty |
| `vehicle` | - | `text/plain; charset=utf-8` | yes, may be empty |
| `attachments` | `report.zip` | `application/zip` | yes |

```
--MaSzynaReport1849302847
Content-Disposition: form-data; name="api_version"
Content-Type: text/plain; charset=utf-8

1
--MaSzynaReport1849302847
Content-Disposition: form-data; name="title"
Content-Type: text/plain; charset=utf-8

[20261003-1708] Stary Jawór / SU46-048: the doors do not open
...
--MaSzynaReport1849302847
Content-Disposition: form-data; name="attachments"; filename="report.zip"
Content-Type: application/zip

<zip bytes>
--MaSzynaReport1849302847--
```

`api_version` is raised whenever the fields or the archive change, so a server can tell which
shape it was sent. The fields are enough to open the issue without unpacking anything; the archive
is the one file the issue links to.

### The fields

Every value is UTF-8 text:

| Field | Value |
|---|---|
| `title` | the issue's title: `[<build>] <scenery title> / <vehicle>: <description's first line, 80 characters at most>`; in the selector, without a scenery or a vehicle, `[<build>] <description's first line>` |
| `description` | what the player wrote, whole, trimmed; never empty |
| `build` | the build number of the game (`GameDataServer.build_get_number()`, e.g. `20261003-1708`) |
| `scenery` | the scenery's file (`stary_jawor_eszelon.scn`), empty in the selector |
| `vehicle` | the vehicle the player drives (its scenery name), empty when none |

The game sends a report only after the player ticked "I agree to sharing the hardware information
and the report, and to publishing them on GitHub".

### `attachments` - `report.zip`

| File | Contents | Always |
|---|---|---|
| `snapshot.json` | the state of the game, below | yes |
| `screenshot.jpg` | the screen when the form opened, with the player's red marks, JPEG at quality 75 | only when attached |
| `app.log` | the end of this session's log (the engine's file log, `debug/file_logging/log_path`), its last 4 MB; its first lines name the build (`[GameDataServer] Build <number>`) | only while the file logging is on (the Debug settings) - with it off the file would be an earlier session's |
| `gameplay.log` | the scenery's gameplay log (`BugReportRecorder`), its last 50 MB - lines below | when a scenery ran |

#### `gameplay.log`

From the scenery's start, one line per happening: `<time> <simulation time> <kind> <subject>
<details>`, the subject a name and its RID (`SN61-02#2`), the details `key=value`:

```
07:16:42 0.000 player marcin#3 present
07:16:42 0.012 person SN61-02#812 created
07:16:42 0.012 ai SN61-02#812 attached
07:16:42 0.012 ai SN61-02#812 entered vehicle=SN61-02#2 cab=rear role=driver
07:16:42 0.020 vehicle SN61-02#2 created
07:16:42 0.021 trainset SN61-02#2 vehicles=SN61-02#2,bdhpumn_1#4412
07:16:45 2.904 ai SN61-02#812 role=observer vehicle=SN61-02#2 cab=rear
07:16:45 2.904 player marcin#3 entered vehicle=SN61-02#2 cab=rear role=driver
07:17:20 37.640 player Marcin#3 renamed previous=marcin
07:17:33 50.281 command SN61-02#2 direction_increase p1=<null> p2=<null>
07:17:49 66.631 command SN61-02#2 local_brake_set p1=0.016 p2=<null> repeats=60 until=67.631 last=1.0,<null>
07:21:45 108.186 event wylacz_wilis_hi activator=SP42-260
08:16:58 1758.119 scenery left
```

* `player`, `ai`, `person` - a person by its kind: the player (named by its nick, Settings -
  Player; without one the system's user name, else `unnamed`), an AI driver (named after its
  vehicle), a person neither yet; `present` (the player, made with the game), `created`, `freed`,
  `attached` (as an AI driver), `renamed previous=`; `entered`/`left vehicle= cab=` with its
  `role=`, `moved` to a cab `from_vehicle= from_cab=`, `role=` changed in a cab;
* `vehicle` - `created` (in the simulation from its configuration on) or `freed`;
* `trainset` - the vehicles coupled together, in their order, whenever they change;
* `command` - a command to any vehicle (the player's, an AI driver's, the scenario's - it carries no
  sender); a command repeated without a pause is one line with `repeats=`, the last one's
  `until=` simulation time and `last=` values;
* `event` - a launched event and the vehicle that set it off;
* `scenery left` when the player leaves the scenery, `game closed` when the game quits with it
  running - the last line either way.

### `snapshot.json`

The state of the game at the moment the form was opened, as JSON (tabs for indentation). Vectors
are arrays (`[x, y, z]`), positions are in metres in the scenery's coordinates, times are seconds.
Logs are arrays of text lines, oldest first, fields separated by single spaces.

| Section | Contents |
|---|---|
| `hardware` | `build`, `godot` (engine version), `os`, `os_version`, `distribution`, `architecture`, `debug`, `editor`, `double_precision`, `cpu`, `cpu_threads`, `memory` (`OS.get_memory_info()`, bytes), `gpu_vendor`, `gpu`, `gpu_type` (`DEVICE_TYPE_DISCRETE_GPU`, ...), `gpu_driver`, `graphics_api_version`, `rendering_method`, `rendering_driver`, `screen_size`, `window_size`, `window_mode` (`WINDOW_MODE_FULLSCREEN`, ...), `locale`, `language` |
| `scenario` | `vehicle`, `occupied_cab`; with a scenery also `filename`, `title`, `start_time` (hours), `first_train_id` |
| `simulation` | `real_seconds_since_start` (wall clock since the scenery started), `simulation_time`, `time_of_day` (hours), `speed` (asked), `current_speed`, `light_level`; with a scenery also `date` (`YYYY-MM-DD`) and `weather`: `weather` (`WEATHER_RAIN`, ...), `season`, `cloudiness`, `precipitation`, `temperature` (the scenery's, °C), `air_temperature` (the simulation's, °C), `wind_direction` (°), `wind_strength`, `fog_enabled`, `fog_density`, `fog_distance` (m) |
| `camera` | `mode` (`CAMERA_MODE_CABIN`, ...), `target` (vehicle name), `follow_view`, `vehicle_transform` and, with a scenery, `transform` - each `{"origin": [x, y, z], "rotation_radians": [x, y, z]}` |
| `streaming` | `enabled`, `building`, `camera_position`, `camera_chunk`, `draw_distance`, `statistics` (`SceneryStreamingServer.streaming_get_statistics()`) |
| `vehicles` | `trainset`: every vehicle coupled to the player's, `{"name", "position", "state", "config"}` - the full state and configuration dumps; `nearby`: every other vehicle within 2000 m, `{"name", "position", "distance", "state"}` |
| `signal_heads` | lines, nearest first, every signal head within 2000 m (see below) |
| `events` | `launched`: lines, the last 200 events that ran (see below); `upcoming`: the queued events in the order they run, `{"event", "seconds_left"}` |
| `commands` | lines, the last 500 commands the player's vehicle received (see below) |

The dumps in `vehicles` are the bulk of the snapshot - a trainset of a few vehicles makes it a few
hundred kilobytes, so the server should expect a snapshot of up to a few megabytes.

#### Log lines

`commands` - `<time> <simulation time> <vehicle> <command> <p1> <p2>`:

```
14:32:05 1234.567 SU46-048 mainswitch_on 1 null
```

`<time>` is the player's local wall clock, `<simulation time>` the seconds the simulation has run.
A parameter the command does not take is `null`. A command carries no sender: the scenario's
commands to the player's vehicle are logged as well as the player's own.

`events.launched` - `<time> <simulation time> <event> <activator>`:

```
14:32:05 0.018 jawor_wk1_zamknij_onstart
14:33:40 95.240 sem_a1_sp1 SU46-048
```

`<activator>` is the vehicle that set the event off, missing for an event nothing set off.

`signal_heads` - `<distance> <name> <aspect> <x>,<y>,<z> <lights>`:

```
240.3 jawor_a S2 20412.0,0.0,19102.5 O.*..
850.1 Roztocze_E1 - 20271.7,0.0,19540.7 .....
```

`<aspect>` is `-` while no event has set one. `<lights>` is one character per light of the head, in
its order: `.` off, `O` on, `*` blinking.

### Response

| Response | What the game does |
|---|---|
| `2xx` with a JSON body `{"issue_url": "https://github.com/..."}` | the form closes and a confirmation shows the link to the issue; the simulation resumes once the player closes it (OK) |
| `2xx` without `issue_url` | the form closes and the simulation resumes |
| any other status | the form stays open: "The report was refused (HTTP <status>)" |
| no connection, TLS failure, timeout | the form stays open: "The report could not be sent (error <HTTPRequest.Result>)" |

A failed report keeps what the player wrote, so it can be sent again.

## What the server has to do

* **Hold the GitHub credentials.** The game has no secret; anything put in an exported build can be
  read out of it. The Worker holds the private key of a GitHub App (`maszyna-reloaded-bug-tracker`)
  installed on the reports repository only, with `Issues: write` and `Contents: write` (the
  release asset), and opens the issues as the app's bot; the repository is the server's
  configuration, not the game's.
* **Open the issue** with the `title` field as its title, and a body made of `description`, the
  build, scenery and vehicle, the hardware summary and links to the attachments.
* **Store the archive itself.** GitHub's REST API cannot attach a file to an issue, so the server
  keeps `report.zip` (its own storage, a release asset) and links it from the issue. It may unpack
  it to show the screenshot in the issue's body or to summarise the snapshot's `hardware` - an issue
  body is limited to 65536 characters, the snapshot does not fit in it.
* **Limit abuse:** a rate limit per address, a size limit on the request (the archive: a snapshot
  of a few MB and a log of up to 4 MB, which pack small, and a JPEG of a few hundred kB), and a
  refusal of a request with an unknown `api_version`, without `attachments` or with an empty
  `description`.
* **Answer** `2xx` with `{"issue_url": ...}` once the issue exists.

To test a server without the game, a request of the same shape:

```bash
curl -F api_version=1 -F 'title=[test] a title' -F 'description=a test' -F build=test \
     -F scenery= -F vehicle= \
     -F 'attachments=@report.zip;type=application/zip' \
     https://<endpoint>
```

The files of a report the game saved locally (an endpoint set to `user://bug_reports`, then
`user://bug_reports/<time>/`) serve as test data.
