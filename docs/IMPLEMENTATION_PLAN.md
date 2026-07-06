# system-R — Implementation Plan

An open-source lighting console (grandMA3-style) built on the **JUCE 8** framework in
modern C++, buildable and runnable on **Linux, macOS and Windows**.

> Status: greenfield. The previous Java/JavaFX/Gradle prototype (`system_r/`) is being
> retired. Its domain decomposition (api / core / network·artnet / network·sacn / mvrgdtf /
> ui with tiles + per-window props) is a useful reference and is mirrored below.

---

## 1. Goals & scope

**Product goal:** a touch-first lighting control desk with a grandMA3-like UX — multiple
windows, each a resizable grid of tiles hosting different panels.

**In scope (this plan):**

- GDTF fixture library import + patch creation
- Patching fixtures onto multiple DMX universes
- Art-Net 4 and sACN (ANSI E1.31-2018) input **and** output
- A programmer for editing fixture attributes
- Presets for fixture attributes
- Cue storage

**Explicitly deferred** (acknowledged, not built in v1): timecode, MIDI/OSC remotes,
effects/phasers engine, 3D visualiser, networked multi-console session/parameter sharing,
RDM, macros/plugins, pixel-mapping. The architecture must not preclude them.

**Non-negotiable qualities:** deterministic real-time DMX output decoupled from the UI;
a serialisable show document with undo; correct, spec-compliant network protocols.

**Design/scale target (DECIDED):** **256 universes** (≈131,072 DMX channels) as the sizing
budget for the engine, network I/O, and UI performance work — the merge tick, frame
publishing, and live grids are all built and perf-tested against this ceiling.

---

## 2. Technology stack

| Concern | Choice | Rationale |
|---|---|---|
| Language | C++20 | JUCE 8 baseline; `std::span`, concepts, `<bit>` for packet work |
| UI / app framework | JUCE 8.0.x | Cross-platform GUI, UDP sockets, XML, ZIP, threads, `ValueTree` in one dependency |
| Build | CMake ≥ 3.24 + `juce_add_gui_app` | The only sane cross-platform JUCE path (Projucer is legacy); JUCE via `FetchContent` or submodule |
| Show serialisation | `juce::ValueTree` → binary + XML/JSON export | Native change-broadcast + `UndoManager`; maps cleanly onto the document model |
| GDTF/MVR | `juce::ZipFile` + `juce::XmlDocument` (own parser) | Avoids a heavy 3rd-party GDTF lib; keeps the dependency surface to JUCE only |
| Networking | `juce::DatagramSocket` (UDP) | Art-Net (6454) + sACN (5568), uni/multicast/broadcast |
| Tests | Catch2 (or GoogleTest) via CTest | Round-trip packet + GDTF-parse + merge-engine unit tests |
| CI | GitHub Actions matrix (ubuntu/macos/windows) | Guarantees the tri-platform build promise stays true |

**Licensing — DECIDED: open-source (AGPLv3) path.** JUCE 8 is used under its **AGPLv3**
option (no commercial licence purchased). Implications, to action before first release:
- The **distributed application** (JUCE-linked binaries) is licensed **AGPLv3** — full
  source of the distributed version must be offered, including the AGPL §13 network-use
  clause. Add an `AGPLv3` `LICENSE` (or `LICENSE.app`) covering distributed builds.
- First-party `core`/`app` **source** can stay **Unlicense** (public domain is compatible
  with combining into an AGPLv3 work); keep the existing `LICENSE` for the source tree and
  make the combined-work licensing explicit in the README.
- Any third-party lib added later must be AGPLv3-compatible (permissive or GPL-family).

---

## 3. High-level architecture

Strict separation between a **headless engine** (no JUCE GUI dependency, unit-testable) and
the **JUCE UI**. The UI never touches DMX buffers or sockets directly; it issues commands
and reads snapshots.

