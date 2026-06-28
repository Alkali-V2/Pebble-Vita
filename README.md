# Vita

A minimal square-matrix watchface for Pebble Time 2 that will also integrate with Pebble Health. I was inspired by the layout of Pnooka and the status bars of 'tens' which is what prompted my hybrid approach. 

## How to read it

- **4 columns × 3 rows of blocks** show the current hour. Blocks fill top-to-bottom, column by column — at 3 o'clock the leftmost column is fully lit, at 6 the first two columns are lit, and so on up to 11:59am or 11:59pm which clears all the blocks.
- **Minute Progress Blocks** The upcoming hour block fills up as minutes progress and number of exact minute can be shown (optional) if preferred
- **Date** (MM/DD/YY) in the bottom left, aligned with the bar.
- **AM · PM indicator** The "Minutes" text can be edited for AM and PM indication set with border and fill color for matching and contrast purposes.

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
| Show Minute Number | Make number visible in progressing block|
| AM Minute Text | Set text color for the AM text|
| AM Border Color | Set the text color for the AM border|
| PM Minute Text | Set text color for the PM text|
| PM Border Color | Set the text color for the PM border|

## Building

Requires the [Pebble SDK](https://developer.rebble.io/developer.pebble.com/sdk/index.html).

```bash
pebble build
```

The output is `build/vita.pbw`.

## Installing

**On a real watch** (watch and phone must be on the same Wi-Fi network, Developer Mode enabled in the Pebble app):

```bash
pebble install --phone <your-phone-ip>
```

Find your phone's IP in the Pebble app under Settings → Developer Mode.

## Support the project
If you like it but want more, fork it on Github and make your own. If you somehow think I nailed it, give me a heart in the Pebble store. If you hate it, well, that's what reddit is for I guess. Thanks for reading!

## Author

Alkali-V2
