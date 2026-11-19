# Inverter Grid-Support Functions & Dynamic Power Limit — Info Sheet

A plain-language overview of the grid-support features a modern PV micro-inverter provides, what
each is for, what you notice, and how they work together. No technical internals — just what the
features do and mean.

## In one picture

Every feature here steers one of two things the inverter puts onto the grid:

- **Active power (Watt)** — how much energy it actually feeds in.
- **Reactive power / power factor** — how it supports grid voltage and power quality.

Plus one pure **safety** function that only disconnects.

Two simple rules explain how they combine:

1. **The active-power features add up as limits — the strictest one wins.** Several features can
   cap or reduce output at the same time; the inverter always follows the lowest limit currently
   in force.
2. **The reactive-power features are alternatives — only one runs at a time.** There are four
   different ways to decide reactive power; a grid profile uses exactly one of them.

---

## Active power — "how much is fed in"

**DPL — Dynamic Power Limit**
- What it is: a live, adjustable cap on output, set from outside (e.g. by a gateway/home energy
  manager for zero-export or export control).
- What you notice: output follows the commanded limit smoothly, applied in small steps and only
  after the new target holds for a moment, so it never jumps or flickers. It keeps a small
  guaranteed minimum (about 2% of rated power) and shares the limit across the inverter's inputs.
  DPL alone never switches the inverter fully off — a real shut-off uses a separate off command.
- Across firmware versions and models (observed):
    - HMS-4T V01.00.27 — basic limit: cap + smooth ramp only.
    - HMS-4T V01.01.12 — adds per-input (per-MPPT) connect/disconnect handling (PDL).
    - HMS-4T V02.00.04 — adds the guaranteed ~2% minimum and applies limit changes more
      smoothly (reworked into one controller).
    - HMS-2T (V01.00.08 through V01.03.09) — basic limit only (cap + ramp); no per-input
      handling and no guaranteed minimum, i.e. still at the earlier HMS-4T V01.00.27 level.

**APC — Active Power Control**
- What it is: the framework that enables active-power control and sets how fast output is allowed
  to change (soft-start and ramp rate).
- What you notice: power changes are smooth and standards-compliant instead of abrupt.

**FW — Frequency-Watt**
- What it is: automatic reduction of output when the grid frequency rises too high, and recovery
  when it settles — a mandatory grid-stabilisation response.
- What you notice: temporary throttling during over-frequency events; fully automatic.

**VW — Volt-Watt**
- What it is: the same idea driven by grid voltage — reduce output when voltage rises too high.
- What you notice: temporary throttling when local voltage is high; fully automatic.

## Reactive power / power quality — "voltage & power-factor support" (choose one)

**SPF — Specified Power Factor**
- What it is: the inverter holds a fixed power factor (a set ratio of reactive to active power).
- Use: simplest way to meet a required power factor.

**WPF — Watt-Power-Factor**
- What it is: the power factor changes with output level — neutral at low power, more supportive
  near full power.
- Use: voltage support that only engages when generation is high.

**Volt-Var (also called "CC")**
- What it is: reactive power follows the grid voltage — absorb when voltage is high, supply when
  low — to actively hold local voltage steady.
- Use: the most active voltage-support mode, where the grid operator requires it.

**RPC — Reactive Power Control**
- What it is: a fixed reactive-power setpoint on command, independent of output or voltage.
- Use: direct reactive-power dispatch by the grid operator.

## Safety

**ID — Island Detection (anti-islanding)**
- What it is: detects loss of the utility grid and disconnects, so the inverter never energises a
  dead grid.
- What you notice: nothing in normal operation; on grid loss the inverter shuts off and stays off
  until the grid returns. This always takes priority over everything else.
- On/off: when switched **on**, it does more than just watch the limits — it actively looks for a
  persistent abnormal frequency deviation and disconnects if it lasts; when switched **off**, only
  the standard over/under voltage & frequency windows guard against a lost grid.

---

## How they work together — at a glance

| Feature | Controls | Group | Runs alongside | Excludes |
|---|---|---|---|---|
| DPL | output limit (live) | active power | APC, FW, VW | — |
| APC | change rate / enable | active power | DPL, FW, VW | — |
| FW  | output vs frequency | active power | DPL, APC, VW | — |
| VW  | output vs voltage | active power | DPL, APC, FW | — |
| SPF | fixed power factor | reactive power | one active-power feature | WPF, Volt-Var, RPC |
| WPF | power factor vs output | reactive power | one active-power feature | SPF, Volt-Var, RPC |
| Volt-Var / CC | reactive vs voltage | reactive power | one active-power feature | SPF, WPF, RPC |
| RPC | fixed reactive power | reactive power | one active-power feature | SPF, WPF, Volt-Var |
| ID  | disconnect on grid loss | safety | everything | — (overrides all on trip) |

Notes:
- Active-power features never conflict — they simply intersect, and the tightest limit applies.
- The four reactive-power features are mutually exclusive. A grid profile normally activates
  exactly one; if several were enabled at once, the inverter still applies only ONE, by a fixed
  priority — **Volt-Var, then Watt-Power-Factor, then Specified Power Factor, then Reactive
  Power Control, then a fifth curve-based reactive mode** — and the others are ignored. They
  never add up.
- Active and reactive power share the inverter's total capacity, so at very high output the
  available reactive support is reduced (and vice-versa).
- Island Detection is independent protection; when it disconnects, all output stops.

## Good to know (how the inverter actually behaves)

- **Each function is configured individually** in the grid profile — its own on/off plus its own
  thresholds or curve. A profile therefore uses any combination of active-power features together
  with exactly one reactive-power mode.
- **The strictest active-power limit always wins.** If, say, an export limit (DPL) and an
  over-frequency reduction (FW) apply at the same time, the inverter follows whichever allows less
  — automatically, without any conflict between them.
- **Limits are applied gently, not abruptly** — small steps with a brief settle time — which is
  why output ramps rather than steps when a new limit arrives.
- **Reactive support gives way to active power when the inverter is near full output**, because
  both draw on the same total capacity; more headroom for voltage/power-factor support appears
  whenever active power is reduced.
