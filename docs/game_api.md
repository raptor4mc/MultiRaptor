# MagPhos Game / Graphics API (Preview)

Current runtime preview built-ins:

- `canvasCreate(width, height)`
- `inputIsKeyDown(key)`
- `timerNowMs()`
- `spriteLoad(path)`
- `spriteDraw(canvas, sprite, x, y)`
- `audioPlay(path)`

These are foundational placeholders for browser/native game API layers.

## Browser direction
- Canvas-backed rendering in playground/web targets.
- Input events for keyboard/mouse/touch.

## Native direction
- Raylib or SDL2 binding layer.
- Sprite batching and audio mixer integration.
