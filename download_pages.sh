#!/bin/sh

mkdir -p data/{kanjis,img}
wget -c 'http://kanjidamage.com/kanji' -P data/

IFS=$'\n'
for link in $(grep '^<td><a href' data/kanji); do
    url=$(echo $link|sed 's/.*href="\([^"]*\)">.*/\1/')
    url="http://kanjidamage.com/$url"
    wget -c "$url" -P data/kanjis
    if echo $link|grep 'src="'>/dev/null; then
        img=$(echo $link|sed 's/.*src="\([^"]*\)".*/\1/')
        url="http://kanjidamage.com/$img"
        wget -c "$url" -P data/img
    fi
done
