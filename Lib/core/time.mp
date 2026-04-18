fn now_ms() {
  return time()
}

fn sleep_ms(duration) {
  var end = time() + duration
  while time() < end {
  }
  return duration
}
