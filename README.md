## Features
- unicode (see the font in console.c for all available glyphs)
- multiple scrolling windows displayed simultanuously
- handling of escape sequences (not yet; in progress)
- drawing images with base64 (not yet)

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
