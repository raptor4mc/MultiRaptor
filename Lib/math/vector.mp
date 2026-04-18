fn vec2(x, y) {
  return [x, y]
}

fn add_vec2(a, b) {
  return [a[0] + b[0], a[1] + b[1]]
}

fn sub_vec2(a, b) {
  return [a[0] - b[0], a[1] - b[1]]
}

fn dot_vec2(a, b) {
  return (a[0] * b[0]) + (a[1] * b[1])
}
