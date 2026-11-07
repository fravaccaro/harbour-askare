#!/bin/sh
#
# Takes the translatable strings of the desktop file to the translators and
# brings their answers back into it. They cannot travel with the rest: a
# desktop file is read by the launcher and the sharing dialog rather than by
# the app, and a Harbour package may install no catalog where those would
# look for one, so each language needs a line of its own in the file.
#
#   scripts/desktop-translations.sh
#
# It rewrites translations/desktop/harbour-askare.pot from the template, brings
# every .po listed in LINGUAS up to date with it, and writes the desktop file
# the package installs. The result is committed, so that building the app
# needs no gettext: run this after editing harbour-askare.desktop.in or after
# pulling translations, and commit what it changes.
#
# Needs gettext: xgettext, msgmerge and msgfmt.
set -eu

cd "$(dirname "$0")/.."

template=harbour-askare.desktop.in
desktop=harbour-askare.desktop
podir=translations/desktop
pot=$podir/harbour-askare.pot

# Only the strings a user reads: the name of the app is a name, not a word.
# The lines opening "Translators:" above a key in the template are carried
# into the .pot as notes, since neither string says much on its own.
xgettext --language=Desktop -k -kComment -kDescription \
    --add-comments=Translators: --from-code=UTF-8 \
    --package-name=harbour-askare \
    --msgid-bugs-address=https://github.com/fravaccaro/harbour-askare/issues \
    --output="$pot" "$template"

# xgettext leaves the charset for the translator to fill in, and Transifex
# hands back UTF-8 whatever is asked of it.
sed -i 's/charset=CHARSET/charset=UTF-8/' "$pot"

while read -r lang; do
    [ -n "$lang" ] || continue
    msgmerge --quiet --update --backup=none "$podir/$lang.po" "$pot"
done < "$podir/LINGUAS"

msgfmt --desktop -k -kComment -kDescription \
    --template="$template" -d "$podir" --output-file="$desktop"

# The notes to the translators belong to the template, not to the file the
# launcher reads, and the file says where it came from so that an edit made
# in it rather than in the template is caught before the next run drops it.
{
    printf '%s\n' \
        '# Generated from harbour-askare.desktop.in by scripts/desktop-translations.sh.' \
        '# Edit the template or the translations under translations/desktop, never this file.'
    sed '/^#/d' "$desktop"
} > "$desktop.new"
mv "$desktop.new" "$desktop"
