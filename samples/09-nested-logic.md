# Sample 09 — Nested Logic

## Preview
Demonstrates nested `if/else` for grading.

## Code
```mp
var score = 82
if score >= 90 {
  print "Grade A"
}
else {
  if score >= 80 {
    print "Grade B"
  }
  else {
    print "Grade C or lower"
  }
}
```

## Expected Output
```text
Grade B
```