```
                        ┌───────────────────────────────────────────┐
                        │                 UI (JUCE msg thread)       │
                        │  Windows → Tiled layout → Panels           │
                        │  (Patch, Fixture Sheet, Programmer,        │
                        │   Presets, Cues, DMX view, Settings)       │
                        └───────────────▲───────────────┬───────────┘
                        snapshots / ValueTree           │ commands (lock-free FIFO)
                        change broadcasts               ▼
        ┌───────────────────────────────────────────────────────────────────┐
        │                     CORE (headless, no GUI)                        │
        │                                                                     │
        │  ShowDocument (ValueTree + UndoManager)                             │
        │    Patch · FixtureInstances · Presets · Sequences/Cues             │
        │                                                                     │
        │  FixtureLibrary  ◄── GDTF/MVR import                                │
        │                                                                     │
        │        ┌──────────────── Parameter / Merge Engine ───────────────┐  │
        │        │  layers: Programmer ▸ active Playback ▸ default          │  │
        │        │  → per-fixture attribute values → encode to DMX slots    │  │
        │        └───────────────────────────┬──────────────────────────────┘ │
        └────────────────────────────────────┼────────────────────────────────┘
                       engine thread (fixed rate ~40 Hz)
                                             ▼  (lock-free triple buffer of universe frames)
        ┌───────────────────────────────────────────────────────────────────┐
        │                       NETWORK I/O layer                            │
        │  Output: Art-Net 4 sender · sACN E1.31 sender  (per-universe cfg)   │
        │  Input:  Art-Net receiver · sACN receiver (merge/monitor sources)   │
        └───────────────────────────────────────────────────────────────────┘
```

### Threading model

- **UI thread** — JUCE message thread. Reads engine snapshots; mutates `ShowDocument` for
  structural edits (patch, record cue); sends real-time edits (programmer touches) to the
  engine via a lock-free command FIFO (`juce::AbstractFifo`). Never blocks the engine.
- **Engine thread** — a `HighResolutionTimer`/dedicated thread at a fixed rate (~40 Hz).
  Drains the command FIFO, advances cue fades, runs the merge, encodes universe frames,
  publishes them into a **lock-free triple buffer**. No allocations, no locks in the hot path.
- **Network output** — reads the latest published frame, packetises, sends. Can share the
  engine thread initially; split out if send jitter demands it. Honours per-protocol keepalive.
- **Network input** — one receiver thread per protocol; parses inbound packets into
  source-tracked universe buffers (for monitoring / HTP-merge / tracking-backup later).

Rule: structural/model data flows UI→engine as commands and engine→UI as immutable
snapshots. The only shared mutable memory is the triple-buffered frame set, swapped atomically.

---

## 4. Module / directory layout

```
system-R/
├─ CMakeLists.txt                # top-level; fetches JUCE, adds subdirs
├─ cmake/                        # helper modules, warnings, sanitizers
├─ external/JUCE/                # submodule (or FetchContent)
├─ libs/
│  ├─ core/                      # headless engine + domain model (no JUCE GUI)
│  │  ├─ model/                  # ShowDocument, FixtureInstance, Patch, Universe
│  │  ├─ library/                # FixtureType, DmxMode, ChannelFunction, Attribute
│  │  ├─ gdtf/                    # GDTF/MVR zip+xml parser → FixtureType
│  │  ├─ engine/                 # merge/parameter engine, value stack, encoder
│  │  ├─ programmer/             # active-value store & edit ops
│  │  ├─ presets/                # preset pools & resolution
│  │  ├─ cues/                    # sequences, cues, tracking, fade playback
│  │  └─ net/                     # artnet/ + sacn/ (packets, sender, receiver)
│  └─ testutil/
├─ app/                          # JUCE GUI application
│  ├─ Main.cpp / Application
│  ├─ windows/                   # DocumentWindow per screen, window manager, .prp persistence
│  ├─ layout/                    # tiled grid layout manager (splitters, add/remove tiles)
│  ├─ panels/                    # PatchPanel, FixtureSheet, ProgrammerPanel, PresetPool,
│  │                             # CuePanel, DmxOutputView, SettingsPanel
│  ├─ commands/                  # UI→engine command builders
│  └─ look/                      # LookAndFeel, theming, touch metrics
├─ tests/                        # Catch2 + CTest
├─ resources/                    # gdtf.xsd / gdtf.sch (validation), icons, default show
└─ docs/
```

`core/` compiles against `juce_core`, `juce_events`, `juce_data_structures` only (no GUI) so
it links into the test binary and could later back a headless/CLI node.

---

## 5. Domain model (ValueTree schema sketch)

