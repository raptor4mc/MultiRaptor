fn square(x) {
  return x * x
}

fn cube(x) {
  return x * x * x
}

fn clamp(value, min, max) {
  when value < min {
    return min
  }

  when value > max {
    return max
  }

  return value
}
