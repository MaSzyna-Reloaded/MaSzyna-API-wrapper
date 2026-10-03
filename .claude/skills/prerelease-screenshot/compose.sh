#!/usr/bin/env bash
# Composes a pre-release screenshot: the loading screen logo on the left and the build number
# on the right, both on one line. Usage:
#   compose.sh <input.png> <build> <output.png> [line_top_y] [label]
# line_top_y - top of the build number in 1080p pixels (default 760); the logo is centred on it.
# label      - eyebrow above the number (default "PRE-ALPHA BUILD").
set -euo pipefail

INPUT="$1"
BUILD="$2"
OUTPUT="$3"
LINE_TOP="${4:-760}"
LABEL="${5:-PRE-ALPHA BUILD}"

REPO="$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
LOGO="$REPO/demo/loading_screen/logo.png"
FONT_LABEL=/usr/share/fonts/noto/NotoSans-SemiBold.ttf
FONT_BUILD=/usr/share/fonts/noto/NotoSans-Black.ttf
# Colors of loading_board.tscn: Eyebrow and Scenery labels
COLOR_LABEL='#73b8ff'
COLOR_BUILD='#e6edfc'

# Every size is designed for 1920x1080 and scaled to the input's width
WIDTH=$(magick identify -format '%w' "$INPUT")
HEIGHT=$(magick identify -format '%h' "$INPUT")
s() { awk -v v="$1" -v w="$WIDTH" 'BEGIN { printf "%d", v * w / 1920 }'; }

MARGIN=$(s 44)
LOGO_WIDTH=$(s 400)
BUILD_SIZE=$(s 96)
LABEL_SIZE=$(s 22)
LINE_Y=$(s "$LINE_TOP")
LABEL_Y=$((LINE_Y - $(s 25)))
# The trimmed logo is 400x97; its centre sits on the centre of the digits
LOGO_Y=$((LINE_Y - $(s 15)))
SHADOW_Y=$((LINE_Y + $(s 50)))

magick "$INPUT" \
  \( -size "${WIDTH}x${HEIGHT}" xc:none -fill 'rgba(0,0,0,0.6)' \
     -draw "ellipse $(s 220),$SHADOW_Y $(s 460),$(s 180) 0,360" \
     -draw "ellipse $((WIDTH - $(s 320))),$SHADOW_Y $(s 480),$(s 160) 0,360" \
     -blur "0x$(s 110)" \) -composite \
  \( "$LOGO" -trim +repage -resize "${LOGO_WIDTH}x" \) -geometry "+$MARGIN+$LOGO_Y" -composite \
  -gravity northeast \
  -font "$FONT_LABEL" -fill "$COLOR_LABEL" -pointsize "$LABEL_SIZE" -kerning "$(s 3)" \
    -annotate "+$((MARGIN + $(s 4)))+$LABEL_Y" "$LABEL" \
  -font "$FONT_BUILD" -fill "$COLOR_BUILD" -pointsize "$BUILD_SIZE" -kerning 0 \
    -annotate "+$MARGIN+$LINE_Y" "$BUILD" \
  "$OUTPUT"
echo "$OUTPUT"
