fn set_new() {
  return objectCreate()
}

fn set_add(set, value) {
  return objectSet(set, toString(value), true)
}

fn set_has(set, value) {
  return objectGet(set, toString(value))
}