The show is one `ValueTree` tree under an `UndoManager`. Sketch:

```
SHOW
├─ PATCH
│  └─ FIXTURE  fid, name, gdtfTypeId, modeName, universe, address, subfixtureOf?, position(x,y,z)
├─ UNIVERSES
│  └─ UNIVERSE  index(1..N), outProtocol(artnet|sacn|both), net/subnet/uni | e131Universe,
│               destIp/multicast, priority, enabled
├─ LIBRARY                       # cache of imported FixtureTypes (or referenced by hash)
├─ PRESETS
│  └─ POOL  featureGroup(Dimmer|Position|Color|Gobo|Beam|Focus|Control|Shapers|Video)
│     └─ PRESET  id, name, kind(selective|universal|global), VALUES{fid→{attr→value}}
├─ SEQUENCES
│  └─ SEQUENCE  id, name
│     └─ CUE  number, name, fadeIn, fadeOut, delay, VALUES (tracked)
└─ SETTINGS  networkInterfaces, outputRate, theme, windows[]
```

- **Attributes** follow GDTF naming (`Dimmer`, `Pan`, `Tilt`, `ColorAdd_R/G/B`, `Zoom`,
  `Gobo1`, `Shutter1`, …) grouped by **FeatureGroup** for panel/preset organisation.
- Preset/cue values reference *attributes*, not raw DMX — resolution to DMX slots happens in
  the engine via the fixture's `DmxMode`/`ChannelFunction` mapping. This keeps presets
  portable across fixture types where attributes match (grandMA3 behaviour).
- Fixtures with multiple geometry instances become **subfixtures** (multi-cell/pixel bars).

---

## 6. Feature deep-dives

### 6.1 GDTF / MVR import (`core/gdtf`)

- A GDTF file is a ZIP whose `description.xml` holds `<FixtureType>` with
  `AttributeDefinitions` (ActivationGroups, FeatureGroups, Attributes), `Wheels`,
  `PhysicalDescriptions`, `Models`, `Geometries`, and one or more `DMXModes`.
- Each `DMXMode` → `DMXChannels` → `LogicalChannels` → `ChannelFunctions` → `ChannelSets`,
  plus `Relations`. Parse into an internal `FixtureType { modes[], geometries[], attributes[] }`.
- Handle: coarse/fine/ultra bytes (8/16/24-bit), default values, highlight/reset values,
  physical↔DMX ranges per `ChannelFunction`, virtual/no-feature channels, wheel slots.
- Validate against bundled `gdtf.xsd` (+ `gdtf.sch`) — carried over from the old prototype.
- **MVR** (`.mvr` zip → `GeneralSceneDescription.xml`) for importing an entire rig with
  addresses/positions in one step — **v1 requirement** (DECIDED). Both paths ship in v1:
  GDTF import + manual patch **and** MVR import of a full scene (fixtures, addresses,
  positions, layers) in one action. Shared `FixtureType`/geometry parsing backs both.
- Risk: GDTF is large and uneven across real-world files. Build a corpus of GDTF Share
  samples into the test suite and parse defensively (unknown → sensible default, never crash).

### 6.2 Patch & universes (`core/model`)

- Assign each fixture a start address on a universe; the mode's channel count determines the
  footprint. Detect/flag overlaps. Support patching one fixture across a universe boundary
  only if disallowed (flag it) — keep MVP simple: no boundary spanning.
- Universes are logical (1..N, thousands possible); each maps independently to an Art-Net
  port-address and/or an sACN universe with its own output config.

### 6.3 Network I/O (`core/net`)

Reuse the old prototype's packet decomposition (it already had ArtDmx/ArtPoll/ArtPollReply/
ArtSync and sACN Data/Sync/Discovery packet classes) — port to C++ with round-trip tests.

**Art-Net 4** (Artistic Licence), UDP port **6454**:
- Header ID `"Art-Net\0"`; little-endian opcodes: `OpPoll 0x2000`, `OpPollReply 0x2100`,
  `OpDmx 0x5000`, `OpSync 0x5200`, `OpAddress 0x6000`.
- `ArtDmx`: ProtVer 14, Sequence, Physical, 15-bit **Port-Address** = Net(7)·Sub-Net(4)·
  Universe(4) → 32,768 universes; length + ≤512 data bytes.
