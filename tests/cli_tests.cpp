#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/wait.h>

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

int exitCodeOf(int rc) {
    if (rc == -1) return -1;
#ifdef WEXITSTATUS
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);
#endif
    return rc;
}
}

int main() {
    const std::string sourcePath = "/tmp/magphos_cli_test.mp";
    const std::string brokenPath = "/tmp/magphos_cli_broken.mp";
    const std::string runtimeFailPath = "/tmp/magphos_cli_runtime_fail.mp";
    const std::string emptyPath = "/tmp/magphos_cli_empty.mp";
    {
        std::ofstream out(sourcePath);
        out << "import game.engine\n";
        out << "print \"Hello\"\n";
    }
    {
        std::ofstream out(brokenPath);
        out << "print \"Hello\"\n";
        out << "if {\n";
    }
    {
        std::ofstream out(runtimeFailPath);
        out << "var x = 10\n";
        out << "var y = 0\n";
        out << "print x / y\n";
    }
    {
        std::ofstream out(emptyPath);
    }

    int rc = std::system(("/tmp/magphos_tests/magphos_cli --check " + sourcePath + " >/tmp/cli_check.txt 2>/tmp/cli_check_err.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_check.txt").find("ok") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --deps " + sourcePath + " >/tmp/cli_deps.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_deps.txt").find("game.engine") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --tokens " + sourcePath + " >/tmp/cli_tokens.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_tokens.txt").find("import") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --check --json " + sourcePath + " >/tmp/cli_check_json.txt 2>/tmp/cli_check_json_err.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_check_json.txt").find("\"ok\":true") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --check --json " + emptyPath + " >/tmp/cli_check_empty_json.txt 2>/tmp/cli_check_empty_json_err.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_check_empty_json.txt").find("\"ok\":true") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --check --json " + brokenPath + " >/tmp/cli_check_bad_json.txt 2>/tmp/cli_check_bad_json_err.txt").c_str());
    assert(exitCodeOf(rc) == 3);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"ok\":false") != std::string::npos);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"errors\"") != std::string::npos);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"errorDomain\":\"parser\"") != std::string::npos);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"errorCode\":\"PARSER_PARSE_ERROR\"") != std::string::npos);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"traceId\":\"trace-") != std::string::npos);
    assert(readFile("/tmp/cli_check_bad_json.txt").find("\"logs\":[") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --deps --json " + sourcePath + " >/tmp/cli_deps_json.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_deps_json.txt").find("\"deps\":[\"game.engine\"]") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --tokens --json " + sourcePath + " >/tmp/cli_tokens_json.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_tokens_json.txt").find("\"tokens\":[") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --run " + sourcePath + " >/tmp/cli_run.txt 2>/tmp/cli_run_err.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_run.txt").find("Hello") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --module-graph --json " + sourcePath + " >/tmp/cli_module_graph_json.txt").c_str());
    assert(exitCodeOf(rc) == 0);
    assert(readFile("/tmp/cli_module_graph_json.txt").find("\"moduleGraph\"") != std::string::npos);
    assert(readFile("/tmp/cli_module_graph_json.txt").find("game.engine") != std::string::npos);

    rc = std::system(("/tmp/magphos_tests/magphos_cli --run --json " + runtimeFailPath + " >/tmp/cli_run_runtime_fail_json.txt 2>/tmp/cli_run_runtime_fail_json_err.txt").c_str());
    assert(exitCodeOf(rc) == 3);
    assert(readFile("/tmp/cli_run_runtime_fail_json.txt").find("\"runtimeErrorCode\":\"RUNTIME_FAILURE\"") != std::string::npos);
    assert(readFile("/tmp/cli_run_runtime_fail_json.txt").find("\"errorDomain\":\"runtime\"") != std::string::npos);
    assert(readFile("/tmp/cli_run_runtime_fail_json.txt").find("\"errorCode\":\"RUNTIME_EXECUTION_ERROR\"") != std::string::npos);
    assert(readFile("/tmp/cli_run_runtime_fail_json.txt").find("\"stackTrace\":[\"magphos_cli::--run\"") != std::string::npos);
    assert(readFile("/tmp/cli_run_runtime_fail_json.txt").find("Division by zero") != std::string::npos);

    rc = std::system("/tmp/magphos_tests/magphos_cli --check /tmp/does_not_exist.mp >/tmp/cli_missing.txt 2>/tmp/cli_missing_err.txt");
    assert(exitCodeOf(rc) == 2);

    rc = std::system("/tmp/magphos_tests/magphos_cli --unknown >/tmp/cli_unknown.txt 2>/tmp/cli_unknown_err.txt");
    assert(exitCodeOf(rc) == 2);

    return 0;
}
