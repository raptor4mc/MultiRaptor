fn read_all(handle) {
  return readFile(handle)
}

fn write(handle, chunk) {
  return appendFile(handle, chunk)
}

fn close(handle) {
  return 0
}