- Send on change and as keepalive (≥ every 4 s); cap refresh (~44 Hz). Support `ArtSync`
  for synchronised frame release. `ArtPoll`/`ArtPollReply` for discovery.

**sACN — ANSI E1.31-2018**, UDP port **5568**:
- Three layers: **Root** (ACN PID `"ASC-E1.17"`, CID/UUID per source), **Framing**
  (64-byte source name, **priority** 0–200 default 100, sync address, sequence #, options:
  preview/stream_terminated/force_sync, universe), **DMP** (start code `0x00` + ≤512 slots).
- **Multicast** `239.255.{uni_hi}.{uni_lo}` for universes 1–63999; universe **discovery** on
  64214 every ~10 s; source-loss timeout 2.5 s; per-address priority (start code `0xDD`) later.
- Multiple-source arbitration by priority; higher wins, ties → HTP (implement on input path).

Both senders read from the same published universe frames; per-universe config selects
protocol(s), destination (unicast/broadcast/multicast), and priority.

**Input is first-class (DECIDED): monitor + HTP/priority merge + tracking-backup.** The
receiver path does more than display incoming universes:
- **Priority/HTP merge** — inbound sources are tracked per universe (by CID/source name);
  arbitration is by sACN priority (higher wins), ties and equal-priority Art-Net resolved
  **HTP**. Merged external input can be fed into the engine as a mergeable layer (see §6.4).
- **Tracking-backup input** — the receiver captures the full incoming universe state so this
  console can mirror another desk's live output (the data plumbing for backup/failover).
  Full backup *role/failover orchestration* is deferred with multi-console, but the input
  capture + merge it depends on is built now.
- Source-loss timeout (sACN 2.5 s) releases a source from the merge.

### 6.4 Parameter / merge engine (`core/engine`)

The correctness-critical heart. Per engine tick, for every patched fixture attribute compute
the winning value from an ordered **value stack**:

```
Programmer (highest) ▸ active Playback/Cue (with running fades) ▸ merged external input ▸ default/home
```

The **merged external input** layer is the HTP/priority-merged result of inbound Art-Net/sACN
(§6.3) — present so this desk can combine with, or track-back, another console's output.
It sits below local playback so local control always wins unless explicitly configured otherwise.

- LTP semantics for most attributes; the **Programmer overrides everything** while active
  (grandMA3 model). Dimmer merge policy (HTP option) configurable per install.
- Resolve attribute value → DMX slot(s) via the fixture's `ChannelFunction` (physical→DMX
  range, coarse/fine split). Write into the target universe's 512-byte buffer.
- Cue fades: interpolate attribute values over `fadeIn/fadeOut/delay`; the engine owns the
  fade clock so playback is smooth regardless of UI load.
- Hot path is allocation-free and lock-free; publishes a full universe frame set to the
  triple buffer each tick.

### 6.5 Programmer (`core/programmer`)

- Holds **active** attribute values for the current fixture selection; tracks which attributes
  are "touched" (active) vs untouched — only active attributes are recorded/output-priority.
- Operations: select fixtures, set attribute (absolute/relative), call a preset (stores a
  reference or hard values), Highlight, Clear (partial/all), Off.
- Feeds the engine's top layer directly (real-time, via command FIFO).

### 6.6 Presets (`core/presets`)

- Pools per FeatureGroup (All/Dimmer/Position/Color/Gobo/Beam/Focus/Control/Shapers/Video).
- Store **selective** (per-fixture values), **universal** (per fixture-type), or **global**
  (single value applied to all) — matching grandMA3 semantics.
- Recording a preset snapshots the programmer's active values for the selection.
- **Reference-tracked in v1 (DECIDED).** Calling a preset stores a **reference** to it
  (preset id + affected attributes) in the programmer and in recorded cues — *not* a hard
  copy of the values. Editing the preset later **propagates** to every programmer state and
  cue that references it. Implications built in from the start:
  - Cue/programmer value entries are a tagged union: `hard value | preset reference`.
  - The engine resolves references at evaluation time (preset lookup → current values →
    encode), so a preset edit is reflected on the next engine tick everywhere it's used.
  - Deleting a preset must be handled (block, or convert dependents to hard values); track
    a reference/usage count so the UI can warn and resolve dependents safely.

