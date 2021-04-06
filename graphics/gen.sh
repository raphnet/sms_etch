#!/bin/sh -x

png2tile main.png -pal sms -binary -savepalette main.pal -savetiles main.tiles -savetilemap main.tilemap -savetileimage main.tileimage.png
png2tile sprites.png -pal sms -binary -noremovedupes -nomirror -savepalette sprites.pal -savetiles sprites.tiles

