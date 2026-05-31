# PNOOKA
#### *The P is silent.*

A minimal dot-matrix watchface for Pebble Time. The **P** stands for Pebble. The **NOOKA** is for [Nooka](https://www.nooka.com) — the watch brand whose abstract time-reading aesthetic inspired this design.

![PNOOKA banner](store_assets/banner_720x320.png)

## How to read it

- **4 columns × 3 rows of dots** show the current hour. Dots fill top-to-bottom, column by column — at 3 o'clock the leftmost column is fully lit, at 6 the first two columns are lit, and so on up to 12.
- **60-segment bar** below the dots shows the current minute. Each segment is one minute.
- **Date** (MM/DD/YY) in the bottom left, aligned with the bar.
- **AM · PM indicator** in the bottom right — the active period's dot is lit in the active color.

## Compatibility

| Platform | Device |
|---|---|
| Basalt | Pebble Time |
| Emery | Pebble Time 2 / Pebble Time Steel |

## Settings

Open the watchface settings in the Pebble app to customize:

| Setting | Description |
|---|---|
| Background | Background color |
| Active (dots & bar) | Color of filled hour dots and filled bar segments |
| Inactive (unfilled) | Color of empty hour dots and empty bar segments |
| Show Date | Toggle the date display on or off |

## Building

Requires the [Pebble SDK](https://developer.rebble.io/developer.pebble.com/sdk/index.html).

```bash
pebble build
```

The output is `build/pnooka.pbw`.

## Installing

**On a real watch** (watch and phone must be on the same Wi-Fi network, Developer Mode enabled in the Pebble app):

```bash
pebble install --phone <your-phone-ip>
```

Find your phone's IP in the Pebble app under Settings → Developer Mode.

## Support the project

If you enjoy PNOOKA, a small contribution is always appreciated:

- ☕ [Buy Me a Coffee](https://buymeacoffee.com/ryancady)
- 💳 [PayPal](https://paypal.me/ryancady)

## Author

Ryan Cady
