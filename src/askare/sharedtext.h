#ifndef ASKARE_SHAREDTEXT_H
#define ASKARE_SHAREDTEXT_H

#include <QString>

namespace SharedText {

// Text as it arrived from whichever app sent it, with the line endings of
// wherever it was written made into the ones a task is stored with.
QString body(const QString &text);

// The line that names a task made out of shared text, which is the first one
// it has. A task name is one line, so a name is never more than that.
QString firstLine(const QString &text);

// A shared address, or an empty string when nothing in a task could have
// followed it, which is what tells the caller there was nothing worth keeping.
QString address(const QString &url);

}

#endif