### 6.7 Cues & playback (`core/cues`) — **full tracking model (DECIDED)**

- A **Sequence** is an ordered list of **Cues**; each cue stores recorded attribute values
  plus timing (`fadeIn/fadeOut/delay`).
- **Full tracking from day one** (grandMA3 model): a cue stores only the attributes that
  *change* relative to the tracked state, and values **track forward** through the sequence
  until another cue changes them. The engine resolves a cue's effective output by walking
  from the last hard reference through intervening deltas. This is a core design constraint,
  not a later add-on — the cue store, recording, and engine resolution are all built around it.
- Required tracking semantics to support:
  - **Block cue** — snapshots the full tracked state so downstream edits don't track past it.
  - **Cue-only / track** record + edit modes — apply a change to one cue vs. let it track on.
  - **Tracking through changes** when a value is edited mid-sequence (propagate until the next
    hard change) and correct behaviour on cue insert/delete/reorder (re-track affected range).
  - Mark/"move-in-black" and off/release handling as fast-follow within this model.
- Record from programmer → cue (only active/touched attributes become the cue's stored
  deltas). Playback: Go/Go-back/Goto moves the active cue and starts engine-driven fades; the
  engine samples the tracked state at the target cue as the fade destination.
- v1 = one main sequence with Go/Go-back/Goto + block/cue-only; multiple executors fast-follow.

### 6.8 UI (`app/`)

- **Multiple windows**: a `DocumentWindow` per screen; a window manager tracks them and
  persists geometry/content (carry over the old `.prp` per-window props idea).
- **Tiled layout**: each window hosts a resizable grid of tiles with draggable splitters and
  add/remove/split actions — the grandMA3 "views" concept. Build a custom `TiledLayout`
  manager (JUCE `Grid`/`FlexBox` for leaf sizing; a binary-split tree for the arrangement).
- **Panels** (a tile picks its content type): Patch, Fixture Sheet (grid of fixtures ×
  attributes), Programmer (encoders/attribute editors by FeatureGroup), Preset Pools, Cue
  list, DMX Output view (live universe grid), Settings/Network.
- **Touch-first**: large hit targets, encoder/wheel widgets, on-screen fader/button executors.
  Custom `LookAndFeel`; consider `juce::OpenGLContext` for the heavy live grids.
- UI reads engine snapshots (for live DMX/fixture values) and the `ShowDocument` `ValueTree`
  (for structure), and never writes DMX buffers directly.

---

## 7. Build, tooling & cross-platform

- **CMake** top-level pulls JUCE (submodule `external/JUCE` or `FetchContent`); `core` is a
  static lib, `app` is `juce_add_gui_app`, `tests` is a CTest target.
- Compilers: GCC/Clang (Linux), AppleClang (macOS, universal arm64+x86_64), MSVC (Windows).
- Linux runtime deps: ALSA/FreeType/X11/`libcurl` per JUCE (document `apt` install line).
- **CI**: GitHub Actions matrix `{ubuntu-latest, macos-latest, windows-latest}` → configure,
  build, run CTest. Cache JUCE. This is what keeps the "runs on all three" promise honest.
- Sanitizers (ASan/UBSan) on the Linux CI job for `core` + tests.
- Packaging (later): AppImage/.deb (Linux), signed `.app`/`.dmg` (macOS), MSI/portable (Windows).

---

## 8. Testing strategy

- **Protocol round-trips** — encode→decode→encode identity for every Art-Net and sACN packet;
  golden byte-vectors checked against the specs (highest ROI; catches wire-format bugs).
- **GDTF parsing** — a corpus of real GDTF Share files; assert mode/channel/attribute counts,
  coarse/fine mapping, defaults; fuzz malformed archives (must not crash).
- **Merge engine** — table-driven: given a value stack (programmer/cue/default) assert the
  winning attribute value and encoded DMX bytes, including fades sampled over time.
- **Model/serialisation** — save→load→save `ShowDocument` identity; undo/redo invariants.
- **Manual/integration** — loopback: run a sender, capture with a receiver (or QLC+/sACNView)
  and eyeball a real fixture/visualiser.

---

