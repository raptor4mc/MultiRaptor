fn random() {
  return __core_random()
}

fn random_int(min, max) {
  var raw = __core_random()
  var span = max - min + 1
  return min + (raw % span)
}
