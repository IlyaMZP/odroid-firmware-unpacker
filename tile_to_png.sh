#!/usr/bin/env bash
ffmpeg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s 86x48 -i tile.raw -f image2 -vcodec png tile.png