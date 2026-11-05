#pragma once
#include <QJsonObject>
#include <QSysInfo>
#include <QThread>
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <cpuid.h>
#endif
inline QJsonObject cpuCapabilities(){bool sse2=false,avx=false;
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
 unsigned a,b,c,d;if(__get_cpuid(1,&a,&b,&c,&d)){sse2=(d&bit_SSE2)!=0;avx=(c&bit_AVX)!=0;}
#elif defined(_M_X64)
 sse2=true;
#endif
 return {{"architecture",QSysInfo::currentCpuArchitecture()},{"logical_threads",QThread::idealThreadCount()},{"sse2",sse2},{"avx_hardware",avx},{"processing","portable C++ / x86-64 baseline; no AVX required"},{"os",QSysInfo::prettyProductName()}};
}
