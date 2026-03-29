#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}
}

int main() {
    const std::string sourcePath = "/tmp/magphos_cli_test.mp";
    {
        std::ofstream out(sourcePath);
        out << "import game.engine\n";
        out << "print 1\n";
    }

    int rc = std::system(("/tmp/magphos_tests/magphos_cli --check " + sourcePath + " >/tmp/cli_check.txt 2>/tmp/cli_check_err.txt").c_str());
    assert(rc == 0);
    assert(readFile("/tmp/cli_check.txt").find("ok") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --deps " + sourcePath + " >/tmp/cli_deps.txt").c_str());
    assert(rc == 0);
    assert(readFile("/tmp/cli_deps.txt").find("game.engine") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --tokens " + sourcePath + " >/tmp/cli_tokens.txt").c_str());
    assert(rc == 0);
    assert(readFile("/tmp/cli_tokens.txt").find("import") != std::string::npos);

    return 0;
}
