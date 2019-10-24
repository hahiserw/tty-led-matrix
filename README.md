## Features
- unicode (see the font in console.c for all available glyphs)
- multiple scrolling windows displayed simultanuously
- handling of escape sequences (in progress)
- drawing images with base64 (not yet)

## Youtube demos

Split scrolling demo

[![Split scrolling demo](http://img.youtube.com/vi/6Mvoc2WfrNE/0.jpg)](http://www.youtube.com/watch?v=6Mvoc2WfrNE)

Vertical scrolling and scrolling speed demo

[![Vertical scrolling and scrolling speed demo](http://img.youtube.com/vi/mPno4U57v9A/0.jpg)](http://www.youtube.com/watch?v=mPno4U57v9A)


### TODO
- [X] FIXME scrolling offsets / displaying data
- [ ] scan() should be called by an interrupt, so leds won't blink when receiving data?
- [ ] send some character / escape code when text scrolled the full cycle?
- [ ] handle usb suspend
  https://forum.pjrc.com/threads/27115-SNOOZE-to-return-wakeup-interrupt
- [ ] blinking cursor?
- [X] scroll to buffer-pre on new message (if it exceeds screen width)
- [ ] respond with available escape sequences when incorrect one was received?
- [ ] different configurations of windows to switch between (changable by some escape sequence) during runtime

- [ ] text buffer, so screen doesn't blink on receiving
- [ ] display buffere repeatedly, so there's no blank screen at any time
