#include <stdio.h>
#include <stdlib.h>
//#include <complex.h>
#include <math.h>

#define MAX_THREADS (1024)                                              // Maximum number of threads in a block
#define N_POL (2) //2                                                   // Number of polarizations
#define N_TIME (131072) // (1024) // 8192 //16384 //1 // 8              // Number of time samples
#define N_STI_BLOC (64)                                                 // 64 to accomodate both MeerKAT and VLA specifications. It was 32 for MeerKAT specs.
#define N_STREAMS (1)                                                   // Number of CUDA streams
//#define N_COARSE_FREQ 64 //32                                         // Number of coarse channels processed at a time
#define MAX_COARSE_FREQ (32) // (512)                                   // Max number of coarse channels is the number of channels in 32k mode
#define N_FINE_FREQ (1) //16384                                         // Number of fine channels per coarse channel 2^14 = 16384
//#define N_FREQ (N_COARSE_FREQ*N_FINE_FREQ) // Number of frequency bins after second FFT.  Should actually be 2^14, but due to limited memory on my laptop, arbitrarily 10
#define N_FREQ (MAX_COARSE_FREQ*N_FINE_FREQ)                            // Number of frequency bins after second FFT.  Should actually be 2^14, but due to limited memory on my laptop, arbitrarily 10
//#define N_FREQ_STREAM (N_FREQ/N_STREAMS) // (N_COARSE_FREQ*N_FINE_FREQ)/N_STREAMS // Number of frequency bins after second FFT.  Should actually be 2^14, but due to limited memory on my laptop, arbitrarily 10
//#define N_FREQ_STREAM (N_COARSE_FREQ/N_STREAMS) // (N_COARSE_FREQ*N_FINE_FREQ)/N_STREAMS // Number of frequency bins after second FFT.  Should actually be 2^14, but due to limited memory on my laptop, arbitrarily 10
#define N_ANT (64) // 64                                                // Number of possible antennas (64 is also computationally efficient since it is a multiple of 32 which is the size of a warp)
#define N_BEAM (64) // 64                                               // Number of beams

// Beamformer Recipe file dimensions depending on observatory and mode of operation
#define NANTS_MK        64
#define NANTS_VLA       32
#define NPOL_BFR5       2
#define NCHAN_MK1K      1024  // channels per node * number of nodes = 16*64 for MK
#define NCHAN_MK4K      4096  // channels per node * number of nodes = 64*64 for MK
#define NCHAN_MK32K     32768 // channels per node * number of nodes = 512*64 for MK
#define NCHAN_VLA       1024  // channels per node * number of nodes = 32*32 for COSMIC
#define NTIMES_BFR5     32768 //30
#define NTIMES_VLA_BFR5 151 //30
#define NBEAMS_MK       64
#define NBEAMS_VLAD     31
#define NBEAMS_VLAR     5

// "2" for inphase and quadrature
#define N_INPUT       (unsigned long int)(2*N_POL*N_TIME*N_FREQ*N_ANT)                  // Size of input. Currently, same size as output
#define N_COEFF       (unsigned long int)(2*N_POL*N_ANT*N_BEAM*N_FREQ)                  // Size of beamformer coefficients
#define DELAY_POLYS   (unsigned long int)(2)                                            // Number of coefficients in polynomial
#define N_DELAYS      (unsigned long int)(DELAY_POLYS*N_ANT*N_BEAM)                     // Size of first order polynomial delay array
#define N_OUTPUT      (unsigned long int)(2*N_POL*(N_BEAM)*N_FREQ*N_TIME)                 // Size of beamformer output
#define N_FFT         (16384)                                                           // N-points of FFT in 32k mode
#define N_BF_POW      (unsigned long int)((N_BEAM+1)*N_FREQ*N_FFT)                          // Size of beamformer output after abs()^2 and short time integration


// VLASS specs
#define VLASS_N_TIME (10240000) // (5120000) // 1024000 with desired specs (~10 seconds) and 5120000 with required specs (~5 seconds)
#define VLASS_N_FREQ (1) // 1 of 32 coarse channels
#define VLASS_N_FFT  (128000) // Lowest frequency resolution
#define VLASS_N_BEAM (32) // Max number of beams
#define VLASS_N_WIN  (VLASS_N_TIME/VLASS_N_FFT) // Number of spectral windows

#define VLASS_N_INPUT       (unsigned long int)(2*N_POL*VLASS_N_TIME*VLASS_N_FREQ*(N_ANT/2))              // Size of input. Currently, same size as output
#define VLASS_N_COEFF       (unsigned long int)(2*N_POL*(N_ANT/2)*VLASS_N_BEAM*VLASS_N_FREQ)              // Size of beamformer coefficients
#define VLASS_N_BF_POW      (unsigned long int)((VLASS_N_BEAM+1)*VLASS_N_FREQ*VLASS_N_FFT*VLASS_N_WIN)        // Size of beamformer output after abs()^2 and short time integration

