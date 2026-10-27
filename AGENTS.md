# Agent Guidelines & Engineering Standards

This document defines architectural boundaries, coding conventions, and workflow rules for AI agents and contributors working on this Qt/QML codebase. All changes must strictly adhere to these guidelines.

---

## 1. Single Responsibility & Function Granularity

Write small, deterministic functions that perform exactly one task.

- **One purpose per unit:** A function must either transform data, query state, or trigger an action—never all three simultaneously.
- **Function size:** Keep C++ functions under 25 lines and QML/JS functions under 15 lines. If a function requires branching across multiple logical stages, decompose it into private helpers.
- **C++ Purity:** Prefer pure free functions (in anonymous namespaces or dedicated utility namespaces) or `const` member methods over state-mutating routines. Avoid hidden side effects.
- **Declarative first in QML:** Do not use JavaScript functions in QML for behavior that can be expressed declaratively with state bindings, `Binding`, or property expressions.

---

## 2. Reuse & Dependency Discipline

Never re-implement existing logic. Search the repository before adding new helpers or algorithms.

- **Check core utilities first:** Review existing utility modules, shared helpers, and base models before writing new logic.
- **Leverage Qt APIs:** Prefer standard Qt primitives (`QStringView`, `QDir`, `QUrl`, `QRegularExpression`, `QtConcurrent`) and standard library algorithms (`<algorithm>`) over custom parsing or filtering loops.
- **DRY enforcement:** If identical logic appears in two places, extract it into a shared utility class/function or a reusable QML component immediately.
- **Minimal coupling:** New helpers must accept the narrowest required type (e.g., pass `QStringView` or `const QUrl&` rather than entire configuration objects).

---

## 3. Code Style & Conventions

Follow official Qt and QML style guides consistently across the entire project.

### C++ Standards
- **Naming:**
  - Classes and Structs: `PascalCase`
  - Functions and Methods: `camelCase`
  - Member variables: `m_camelCase`
  - Constants and Enums: `PascalCase` or `kCamelCase`
- **Modern C++:** Target C++11, matching the Sailfish OS Qt 5.6 toolchain. Prefer `auto` only when the concrete type is explicitly repeated on the right side of the assignment. Do not use `QStringView`.
- **Memory & Object Trees:**
  - Strictly use `QObject` parent-child hierarchy for ownership where applicable.
  - Use smart pointers (`std::unique_ptr`, `QPointer`, `QSharedPointer`) when `QObject` ownership is insufficient. Never leave raw owning pointers.
- **Signals & Slots:** Use the modern compile-time syntax exclusively (`connect(sender, &Sender::signal, receiver, &Receiver::slot)`). String-based `SIGNAL()` and `SLOT()` macros are forbidden.

### QML Standards
- **Property Order:** Maintain uniform declaration order within every QML element:
  1. `id`
  2. Property declarations (`property int count`)
  3. Signal declarations
  4. JavaScript functions
  5. Object properties (`x`, `y`, `width`, `height`)
  6. Visual layout anchors (`anchors.*`)
  7. Appearance properties (`color`, `opacity`, `visible`)
  8. Signal handlers (`onClicked`, `onTextChanged`)
  9. Child items / Components
  10. States and transitions
- **Property Bindings:** Avoid breaking declarative bindings with imperative JavaScript assignments (e.g., avoid `width = parent.width` inside an imperative handler).
- **Handlers:** Avoid inline heavy JavaScript inside event handlers (`onClicked: { ... }`). Delegate non-trivial logic to dedicated C++ backend slots or well-scoped helper functions.

---

## 4. QML Component Library Development & Maintenance

Shared UI elements must live in the designated design system / component library module.

- **Atomic Design:** Keep reusable UI controls (`AppButton`, `AppTextInput`, `AppDialog`) decoupled from application domain state and network models.
- **Strict Encapsulation:**
  - Never reference outer parent IDs or global singleton models directly inside library components.
  - Expose external behavior exclusively via explicit `property`, `signal`, and alias declarations.
- **Documentation:** Every reusable component must contain a brief QDoc/header comment explaining its properties, signals, and an example usage snippet.
- **Preview & Isolation:** Ensure shared components can instantiate independently inside a dummy test harness or `qmlscene` without crashing due to missing backend dependencies.

---

## 5. Living Documentation & Continuous Alignment

This document is a living contract. Keeping it accurate is part of the implementation definition of done.

- **Synchronized updates:** When refactoring directory layouts, changing project dependencies, deprecating utilities, or establishing new UI patterns, update this file in the exact same commit or pull request.
- **Agent Verification Step:** Before concluding any task, verify:
  1. Did I introduce any duplicate logic?
  2. Are all new QML components properly isolated and placed in the component library?
  3. Does the implementation violate any convention stated in this file?

---

## 6. Project layout

| Path | Purpose |
|---|---|
| `src/askare/config.*` | Server URL, user name, TLS preference |
| `src/askare/secretsstore.*` | App password in a device-lock Sailfish Secrets collection |
| `src/askare/loginflow.*` | Nextcloud login flow v2 |
| `src/askare/httpclient.*` | `QNetworkAccessManager` wrapper |
| `src/askare/davclient.*` | CalDAV PROPFIND, REPORT, PUT, DELETE |
| `src/askare/davxml.*` | DAV XML readers |
| `src/askare/ics.*` | VTODO unfold, parse, rewrite |
| `src/askare/store.*` | SQLite copy of lists and tasks |
| `src/askare/session.*` | QML-facing controller |
| `src/askare/calendarmodel.*` | Task list model |
| `src/askare/taskmodel.*` | Tasks of one list |
| `qml/pages/*` | Silica pages |
| `README.md`, `docs/*` | Documentation site: Jekyll with `just-the-docs`, configured in `_config.yml` |
| `landing/index.html` | Marketing page, kept out of the Jekyll build; published copy lives in the portfolio site |

Networking and ICS stay in C++. Pages bind to `Askare`, `Calendars`, and `Tasks`.

User documentation lives under `docs/`, developer documentation under `docs/devel/`. A change to behaviour, settings, permissions or the build updates the matching page in the same commit.

Sailjail private directories follow `OrganizationName` / `ApplicationName`:

- `~/.config/org.frapps/harbour-askare`
- `~/.local/share/org.frapps/harbour-askare`
- `~/.cache/org.frapps/harbour-askare`