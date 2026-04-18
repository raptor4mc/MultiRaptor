fn print(value) {
  return __core_print(value)
}

fn println(value) {
  __core_print(value)
  return __core_print("\n")
}
