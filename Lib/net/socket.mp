fn connect(host, port) {
  return tcpConnect(host, port)
}

fn send(socket, data) {
  return socketSend(socket, data)
}

fn recv(socket, max_bytes) {
  return socketRecv(socket, max_bytes)
}

fn close(socket) {
  return socketClose(socket)
}
