fn exists(path) {
  return fileExists(path)
}

fn remove(path) {
  return exec("rm -f " + path)
}

fn mkdir(path) {
  return exec("mkdir -p " + path)
}
