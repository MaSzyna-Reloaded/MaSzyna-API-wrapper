@tool
extends Node

## A missing sound plays this instead: a vehicle whose data lacks a file stays quiet there rather
## than failing, as the original only logs it (audio.cpp:147). A tenth of a second of 16-bit mono.
const SILENCE_MIX_RATE:int = 44100
const SILENCE_BYTES:int = 8820

## An Ogg page: "OggS", version, header type, then the granule position at this offset (RFC 3533)
const OGG_CAPTURE_PATTERN:String = "OggS"
const OGG_GRANULE_OFFSET:int = 6
const OGG_SEGMENT_COUNT_OFFSET:int = 26
const OGG_PAGE_HEADER_SIZE:int = 27
## The largest page: its header, 255 lacing values and 255 segments of 255 bytes
const OGG_MAX_PAGE_SIZE:int = 65307
## The last page is looked for in this much of the end of the file first - it is rarely longer
const OGG_TAIL_SIZE:int = 8192
## The Vorbis identification header (the first packet): type 1, "vorbis", version, channels, then
## the sample rate at this offset (Vorbis I specification, 4.2.2)
const VORBIS_SAMPLE_RATE_OFFSET:int = 12
const VORBIS_SAMPLE_RATE_SIZE:int = 4

var _silence:AudioStreamWAV = AudioStreamWAV.new()


func _init() -> void:
    _silence.format = AudioStreamWAV.FORMAT_16_BITS
    _silence.mix_rate = SILENCE_MIX_RATE
    var data:PackedByteArray = PackedByteArray()
    data.resize(SILENCE_BYTES)
    _silence.data = data


func get_stream(name:String, loop:bool = false) -> AudioStream:
    var project_data_dir:String = UserSettings.get_maszyna_game_dir()
    var sounds_dir:String = project_data_dir.path_join("sounds")
    var full_path:String = sounds_dir.path_join(MaszynaDataPath.resolve(sounds_dir, name + ".ogg"))
    if not ResourceLoader.exists(full_path):
        push_warning("[%s] file does not exist: %s" % [self, full_path])
        return _silence
    var stream:AudioStreamOggVorbis = load(full_path)  # uses godot's builtin resource cache
    if stream and not stream.loop == loop:
        stream = stream.duplicate(0)
        stream.loop = loop
    return stream


## The duration [s] of a sound of the game, read off its Ogg pages without loading or decoding it:
## the samples the last page ends at over the sample rate of the identification header. 0.0 when
## the file is missing or is no Ogg Vorbis - silently, as an unknown length is an expected outcome
## where it is asked for (a voice's start placed before the file is read).
func get_stream_length(name:String) -> float:
    if not name:
        return 0.0
    var sounds_dir:String = UserSettings.get_maszyna_game_dir().path_join("sounds")
    var file:FileAccess = FileAccess.open(
            sounds_dir.path_join(MaszynaDataPath.resolve(sounds_dir, name + ".ogg")), FileAccess.READ)
    if not file:
        return 0.0
    var first_page:PackedByteArray = file.get_buffer(OGG_PAGE_HEADER_SIZE)
    if first_page.size() < OGG_PAGE_HEADER_SIZE \
            or not first_page.slice(0, OGG_CAPTURE_PATTERN.length()).get_string_from_ascii() == OGG_CAPTURE_PATTERN:
        return 0.0
    file.seek(OGG_PAGE_HEADER_SIZE + first_page[OGG_SEGMENT_COUNT_OFFSET])
    var identification:PackedByteArray = file.get_buffer(VORBIS_SAMPLE_RATE_OFFSET + VORBIS_SAMPLE_RATE_SIZE)
    var sample_rate:int = identification.decode_u32(VORBIS_SAMPLE_RATE_OFFSET)
    if sample_rate <= 0:
        return 0.0
    for tail_size:int in [OGG_TAIL_SIZE, OGG_MAX_PAGE_SIZE + OGG_PAGE_HEADER_SIZE]:
        var start:int = maxi(file.get_length() - tail_size, 0)
        file.seek(start)
        var tail:PackedByteArray = file.get_buffer(file.get_length() - start)
        var at:int = tail.rfind(OGG_CAPTURE_PATTERN.unicode_at(0), tail.size() - OGG_PAGE_HEADER_SIZE)
        while at >= 0:
            if tail.slice(at, at + OGG_CAPTURE_PATTERN.length()).get_string_from_ascii() == OGG_CAPTURE_PATTERN:
                var granule:int = tail.decode_s64(at + OGG_GRANULE_OFFSET)
                # -1: no packet ends on this page
                if granule >= 0:
                    return float(granule) / sample_rate
            at = tail.rfind(OGG_CAPTURE_PATTERN.unicode_at(0), at - 1) if at > 0 else -1
        if start == 0:
            break
    return 0.0
