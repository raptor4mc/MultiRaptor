fn trim(value) {
  return regexReplace(value, "^\\s+|\\s+$", "")
}

fn starts_with(value, prefix) {
  if len(prefix) > len(value) {
    return false
  }

  return substring(value, 0, len(prefix)) == prefix
}

fn ends_with(value, suffix) {
  if len(suffix) > len(value) {
    return false
  }

  var start = len(value) - len(suffix)
  return substring(value, start, len(suffix)) == suffix
}

fn contains(value, needle) {
  if needle == "" {
    return true
  }

  return len(replace(value, needle, "")) != len(value)
}
