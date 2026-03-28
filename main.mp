//!
this is a block comment
!/
/! MagPhos example (.mp)
var x = 5
var y = 7
var total = 0

fn add(a, b) {
  return a + b
}

when x < y {
  print "x is smaller"
}

loop 3 {
  set total = total + add(x, y)
}

print "total=" + total
