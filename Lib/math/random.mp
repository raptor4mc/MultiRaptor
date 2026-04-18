fn random_int(min, max) {
  var span = max - min + 1
  return min + floor(random() * span)
}
