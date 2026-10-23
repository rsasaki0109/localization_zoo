// Continuous native evaluation of bounded scan batches. stdin is little-endian
// uint32 frame, uint32 point_count, then raw KITTI float32 XYZI. No GT interface.
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
extern "C" std::size_t preprocess(float*, std::size_t);
using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;
int main() {
  try {
    InTenLoamPaperParams p;
    p.accelerated = true;
    p.max_range = 120.0;
    p.s2m_plane_max_dist = .05;
    p.s2s_mapped_prior = true;
    p.s2s_translation_prior_ratio = .03;
    p.s2s_ground_pca = true;
    p.s2s_facade_pca = true;
    InTenLoamPaper reference(p);
    p.s2s_geometric_prior = true;
    InTenLoamPaper candidate(p);
    uint32_t expected = 0;
    while (true) {
      uint32_t frame, count;
      std::cin.read(reinterpret_cast<char*>(&frame), sizeof(frame));
      if (std::cin.eof() && std::cin.gcount() == 0) break;
      if (!std::cin.read(reinterpret_cast<char*>(&count), sizeof(count)) || frame != expected || count == 0 || count > 10000000)
        throw std::runtime_error("invalid frame header/order");
      std::vector<float> raw(4*count);
      if (!std::cin.read(reinterpret_cast<char*>(raw.data()), raw.size()*sizeof(float)))
        throw std::runtime_error("truncated scan");
      const auto kept = preprocess(raw.data(), count);
      std::vector<Eigen::Vector3d> points;
      std::vector<float> intensity;
      points.reserve(kept); intensity.reserve(kept);
      for (size_t i = 0; i < kept; ++i) {
        points.emplace_back(raw[4*i],raw[4*i+1],raw[4*i+2]);
        intensity.push_back(raw[4*i+3]);
      }
      if (intensity.empty()) throw std::runtime_error("empty processed scan");
      if (*std::max_element(intensity.begin(), intensity.end()) > 1.0f)
        for (auto& v : intensity) v /= 255.0f;
      InTenLoamPaperTrace rt, ct;
      const auto start = std::chrono::steady_clock::now();
      const auto r = reference.process(points,intensity,nullptr,&rt);
      const auto middle = std::chrono::steady_clock::now();
      const auto c = candidate.process(points,intensity,nullptr,&ct);
      const auto end = std::chrono::steady_clock::now();
      if (!r.pose.allFinite() || !c.pose.allFinite()) throw std::runtime_error("nonfinite output");
      auto out = serial::record();
      out << "{\"frame\":" << frame << ",\"input_points\":" << kept;
      out << ",\"reference_seconds\":" << std::chrono::duration<double>(middle-start).count();
      out << ",\"candidate_seconds\":" << std::chrono::duration<double>(end-middle).count();
      auto result = [&](const char* name,const InTenLoamPaperTrace& t) {
        out << ",\"" << name << "\":{\"pose\":"; serial::matrix(out,t.pose);
        out << ",\"s2s_input\":"; serial::matrix(out,t.s2s.input);
        out << ",\"s2s_output\":"; serial::matrix(out,t.s2s.output);
        out << ",\"s2s_fallback\":" << t.s2s_fallback;
        out << ",\"s2m_accepted\":" << t.s2m_accepted;
        out << ",\"geometric_prior_used\":" << t.geometric_prior_used;
        out << ",\"geometric_relative\":"; serial::matrix(out,t.geometric_relative);
        out << '}';
      };
      result("reference",rt);result("candidate",ct);
      out << '}'; std::cout << out.str() << '\n' << std::flush;
      if (!std::cout) throw std::runtime_error("output failure");
      ++expected;
    }
    return 0;
  } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
