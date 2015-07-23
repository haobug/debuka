#!/bin/bash
dir=$1
find $dir -type f -name "*.webp" |while read webp; do echo $webp && dwebp -o `filename $webp`.png $webp; done;
