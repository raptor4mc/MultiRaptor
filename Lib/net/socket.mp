fn connect(host, port) {
  return __core_tcp_connect(host, port)
}

fn send(socket, data) {
  return __core_socket_send(socket, data)
}

fn recv(socket, max_bytes) {
  return __core_socket_recv(socket, max_bytes)
}

fn close(socket) {
  return __core_socket_close(socket)
}
