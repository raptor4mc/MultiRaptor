#include "runtime/environment.h"
int main() { magphos::runtime::Environment e; e.set("k", "v"); return e.get("k") == "v" ? 0 : 1; }
