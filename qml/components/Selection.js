/*!
    Helpers shared by the controls that let several options be on at once.

    Both work on plain arrays and answer new ones rather than changing the one
    they are given, so the bindings that read a selection follow it.
*/
.pragma library

function has(values, value) {
    return values.indexOf(value) >= 0
}

// values without value if it was among them, with it appended if it was not.
function toggled(values, value) {
    var out = []
    for (var i = 0; i < values.length; i++) {
        if (values[i] !== value)
            out.push(values[i])
    }
    if (!has(values, value))
        out.push(value)
    return out
}
