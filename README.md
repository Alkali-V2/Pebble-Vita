# Pebble Vita

## How to Read It

**Hour Grid** — A 4×3 grid of rounded blocks represents the 12-hour cycle. Blocks fill column by column, top to bottom: at 3:00 the first column is fully lit, at 6:00 the first two columns are, and so on. The next upcoming block progressively fills from top to bottom as minutes tick by.

**Minute Number** — The current minute (00–59) is drawn inside the filling block with an outlined font. AM and PM are distinguished by swappable text/border color pairs, so you can tell the period at a glance without a separate indicator.

**Progress Bars** — Three bars sit below the grid:

- **Day bar** (top-left) — Progress through the current day of the month, or day of the year (configurable). Shows the day number inside the bar.
- **Month bar** (top-right) — Progress through the current month of the year. Shows the month number inside the bar.
- **Life bar** (full width, bottom) — Progress from your birthday toward a target year you set. Shows your current age on the left and today's step count on the right.

The left/right positions of the Day and Month bars can be swapped in settings. All in-bar numbers can be individually toggled on or off.

**Health-Colored Life Bar** — When enabled, the life bar's fill color reflects your daily step count using a gradient mapped to the steps-vs-mortality dose-response curve from the medical literature:

| Steps | Color | Significance |
|---|---|---|
| 0–3,500 | Red | Below protective threshold |
| 5,500 | Orange | Climbing the steepest benefit curve |
| 7,000 | Yellow | ~47% lower all-cause mortality vs sedentary |
| 8,800 | Yellow-green | Near modeled optimal dose |
| 12,500+ | Green | Highest-benefit cohort |

When health data isn't available (permissions not granted, Aplite platform), the life bar falls back to its static configured color rather than showing a misleading red.

## Compatibility

| Platform | Device | Display |
|---|---|---|
| Basalt | Pebble Time | 144×168, color |
| Emery | Pebble Time 2 / Time Steel | 200×228, color |
| Aplite | Pebble Classic / Steel | 144×168, B&W |
| Diorite | Pebble 2 | 144×168, B&W |

B&W platforms default to black/white with dithered gray for the inactive state. Color platforms show the full 64-color Pebble palette in the settings picker.

Health features (step count, health-colored life bar) require Pebble Health and are compiled out on Aplite, which has no HealthService support.

## Settings

All settings are configured through the Clay settings page in the Pebble companion app and persist across watch restarts.

**Colors**

| Setting | Description |
|---|---|
| Background | Screen background color |
| Active (dots & bar) | Filled hour blocks and default bar fill |
| Inactive (unfilled) | Empty hour blocks and bar empty portions |
| AM Minute Text / Border | Text and outline colors for the minute number during AM |
| PM Minute Text / Border | Text and outline colors for the minute number during PM |
| Day Bar Color | Fill color for the day progress bar |
| Month Bar Color | Fill color for the month progress bar |
| Life Bar Color | Fill color for the life bar (overridden when health coloring is on) |

**Display Toggles**

| Setting | Description |
|---|---|
| Show Minute Number | Show/hide the minute digits in the filling block |
| Show Day Number on Bar | Show/hide the day number inside the day bar |
| Show Month Number on Bar | Show/hide the month number inside the month bar |
| Show Age on Life Bar | Show/hide your current age on the left end of the life bar |
| Show Step Count on Life Bar | Show/hide today's step count on the right end of the life bar |
| Show Health Color Goals | Enable/disable the step-based color gradient on the life bar |

**Bar Configuration**

| Setting | Description |
|---|---|
| Top Row Order | Swap the Day and Month bars between left and right positions |
| Day Bar Shows | Choose between day-of-month or day-of-year for the day bar |

**Life Progress**

| Setting | Description |
|---|---|
| Birth Year | Your birth year (1900–2150) |
| Birth Month | Your birth month (1–12) |
| Birth Day | Your birth day (1–31) |
| Live Until Year | Target year for the life bar's 100% mark (1900–2150) |

## Project Structure

```
src/c/main.c           — All watch-side C code (drawing, settings, tick handler, health)
src/pkjs/index.js      — PebbleKit JS entry point; Clay initialization
src/pkjs/config.js     — Clay settings page configuration
package.json           — App manifest (UUID, messageKeys, target platforms, capabilities)
wscript                — Waf build script
p5 sketch/             — Original p5.js prototype used to design the layout (not part of build)
```

## Building

Requires the [Pebble SDK](https://developer.rebble.io/developer.pebble.com/sdk/index.html) or [CloudPebble](https://cloudpebble.net/).

```bash
pebble build
```

The output is `build/pnooka.pbw` (bundle name set in `wscript`).

## Installing

Watch and phone must be on the same Wi-Fi network with Developer Mode enabled in the Pebble app:

```bash
pebble install --phone <your-phone-ip>
```

Find your phone's IP in the Pebble app under Settings → Developer Mode.

## Technical Notes

- The hour grid, bar positions, and font sizes are computed once at launch (`window_load`) and cached for the lifetime of the app. No layout work happens on the per-minute tick.
- Bar fractions (day/month/life) recompute only when the calendar day rolls over or settings change, not every minute.
- Step count refreshes every minute via `health_service_sum_today`, which is a stored-data lookup, not a sensor poll.
- All calendar math is integer-only (no floating point) since Pebble's Cortex-M has no FPU.
- Text outlines use a 4-direction (N/S/E/W) offset halo for legibility over both filled and empty bar portions.
- Colors arrive from Clay as 24-bit RGB and are converted to Pebble's 6-bit ARGB format via `rgb24_to_argb8()`. All incoming values are width-normalized through `tuple_to_int()` to handle PebbleKit JS's variable-width integer packing.
- Inbox buffer is 256 bytes (sufficient for 22 message keys); outbox is minimal since the watch never sends messages.

Disclaimer

The health-related features of this watchface, including the step-count display and color gradient, are provided for informational and motivational purposes only. They are not intended to constitute medical advice, diagnosis, or treatment, and should not be relied upon as a substitute for professional medical judgment. The step-count thresholds and color stops referenced in this project are derived from published epidemiological research but are simplified generalizations — they do not account for individual health conditions, age, mobility limitations, or other personal factors. No representation or warranty, express or implied, is made regarding the accuracy, completeness, or applicability of any health-related information displayed by this watchface. The author is not a licensed medical professional. Always consult a qualified healthcare provider before making decisions about your physical activity, health, or wellness. Use of this watchface is entirely at your own risk.

## Author

Alkali-V2
