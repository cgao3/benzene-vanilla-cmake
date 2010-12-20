FILES="\
favicon.ico \
benzene-screenshot.html \
benzene-screenshot.png \
benzene-screenshot-thumb.png \
index.html \
"

echo "Enter user id for SourceForge:"
read NAME

scp $FILES $NAME,benzene@web.sourceforge.net:/home/groups/b/be/benzene/htdocs
