#pragma once

namespace v1stdp::main::simulation::constant {

// #define MOD (70.0 / 126.0)
inline double constexpr MOD = (1.0 / 126.0);

// NOTE: Don't attempt to just modify the dt without reading the code below, as it will likely break things.
inline double constexpr dt = 1.0;

inline double constexpr BASEALTD = (14e-5 * 1.5 * 1.0);
inline double constexpr RANDALTD = 0.0;
inline double constexpr ALTP = (8e-5 * .008 * 1.0); // Note ALTPMULT below
inline double constexpr MINV = -80.0;
inline int constexpr TAUVLONGTRACE = 20000;
inline double constexpr LATCONNMULTINIT = 5.0; // The ALat coefficient; (12 * 36/100)

inline unsigned constexpr NBI = 20;
inline unsigned constexpr NBE = 100;

inline unsigned constexpr NBNEUR = (NBE + NBI);

inline double constexpr WFFINITMAX = .1;
inline double constexpr WFFINITMIN = 0.0;
inline double constexpr MAXW = 50.0;
inline double constexpr VSTIM = 1.0;

inline unsigned constexpr TIMEZEROINPUT = 100;
inline unsigned constexpr PRESTIMEMIXING = 350; // in ms
inline unsigned constexpr PRESTIMEPULSE = 350;
inline unsigned constexpr NBPATTERNSSPONT = 300;
inline unsigned constexpr PRESTIMESPONT = 1000;
inline unsigned constexpr PULSESTART = 0;

// #define NBPRESPERPATTERNLEARNING 30
inline unsigned constexpr NBMIXES = 30;

inline unsigned constexpr PATCHSIZE = 17;
inline unsigned constexpr FFRFSIZE = (2 * PATCHSIZE * PATCHSIZE);

// Inhibition parameters
// in ms
inline unsigned constexpr TAUINHIB = 10;
inline double constexpr ALPHAINHIB = .6;

// in KHz (expected number of thousands of VSTIM received per second through noise)
inline double constexpr NEGNOISERATE = 0.0;

// in KHz (expected number of thousands of VSTIM received per second through noise)
inline double constexpr POSNOISERATE = 1.8;

inline unsigned constexpr A = 4;
inline double constexpr B = .0805;
inline double constexpr Isp = 400.0;

inline double constexpr TAUZ = 40.0;
inline double constexpr TAUADAP = 144.0;
inline double constexpr TAUVTHRESH = 50.0;
inline double constexpr C = 281.0;
inline double constexpr Gleak = 30.0;
inline double constexpr Eleak = -70.6;

// in mV
inline double constexpr DELTAT = 2.0;

inline double constexpr VTMAX = -30.4;
inline double constexpr VTREST = -50.4;

// Also in mV
inline int constexpr VPEAK = 20;

inline double constexpr VRESET = Eleak;

// -45.3 //MINV // Eleak // VTMAX
inline double constexpr THETAVLONGTRACE = -45.3;

inline double constexpr MAXDELAYDT = 20;

// (3.0 / dt) // Number of steps that a spike should take - i.e. spiking
inline unsigned constexpr NBSPIKINGSTEPS = 1;

// time (in ms) / dt.
// (dt-.001)
inline unsigned constexpr REFRACTIME = 0;

inline double constexpr THETAVPOS = -45.3;
inline double constexpr THETAVNEG = Eleak;

// all 'tau' constants are in ms
inline double constexpr TAUXPLAST = 15.0;
inline double constexpr TAUVNEG = 10.0;
inline double constexpr TAUVPOS = 7.0;

// 70  // in mV^2
inline unsigned constexpr VREF2 = 50;

inline unsigned constexpr NBNOISESTEPS = 73333;

} // namespace v1stdp::main::simulation::constant