## 9. Phased roadmap

Each phase ends in something runnable. **Phase 0/1 is a walking skeleton** proving the whole
vertical (UI → engine → wire) before breadth is added.

**Phase 0 — Scaffolding** ✅ *done*
CMake + JUCE building an empty JUCE app on all three OSes in CI. `core` lib + test harness.
License decision on JUCE. → *empty window opens everywhere.*

**Phase 1 — Walking skeleton (vertical slice)** ✅ *done*
Minimal `ShowDocument`; hard-coded single dimmer fixture; a programmer slider; engine tick;
**sACN output** of one universe. Prove UI→engine→wire→real DMX end-to-end.

**Phase 2 — Network I/O complete** ✅ *done*
Full Art-Net 4 + sACN E1.31 senders (multi-universe, keepalive, multicast, priority) and
receivers **with priority/HTP input merge + tracking-backup capture**, round-trip tests, and
scaling to **256 universes**. Per-universe output config + DMX Output view panel.

> Delivered: `core/artnet` (ArtDmx/ArtSync/ArtPoll/ArtPollReply build+parse), `ArtNetSender`,
> `ArtNetReceiver`; generalised multi-universe `SacnSender` + `E131Packet` parser +
> `SacnReceiver`; `core/net/InputMerger` (priority-wins, HTP-among-equals, source-loss
> timeout, per-source capture for tracking-backup); `UniverseOutput` config + `OutputManager`
> (change-detection + keepalive, one socket per protocol); engine renders/publishes N
> universes; `DmxOutputView` live grid panel. 36 unit tests (round-trip + merge + keepalive +
> 256-universe budget). *Deferred to later phases:* wiring the merged external-input layer into
> the engine value stack (Phase 4 merge engine) and ArtSync-gated frame release.

**Phase 3 — Fixtures & patch (GDTF + MVR)**
GDTF parser → `FixtureLibrary`; **MVR scene import** (fixtures/addresses/positions/layers);
import UI; Patch panel; multi-universe patching with overlap detection; Fixture Sheet panel.
Engine encodes real modes (coarse/fine, channel functions).

**Phase 4 — Programmer & presets (reference-tracked)**
Selection + attribute editing by FeatureGroup; active-value tracking; Highlight/Clear/Off;
preset pools (record/call) with selective/universal/global and **reference-tracked calls**
(preset edits propagate to programmer + cues; usage counting for safe delete).

**Phase 5 — Cues & playback (full tracking)**
Sequences + cues built on the **full tracking** model (delta storage, track-forward
resolution, block + cue-only edit modes, re-track on insert/delete); record from programmer;
Go/Go-back/Goto with engine-driven fades sampling the tracked state; a Cue list panel and a
basic executor.

**Phase 6 — UI depth & polish**
Multi-window management with persisted layouts; full tiled layout (split/resize/add/remove);
touch LookAndFeel; settings/network UI; save/load show files; undo/redo throughout.

**Phase 7 — Hardening & packaging**
Performance passes at the **256-universe** target (thousands of fixtures), soak tests,
installers, docs.

---

## 10. Risks & open questions

**Risks**
- *Merge-engine correctness & real-time safety* — the make-or-break subsystem. Mitigate with
  the walking skeleton early, exhaustive table tests, and a strict no-alloc/no-lock hot path.
- *GDTF variability* — real files are inconsistent; parse defensively against a broad corpus.
- *Scope* — grandMA3 is enormous. The phase gates and the deferred-features list keep v1 finite.
- *UI performance* — live grids over many universes; budget for OpenGL-backed rendering.
- *Network timing/jitter* — keepalive + refresh caps + optional dedicated send thread.

**All resolved** ✅
1. **JUCE licensing** — open-source **AGPLv3** path (see §2 licensing note for actions).
2. **Cue tracking model** — **full tracking** from the start (see §6.7).
3. **Presets** — **reference-tracked** in v1; preset edits propagate (see §6.6).
4. **MVR import** — **v1 requirement**, alongside GDTF import (see §6.1, Phase 3).
5. **Input use** — monitor **+ HTP/priority merge + tracking-backup** capture (see §6.3–6.4).
6. **Target scale** — **256 universes** (~131k channels) as the design/perf budget (see §1).
```
