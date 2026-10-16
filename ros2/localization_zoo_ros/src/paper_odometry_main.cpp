#include <string>

int runPaperOdometryNode(const std::string& method, int argc, char** argv);

int main(int argc, char** argv) {
  return runPaperOdometryNode(ZOO_PAPER_METHOD, argc, argv);
}
