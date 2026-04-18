fn read_all(handle) {
  return __core_stream_read_all(handle)
}

fn write(handle, chunk) {
  return __core_stream_write(handle, chunk)
}

fn close(handle) {
  return __core_stream_close(handle)
}