#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif
#ifndef max
#define max(a,b) ((a > b) ? a : b)
#endif

#define PI 3.14159265

// p - polarization index
// t - time sample index
// w - time window index
// c - coarse channel frequency index
// f - fine channel frequency index
// a - antenna index
// b - beam index

#define data_in_idx(p, t, w, c, a, Np, Nt, Nw, Nc)           ((p) + (Np)*(t) + (Nt)*(Np)*(w) + (Nw)*(Nt)*(Np)*(c) + (Nc)*(Nw)*(Nt)*(Np)*(a))
//#define data_in_idx(p, t, w, a, c, Np, Nt, Nw, Na)           ((p) + (Np)*(t) + (Nt)*(Np)*(w) + (Nw)*(Nt)*(Np)*(a) + (Na)*(Nw)*(Nt)*(Np)*(c))
#define data_raw_idx(p, t, c, a, Np, Nt, Nc)                 ((p) + (Np)*(t) + (Nt)*(Np)*(c) + (Nc)*(Nt)*(Np)*(a))
//#define data_raw_idx(p, t, b, c, a, Np, Nt, Nb, Nc)          ((p) + (Np)*(t) + (Nt)*(Np)*(b) + (Nb)*(Nt)*(Np)*(c) + (Nc)*(Nb)*(Nt)*(Np)*(a))
//#define data_in_idx(p, c, a, t, w, Np, Nc, Na, Nt)           ((p) + (Np)*(c) + (Nc)*(Np)*(a) + (Na)*(Nc)*(Np)*(t) + (Nt)*(Na)*(Nc)*(Np)*(w))
#define data_tr_idx(t, a, p, c, w, Nt, Na, Np, Nc)           ((t) + (Nt)*(a) + (Na)*(Nt)*(p) + (Np)*(Na)*(Nt)*(c) + (Nc)*(Np)*(Na)*(Nt)*(w))
#define data_fft_out_idx(f, a, p, c, w, Nf, Na, Np, Nc)      ((f) + (Nf)*(a) + (Na)*(Nf)*(p) + (Np)*(Na)*(Nf)*(c) + (Nc)*(Np)*(Na)*(Nf)*(w))
// The "Nf" below is equal in value to "Nt*Nc" that is the dimension of "t" since this is the number of FFT points muliplied by the number of coarse channels
#define data_fftshift_idx(a, p, f, c, w, Na, Np, Nf, Nc)     ((a) + (Na)*(p) + (Np)*(Na)*(f) + (Nf)*(Np)*(Na)*(c) + (Nc)*(Nf)*(Np)*(Na)*(w))
#define coeff_idx(a, p, b, f, Na, Np, Nb)                    ((a) + (Na)*(p) + (Np)*(Na)*(b) + (Nb)*(Np)*(Na)*(f))
//#define phase_idx(a, p, f, Np)                               ((a) + (N_ANT)*(p) + (Np)*(N_ANT)*(f))
//#define delay_idx(d, a, b, Na)                               ((d) + DELAY_POLYS*(a) + DELAY_POLYS*(Na)*(b)) // Should be correct indexing
#define cal_all_idx(a, p, f, Na, Np)                         ((a) + (Na)*(p) + (Np)*(Na)*(f))
#define delay_idx(a, b, t, Na, Nb)                           ((a) + (Na)*(b) + (Nb)*(Na)*(t))
#define coh_bf_idx(p, b, f, c, w, Np, Nb, Nc, Nf)            ((p) + (Np)*(b) + (Nb)*(Np)*(f) + (Nf)*(Nb)*(Np)*(c) + (Nc)*(Nf)*(Nb)*(Np)*(w))
#define pow_bf_idx(f, c, s, b, Nf, Nc, Ns)                   ((f) + (Nf)*(c) + (Nc)*(Nf)*(s) + (Ns)*(Nc)*(Nf)*(b)) // Changed to efficiently write each beam to a filterbank file

typedef struct complex_t{
	float re;
	float im;
}complex_t;

signed char* simulate_data_ubf(int n_sim_ant, int nants, int n_pol, int n_chan, int nt, int n_win, int sim_flag, int telescope_flag, float rect_zero_samps, float freq_band_shift, int chan_idx, int filenum, int num_files, int just_noise);
float* simulate_coefficients_ubf(int n_sim_ant, int nants, int n_pol, int max_n_beams, int actual_n_beams, int n_chan, int sim_flag, int telescope_flag);
double *delay_bfr5(int nants, int n_beam, int sim_flag, int telescope_flag);
