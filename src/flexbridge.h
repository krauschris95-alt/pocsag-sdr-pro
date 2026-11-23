#pragma once
#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <complex>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <array>

namespace paging {
struct FlexPage {
 quint64 address=0;
 int baud=0, levels=0;
 QString type, text;
};

// Receive-only FLEX frontend. A separate, pinned multimon-ng FLEX_NEXT
// executable validates/decodes FLEX frames. This bridge does NOT decode
// ReFLEX and must never advertise that capability.
class FlexBridge {
public:
 using PageHandler=std::function<void(FlexPage)>;
 using ErrorHandler=std::function<void(QString)>;
 FlexBridge(double iqRate, PageHandler onPage, ErrorHandler onError);
 ~FlexBridge();
 bool start();
 static std::optional<FlexPage> parseValidatedJson(const QByteArray& line);
 void submit(const QByteArray& cu8);
 void stop();
private:
 void run();
 void processIq(const QByteArray& cu8,QByteArray& pcm);
 void demod48(const std::complex<double>& sample,QByteArray& pcm);
 void appendPcm(double normalized,QByteArray& pcm);
 void parseOutput(const QByteArray& bytes);
 void parseLine(const QByteArray& line);
 double iqRate_;
 PageHandler onPage_;
 ErrorHandler onError_;
 std::thread thread_;
 std::mutex mutex_;
 std::condition_variable wake_;
 std::deque<QByteArray> queue_;
 std::size_t queuedBytes_=0;
 bool closing_=false,started_=false,overflowReported_=false;
 std::array<std::complex<double>,4> lowpass_{};
 std::complex<double> previous48_{};
 bool have48_=false;
 double inputPhase_=0,dc_=0,lastAudio48_=0;
 quint64 audio48Index_=0, nextAudioTick_=0;
 QByteArray outputLines_;
};
}
