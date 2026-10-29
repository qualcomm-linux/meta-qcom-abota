# AGENTS.md

Guidance for AI coding agents working in the **meta-qcom-abota** layer.

This layer adds signed SWUpdate A/B OTA support for Qualcomm boards (e.g.
qcs8300-ride-sx) on top of `meta-qcom`. Follow `CONTRIBUTING.md` for the
contribution and DCO requirements; this file adds agent-specific conventions,
chiefly the branch/PR workflow and how to write commits.

## Branch and pull request workflow

`CONTRIBUTING.md` is authoritative; this is the short version agents must
follow.

- **Never commit directly to `main`.** `main` tracks the upstream project.
  All work goes on a topic branch and reaches `main` only through a reviewed
  pull request.
- **Start every change on a new topic branch** cut from an up-to-date `main`:

  ```sh
  git checkout -b <topic-branch> main
  ```

- **Keep the branch rebased on upstream** before pushing, so it applies cleanly
  and the PR is a minimal diff:

  ```sh
  git pull --rebase upstream main
  ```

- **One focused change per branch/PR.** Independent changes go in separate
  branches and separate pull requests, not bundled together.
- **Push to your fork (`origin`), then open the PR against `main`:**

  ```sh
  git push -u origin <topic-branch>
  ```

- Do **not** push (to any remote) without explicit confirmation from the user.

## Commit message best practices (project style)

Commits in this layer follow the same style as `meta-qcom`. Read
`CONTRIBUTING.md` first — it is authoritative; the rules below make the
agent-specific expectations explicit.

### Structure

- **One atomic change per commit.** Do not bundle unrelated changes, and do not
  add follow-up "fixup" commits to a series — fold a fix back into the commit
  that introduced the problem.
- **Subject:** `recipe-or-file: summary of the change`, imperative mood, no
  trailing period. Match the prefix style already in `git log` (e.g.
  `sw-description:`, `abota-image:`, `swupdate:`, `README.md:`).
- **Body:** plain English. State the problem/motivation first, then what the
  change does. Explain facts that are *not* visible in the diff; omit what the
  diff already shows. One or two short paragraphs is usually enough.

### Required trailers

Every commit ends with these two trailers, in this exact order:

```
Assisted-by: Claude Code:<model-id>
Signed-off-by: <Name> <email>
```

- `Assisted-by:` names the agent and model, e.g. `Assisted-by:
  Claude Code:claude-opus-4.8`. Include it on any commit an agent helped author.
- `Signed-off-by:` is the DCO sign-off (https://developercertificate.org/).
- **`Assisted-by` comes before `Signed-off-by`**, so the sign-off is the last
  trailer in the message.

### Identity — never fabricate

Take the sign-off name and email from the local git config, never invent them:

```
Signed-off-by: $(git config user.name) <$(git config user.email)>
```

If `user.name`/`user.email` are unset, stop and ask the user to configure them
rather than guessing.

### Writing the trailers in the correct order

Do **not** run `git commit -s` and then append `Assisted-by` afterward: `-s`
appends `Signed-off-by` as the last line, which puts the two trailers in the
wrong order. Instead write both trailers yourself, in one message:

```sh
git commit -m "recipe-name: summary of the change

Explain the problem being solved and what the change does, covering
anything not obvious from the diff.

Assisted-by: Claude Code:claude-opus-4.8
Signed-off-by: Roshan Sivakumar <roshs@qti.qualcomm.com>"
```

which produces:

```
recipe-name: summary of the change

Explain the problem being solved and what the change does, covering
anything not obvious from the diff.

Assisted-by: Claude Code:claude-opus-4.8
Signed-off-by: Roshan Sivakumar <roshs@qti.qualcomm.com>
```
