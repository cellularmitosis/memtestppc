# Sessions

Substantive work on memtestppc+ happens in **sessions**. One Claude conversation
produces one session, recorded as a single `HANDOFF.md` in its own
globally-numbered directory:

```
docs/sessions/NNN-<short-slug>/HANDOFF.md
```

## Why a single HANDOFF per session

The `HANDOFF.md` is the session's **running log, its findings, and its primer for
the next session, all in one file**. We deliberately do not split narrative and
handoff into separate documents — the overhead isn't worth it, and one file is
what actually gets read. So catch-up at the start of a session is deliberately
cheap:

> **`CLAUDE.md` (loaded automatically) + the project `README.md` + the most
> recent `HANDOFF.md`.**

We don't read a stack of old HANDOFFs — that would burn tokens fast. The
trade-off is a writing discipline: **each HANDOFF must stand on its own.** Carry
forward any still-relevant state from earlier sessions into the current one
(restate it, or link the older session dir) rather than assuming the next reader
has seen it.

## What a HANDOFF.md contains

- `# Session NNN: <topic>` — title.
- **What happened** — the running log. What was done, in roughly chronological
  order, with enough detail that a future session can reconstruct the thinking:
  decisions and their rationale, dead-ends hit (so they aren't re-tried), and
  gotchas discovered. Findings live here — there is no separate findings file.
- **Current state** — status on exit: what works, what's half-finished, and
  where the canonical artifacts live now if anything moved.
- **What to do next** — a priority-ordered punch list for the next session, with
  a clear "I'd start with #1" recommendation.

Write it so a stranger could act on it cold: real file paths, real command lines
(the rsync/build/QEMU invocations), and the *why* behind choices. The existing
001–004 handoffs are the model.

## Naming

`NNN-<short-slug>/`

`NNN` is a global, monotonically-increasing session number across the whole life
of the project, starting at `001`. The slug hints at the substantive content
(`001-inception-and-scaffold`, `004-real-hardware`) so sessions are skimmable
from a plain `ls`.

History note: sessions `001-inception-and-scaffold` and `002-cd-boot` were
written *retroactively* (during session 005) from the transcript in
`docs/convos/`, because the original first working session compacted three times
and only emitted one handoff (labelled `003`). Their top-of-file notes say so.
`003` and `004` were written live.

## Start-of-session checklist

1. **Read `CLAUDE.md` (loaded automatically), this project's `README.md`, and the
   most recent `docs/sessions/NNN-*/HANDOFF.md`.** That's the standard catch-up —
   the latest HANDOFF is written to stand on its own. Only dig into older
   HANDOFFs if the latest one points you at something specific.
2. Glance at [`../PLAN.md`](../PLAN.md) for mission and current priorities.
3. Confirm the baseline builds before changing anything load-bearing: rsync to
   the build host and `make memtestppc.iso` (see `CLAUDE.md` → Build & test).

## End-of-session ritual

1. Commit the actual work.
2. **Write this session's `HANDOFF.md`** — every session, not just the eventful
   ones. The only valid reason to skip is that the project has genuinely run out
   of roadmap; even then, a HANDOFF saying exactly that beats silence.
3. Update [`../PLAN.md`](../PLAN.md) if priorities or scope shifted.
4. Update the project `README.md` for anything user-visible that changed — flip a
   status-table row, note a newly-working surface. (See `CLAUDE.md` → README
   format.)
5. **Print the next session's HANDOFF path** in your closing summary as a
   clickable relative link, so the user can paste it into the next session's
   opening prompt.
6. Commit the session notes.
