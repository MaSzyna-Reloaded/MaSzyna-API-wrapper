@tool
## Minimal DDS (DXT1/DXT3/DXT5) loader that can discard the top mipmap levels of an
## oversized texture before it ever reaches the GPU - port of the original engine's own
## texture size clamp (Texture.cpp's CurrentMaxTextureSize mip-skip loop, Globals.h's
## iMaxTextureSize/maxtexturesize .ini option), which this wrapper had no equivalent of:
## MaterialManager.load_texture() used to hand every .dds straight to Godot's own loader
## at full native resolution with no way to cap VRAM use on a weaker GPU.


## Returns null on anything this loader doesn't handle (bad magic, unsupported fourCC,
## a max_size too small to keep any mip level) - callers should fall back to load(path).
static func load_texture(path:String, max_size:int) -> Texture2D:
    var file:FileAccess = FileAccess.open(path, FileAccess.READ)
    if not file:
        return null

    if file.get_buffer(4).get_string_from_ascii() != "DDS ":
        return null

    file.get_32()  # header size
    file.get_32()  # flags
    var height:int = file.get_32()
    var width:int = file.get_32()
    file.get_32()  # pitch_or_linear_size
    file.get_32()  # depth
    var mipmap_count:int = maxi(file.get_32(), 1)
    file.seek(file.get_position() + 44)  # reserved1[11]

    file.get_32()  # pixelformat.size
    file.get_32()  # pixelformat.flags
    var fourcc:String = file.get_buffer(4).get_string_from_ascii()

    var format:Image.Format
    var block_size:int
    match fourcc:
        "DXT1":
            format = Image.FORMAT_DXT1
            block_size = 8
        "DXT3":
            format = Image.FORMAT_DXT3
            block_size = 16
        "DXT5":
            format = Image.FORMAT_DXT5
            block_size = 16
        _:
            return null

    var data_start:int = 4 + 124  # magic + header
    var file_size:int = file.get_length()
    var offset:int = 0
    while (width > max_size or height > max_size) and mipmap_count > 1:
        offset += ((width + 3) / 4) * ((height + 3) / 4) * block_size
        width /= 2
        height /= 2
        mipmap_count -= 1

    var data_size:int = file_size - data_start - offset
    if data_size <= 0:
        return null

    file.seek(data_start + offset)
    var data:PackedByteArray = file.get_buffer(data_size)

    var image:Image = Image.create_from_data(width, height, mipmap_count > 1, format, data)
    if not image:
        return null
    return ImageTexture.create_from_image(image)
