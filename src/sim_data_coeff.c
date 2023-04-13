#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
// #include <iostream>
#include <string.h>
#include <math.h>
#include "sim_data_coeff.h"

// Generate simulated data
signed char *simulate_data_ubf(int n_sim_ant, int nants, int n_pol, int n_chan, int nt, int n_win, int sim_flag, int telescope_flag, float rect_zero_samps, float freq_band_shift, int chan_idx, int filenum, int num_files, int just_noise)
{
	printf("In sim_data_ubf\n");
	unsigned long int n_input = 0;
	int n_ant_config = 0;
	unsigned long int n_subbands = 0;
	if (telescope_flag == 0)
	{
		n_input = N_INPUT;
		n_ant_config = N_ANT;
	}
	else if (telescope_flag == 1)
	{
		n_input = VLASS_N_INPUT;
		n_ant_config = N_ANT / 2;
	}

	signed char *data_sim;
	data_sim = (signed char *)calloc(n_input, sizeof(signed char));

	/*
	'sim_flag' is a flag that indicates the kind of data that is simulated.
	sim_flag = 0 -> Ones
	sim_flag = 1 -> Ones placed in a particular bin (bin 3 for now)
	sim_flag = 2 -> Ones placed in a particular bin at a particular antenna (bin 3 and antenna 3 for now)
	sim_flag = 3 -> Rect placed in a particular bin at a particular antenna (bin 3 and antenna 3 for now)
	sim flag = 4 -> Simulated cosine wave
	sim flag = 5 -> Simulated complex exponential i.e. exp(j*2*pi*f0*t)
	sim flag = 6 -> Simulated drifting signal or complex exponential i.e. exp(j*2*pi*f0*t)
	sim_data = 7 -> Simulated drifting signal that simulates ETI given a particular observatory and mode over a all time samples/blocks in a GUPPI RAW file
	sim flag = 3 -> Simulated radio source in center beam assuming ULA
	*/
	// int sim_flag = 5;
	if (sim_flag == 0)
	{
		for (int i = 0; i < (n_input / 2); i++)
		{
			if (i < ((n_sim_ant * n_input) / (2 * n_ant_config)))
			{
				data_sim[2 * i] = 1;
			}
			else
			{
				data_sim[2 * i] = 0;
			}
		}
	}
	if (sim_flag == 1)
	{
		for (int p = 0; p < n_pol; p++)
		{
			for (int t = 0; t < nt; t++)
			{
				for (int a = 0; a < nants; a++)
				{
					if (a < n_sim_ant)
					{
						data_sim[2 * data_in_idx(p, t, 0, a, 2, n_pol, nt, n_win, nants)] = 1;
					}
				}
			}
		}
	}
	if (sim_flag == 2)
	{
		for (int p = 0; p < n_pol; p++)
		{
			for (int t = 0; t < nt; t++)
			{
				data_sim[2 * data_in_idx(p, t, 0, 2, 2, n_pol, nt, n_win, nants)] = 1;
			}
		}
	}
	if (sim_flag == 3)
	{
		// data_in_idx(p, t, w, c, a, Np, Nt, Nw, Nc)
		for (int p = 0; p < n_pol; p++)
		{
			for (int t = (1024 * 10); t < (nt - (1024 * 10)); t++)
			{
				// data_sim[2 * data_in_idx(p, 0, 2, t, 0, n_pol, n_chan, nants, nt)] = 1;
				// data_sim[2 * data_in_idx(p, 1, 2, t, 0, n_pol, n_chan, nants, nt)] = 1;
				data_sim[2 * data_in_idx(p, t, 0, 2, 2, n_pol, nt, n_win, nants)] = 1;
				// data_sim[2 * data_in_idx(p, 3, 2, t, 0, n_pol, n_chan, nants, nt)] = 1;

				data_sim[2 * data_in_idx(p, t, 0, 2, 2, n_pol, nt, n_win, nants) + 1] = 1;
			}
		}
	}
	if (sim_flag == 4)
	{
		float freq = 1e3; // Resonant frequency

		float tmp_max = 1.0;
		float tmp_min = -1.0;

		for (int t = 0; t < nt; t++)
		{
			for (int f = 0; f < n_chan; f++)
			{
				for (int a = 0; a < nants; a++)
				{
					if (a < n_sim_ant)
					{
						// Requantize from doubles/floats to signed chars with a range from -128 to 127
						// X polarization
						data_sim[2 * data_in_idx(0, t, 0, a, f, n_pol, nt, n_win, nants)] = (signed char)((((cos(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
						// data_sim[2 * data_in_idx(0, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
						//  Y polarization
						data_sim[2 * data_in_idx(1, t, 0, a, f, n_pol, nt, n_win, nants)] = (signed char)((((2 * cos(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
						// data_sim[2 * data_in_idx(1, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
					}
				}
			}
		}
	}
	if (sim_flag == 5)
	{
		float freq = 1e3; // Resonant frequency
		float tmp_max = 1.0;
		float tmp_min = -1.0;

		for (int w = 0; w < n_win; w++)
		{
			for (int t = 0; t < nt; t++)
			{
				for (int f = 0; f < n_chan; f++)
				{
					for (int a = 0; a < nants; a++)
					{
						if (a < n_sim_ant)
						{
							// Requantize from doubles/floats to signed chars with a range from -128 to 127
							// X polarization
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((cos(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(0, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
							//  Y polarization
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((2 * cos(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * freq * t * 0.000001) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(1, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
						}
					}
				}
			}
		}
	}
	if (sim_flag == 6)
	{
		printf("Sim flag 6\n");
		float freq = 1e9; // Resonant frequency
		// float shift_freq = 0; // Shifting frequency depending on time sample
		float sig_shift = 0;
		float tmp_max = 1.0;
		float tmp_min = -1.0;
		// int freq_band_shift = 10000;

		for (int w = 0; w < n_win; w++)
		{
			for (int t = rect_zero_samps; t < (nt - rect_zero_samps); t++)
			// for (int t = 0; t < nt; t++)
			{
				for (int f = 0; f < n_chan; f++)
				{
					for (int a = 0; a < nants; a++)
					{
						if (a < n_sim_ant)
						{
							sig_shift = freq_band_shift * (((float)t / nt) + w);
							// Requantize from doubles/floats to signed chars with a range from -128 to 127
							// X polarization
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((cos(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(0, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
							//  Y polarization
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((2 * cos(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(1, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
						}
					}
				}
			}
		}
	}
	if (sim_flag == 7)
	{
		printf("Sim flag 7\n");
		float freq = 1e9; // Resonant frequency
		// float shift_freq = 0; // Shifting frequency depending on time sample
		float sig_shift = 0;
		float tmp_max = 1.0;
		float tmp_min = -1.0;
		// int freq_band_shift = 10000;

		for (int w = 0; w < n_win; w++)
		{
			for (int t = rect_zero_samps; t < (nt - rect_zero_samps); t++)
			// for (int t = 0; t < nt; t++)
			{
				for (int f = 0; f < n_chan; f++)
				{
					for (int a = 0; a < nants; a++)
					{
						if (a < n_sim_ant)
						{
							sig_shift = freq_band_shift * (((float)t / nt) + (w + n_win * filenum));
							// Requantize from doubles/floats to signed chars with a range from -128 to 127
							// X polarization
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((cos(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(0, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(0, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
							//  Y polarization
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants)] = (signed char)((((2 * cos(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							data_sim[2 * data_in_idx(1, t, w, a, f, n_pol, nt, n_win, nants) + 1] = (signed char)((((sin(2 * PI * (freq + sig_shift) * t) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
							// data_sim[2 * data_in_idx(1, f, a, t, 0, n_pol, n_chan, nants, nt) + 1] = 0;
						}
					}
				}
			}
		}
	}
	if (sim_flag == 8)
	{
		printf("Sim flag 8\n");
		float c = 3e8;					  // Speed of light
		float c_freq = 1504.5830078125e6; // 1.25e9;	   // Center frequency (Hz)
		float lambda = c / c_freq;		  // Wavelength
		float d = lambda / 2;			  // Distance between antennas
		float chan_band = 1e6;			  // Coarse channel bandwidth in Hz
		// float fine_chan_band = chan_band / nt; // Fine channel bandwidth in Hz
		double theta = 0;	// SOI direction/angle of arrival
		double tau = 0;		// Delay
		double tbin = 1e-6; // Time between time samples in seconds
		double pulse_length = 5000e-6;
		int pulse_samps = (int)floor(pulse_length / tbin);
		int zero_samps = (int)(nt - pulse_samps) / 2;
		double rf_freqs = 0;
		double cb = 90; // Center beam in degrees

		float tmp_max = 1.0;
		float tmp_min = -1.0;
		double deg_per_tsamp = 0;

		// Noise //
		double *noise_sim;
		noise_sim = (double *)calloc(n_input, sizeof(double));
		// int upper = 100;
		// int lower = 1;
		int rand_range = 1000;
		double noise_realx = 0;
		double noise_imagx = 0;
		double noise_realy = 0;
		double noise_imagy = 0;

		int pulse = 1; // Amplitude of the pulse

		// Use current time as  seed for random generator
		srand(time(0));
		int raw_files = 1; // If set to 1, then there are RAW files being generated so theta might need to spread over RAW files. If set to 0, there are no RAW files being generated
		float noise_amp = 0;
		float noise_amp2 = 0;
		for (int w = 0; w < n_win; w++)
		{
			for (int t = 0; t < nt; t++)
			{
				for (int f = 0; f < n_chan; f++)
				{
					if (nt > 1)
					{
						if (f == chan_idx)
						{
							noise_amp = 0.00000001;
						}
						else
						{
							noise_amp = 0.00000001;
						}
					}
					else if (nt == 1)
					{
						noise_amp = 0.00000001;
					}
					for (int a = 0; a < nants; a++)
					{
						if (n_pol == 1)
						{
							if (a < n_sim_ant)
							{
								//// X polarization
								// noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % (upper - lower + 1)) + lower);
								// noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % (upper - lower + 1)) + lower);

								// X polarization
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % 2 * rand_range) - rand_range);
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % 2 * rand_range) - rand_range);
							}
							else
							{
								// X polarization
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0;
							}
						}
						else if (n_pol == 2)
						{
							if (a < n_sim_ant)
							{
								//// X polarization
								// noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % (upper - lower + 1)) + lower);
								// noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % (upper - lower + 1)) + lower);
								//// Y polarization
								// noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % (upper - lower + 1)) + lower);
								// noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % (upper - lower + 1)) + lower); // Make this negative if a different polarization is tested

								// X polarization
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % 2 * rand_range) - rand_range);
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % 2 * rand_range) - rand_range);
								// Y polarization
								noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = noise_amp * ((rand() % 2 * rand_range) - rand_range);
								noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = noise_amp * ((rand() % 2 * rand_range) - rand_range); // Make this negative if a different polarization is tested
							}
							else
							{
								// X polarization
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0;
								// Y polarization
								noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0; // Make this negative if a different polarization is tested
							}
						}
					}
				}
			}
		}
		// -----------------//

		for (int w = 0; w < n_win; w++)
		{
			for (int t = 0; t < nt; t++)
			{
				// Reduce the range of angles in order to prevent wrap around - That's what the 100 and 200 are for.
				// theta = ((t / 50 - (nt / 100)) + cb) * PI / 180; // SOI direction/angle of arrival -> Moving across array over time i.e. angle changes each time sample
				// theta = (4 * ((w + filenum * n_win) - ((n_win * num_files) / 2)) + cb) * PI / 180; // SOI direction/angle of arrival -> Moving across array over time i.e. angle changes each time sample

				// If set to 1, then there are RAW files being generated so theta might need to spread over RAW files. If set to 0, there are no RAW files being generated
				if (raw_files == 1)
				{
					if (num_files == 1)
					{
						deg_per_tsamp = 1.0 / 12000;
					}
					else
					{
						deg_per_tsamp = 1.0 / 35000;
					}
					// Degrees per time sample
					theta = (deg_per_tsamp * ((t + (w * nt) + (filenum * n_win * nt)) - ((nt * n_win * num_files) / 2)) + cb) * PI / 180; // SOI direction/angle of arrival -> Moving across array over time i.e. angle changes each time sample
				}
				else
				{
					deg_per_tsamp = 1.0 / 12000;													 // Degrees per time sample
					theta = (deg_per_tsamp * ((t + (w * nt)) - ((nt * n_win) / 2)) + cb) * PI / 180; // SOI direction/angle of arrival -> Moving across array over time i.e. angle changes each time sample
				}
				// theta = cb * PI / 180; // SOI direction/angle of arrival
				// deg_per_tsamp = 1/50000; // Degrees per time sample
				// theta = (deg_per_tsamp*((t + w*nt) - ((nt*n_win) / 2)) + cb) * PI / 180; // SOI direction/angle of arrival -> Moving across array over time i.e. angle changes each time sample

				if (nt > 1)
				{
					// This is a rect window of noise, knowing that there
					// is an FFT performed. If constant noise is used, a
					// delta function is the result which does not show up if a log is used
					if ((t > zero_samps) && (t < (nt - zero_samps)))
					{
						noise_amp2 = 1;
					}
					else
					{
						noise_amp2 = 0;
					}
				}

				for (int f = 0; f < n_chan; f++)
				{
					// Place signal at one coarse channel
					if (n_chan > 1)
					{
						if (f == chan_idx)
						{
							if (just_noise == 0)
							{
								pulse = 1;
								if (nt == 1)
								{
									noise_amp2 = 0.01;
								}
								// noise_amp2 = 0; // 1;
							}
							else if (just_noise == 1)
							{
								pulse = 0;
								if (nt == 1)
								{
									noise_amp2 = 1;
								}
							}
						}
						else
						{
							pulse = 0;
							// noise_amp2 = 0.1; // 100000;
							if (nt == 1)
							{
								noise_amp2 = 1;
							}
						}
					}
					else
					{
						if (just_noise == 0)
						{
							pulse = 1;
							if (nt == 1)
							{
								noise_amp2 = 0.01;
							}
						}
						else if (just_noise == 1)
						{
							pulse = 0;
							if (nt == 1)
							{
								noise_amp2 = 1;
							}
						}
					}
					// printf("SIM_DATA_COEFF: freq = %d, noise_amp2 = %e\n", f, noise_amp2);
					//  rf_freqs = chan_band * f + c_freq;
					rf_freqs = ((chan_band * (f - (n_chan / 2))) + (chan_band / 2) + c_freq); //
					// rf_freqs = ((fine_chan_band * (t + (nt*f) - (nt*n_chan / 2))) + (fine_chan_band / 2) + c_freq)*1e-9;
					for (int a = 0; a < nants; a++)
					{
						tau = (a * d * cos(theta) / c); // Delay in seconds
						if (n_pol == 1)
						{
							if (a < n_sim_ant)
							{
								noise_realx = (noise_amp2 + noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)]);
								noise_imagx = (noise_amp2 + noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1]);
								// Requantize from doubles/floats to signed chars with a range from -128 to 127
								if (nt > 1)
								{
									// X polarization
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
								}
								else if (nt == 1)
								{
									// X polarization
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
								}
							}
							else
							{
								// X polarization
								data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0;
							}
						}
						else if (n_pol == 2)
						{
							if (a < n_sim_ant)
							{
								noise_realx = (noise_amp2 + noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)]);
								noise_imagx = (noise_amp2 + noise_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1]);

								noise_realy = (noise_amp2 + noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)]);
								noise_imagy = (noise_amp2 + noise_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1]);

								// Requantize from doubles/floats to signed chars with a range from -128 to 127
								if (nt > 1)
								{
									// X polarization
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									// Y polarization
									data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realy) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagy) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256); // Make this negative if a different polarization is tested
								}
								else if (nt == 1)
								{
									// X polarization
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagx) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									// Y polarization
									data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = (signed char)(((((pulse * cos(2 * PI * rf_freqs * tau) + noise_realy) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256);
									data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = (signed char)(((((pulse * sin(2 * PI * rf_freqs * tau) + noise_imagy) - tmp_min) / (tmp_max - tmp_min)) - 0.5) * 256); // Make this negative if a different polarization is tested
								}
								/*
								if ((f == (n_chan - 1)) && (w == 0 || w == (n_win / 2) || w == (n_win - 1)))
								{
									if (t == 0 || t == (nt - 1))
									{
										if (a == 0 || a == 1 || a == 2)
										{
											printf("\n");
											printf("SIM_DATA: file idx = %d \n", filenum);
											printf("SIM_DATA: a = %d, t = %d, w = %d and f = %d\n", a, t, w, f);
											printf("SIM_DATA: rf_freqs[%d] = %lf GHz \n", f, rf_freqs * 1e-9);
											printf("SIM_DATA: theta = %lf \n", (theta * 180 / PI));
											printf("SIM_DATA: tau = %e ns \n", tau * 1e9);
											printf("\n");
											printf("SIM_DATA: data_sim_r[x] = %d \n", (int)data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)]);
											printf("SIM_DATA: data_sim_r[x] = %d \n", (int)data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1]);
											printf("SIM_DATA: data_sim_r[y] = %d \n", (int)data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)]);
											printf("SIM_DATA: data_sim_r[y] = %d \n", (int)data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1]);
											printf("\n");
										}
									}
								}
								*/
							}
							else
							{
								// X polarization
								data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								data_sim[2 * data_in_idx(0, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0;
								// Y polarization
								data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan)] = 0;
								data_sim[2 * data_in_idx(1, t, w, f, a, n_pol, nt, n_win, n_chan) + 1] = 0; // Make this negative if a different polarization is tested
							}
						}
					}
				}
			}
		}
	}
	return data_sim;
}

// Generate simulated weights or coefficients
float *simulate_coefficients_ubf(int n_sim_ant, int nants, int n_pol, int max_n_beams, int actual_n_beams, int n_chan, int sim_flag, int telescope_flag)
{
	int n_coeff = 0;
	int n_ant_config = 0;
	if (telescope_flag == 0)
	{
		n_coeff = N_COEFF;
		n_ant_config = N_ANT;
	}
	else if (telescope_flag == 1)
	{
		n_coeff = VLASS_N_COEFF;
		n_ant_config = N_ANT / 2;
	}

	float *coeff_sim;
	coeff_sim = (float *)calloc(n_coeff, sizeof(float));
	/*
	'sim_flag' is a flag that indicates the kind of data that is simulated.
	sim_flag = 0 -> Ones
	sim_flag = 1 -> Scale each beam by incrementing value i.e. beam 1 = 1, beam 2 = 2, ..., beam 64 = 64
	sim_flag = 2 -> Scale each beam by incrementing value in a particular bin (bin 3 and 6 for now). Match simulated data sim_flag = 2
	sim flag = 3 -> Simulated beams from 58 to 122 degrees. Assuming a ULA.
	sim_flag = 4 -> One value at one polarization, one element, one beam, and one frequency bin
	*/
	if (sim_flag == 0)
	{
		for (int i = 0; i < (n_pol * n_ant_config * max_n_beams * n_chan); i++)
		{
			coeff_sim[2 * i] = 1;
			// coeff_sim[2*i + 1] = 1;
		}
	}
	if (sim_flag == 1)
	{
		int tmp = 0;

		for (int p = 0; p < n_pol; p++)
		{
			for (int f = 0; f < n_chan; f++)
			{
				for (int a = 0; a < nants; a++)
				{
					for (int b = 0; b < actual_n_beams; b++)
					{
						if (tmp >= actual_n_beams)
						{
							tmp = 0;
						}
						coeff_sim[2 * coeff_idx(a, p, b, f, nants, n_pol, max_n_beams)] = tmp * 0.01;
						tmp = (tmp + 1) % (max_n_beams + 1);
					}
				}
			}
		}
	}
	if (sim_flag == 2)
	{
		int tmp = 0;
		for (int p = 0; p < n_pol; p++)
		{
			for (int f = 0; f < n_chan; f++)
			{
				for (int a = 0; a < nants; a++)
				{
					for (int b = 0; b < actual_n_beams; b++)
					{
						if (tmp >= actual_n_beams)
						{
							tmp = 0;
						}
						tmp = (tmp + 1) % (actual_n_beams + 1);
						coeff_sim[2 * coeff_idx(a, p, b, f, nants, n_pol, max_n_beams)] = tmp;
					}
				}
			}
		}
	}
	if (sim_flag == 3)
	{
		float c = 3e8;					  // Speed of light
		float c_freq = 1504.5830078125e6; // Center frequency
		float lambda = c / c_freq;		  // Wavelength
		float d = lambda / 2;			  // Distance between antennas

		float chan_band = 1e6; // Fine channel bandwidth in Hz
		double rf_freqs = 0;

		float theta = 0; // Beam angle from 58 to 122 degrees

		float tau_beam = 0; // Delay

		int b = 0;
		float degree_offset = 0;
		int cen_beam = 90; // Center beam

		for (int f = 0; f < n_chan; f++)
		{
			rf_freqs = (chan_band * f + c_freq) * 1e-9;
			for (int a = 0; a < nants; a++)
			{
				for (b = 0; b < actual_n_beams; b++)
				{
					if (telescope_flag == 0)
					{
						theta = ((b - ((float)actual_n_beams / 2)) + 90) * PI / 180; // Beam angle from 58 to 122 degrees - Given SOI at 90 deg or moving across array, the beam with the most power is beamm 33
					}
					else if (telescope_flag == 1)
					{
						degree_offset = 45;
						theta = ((((float)b + 0.5) - ((float)actual_n_beams / 2)) * degree_offset + cen_beam) * PI / 180; // Beam angle from 58 to 122 degrees - Given SOI at 90 deg or moving across array, the beam with the most power is beamm 33
					}

					tau_beam = (a * d * cos(theta) / c) * 1e9; // Delay
					if (n_pol == 1)
					{
						// X polarization
						coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams)] = cos(2 * PI * rf_freqs * tau_beam);
						coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams) + 1] = sin(2 * PI * rf_freqs * tau_beam);
					}
					else if (n_pol == 2)
					{
						// X polarization
						coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams)] = cos(2 * PI * rf_freqs * tau_beam);
						coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams) + 1] = sin(2 * PI * rf_freqs * tau_beam);
						// Y polarization
						coeff_sim[2 * coeff_idx(a, 1, b, f, nants, n_pol, max_n_beams)] = cos(2 * PI * rf_freqs * tau_beam);
						coeff_sim[2 * coeff_idx(a, 1, b, f, nants, n_pol, max_n_beams) + 1] = sin(2 * PI * rf_freqs * tau_beam);

						if (a == 0 || a == 1 || a == nants - 1)
						{
							if (f == 0 || f == 3)
							{
								printf("COEFF_SIM: theta[beam idx = %d] = %f\n", b, theta * 180 / PI);
								printf("COEFF_SIM: tau[beam idx = %d] = %f ns\n", b, tau_beam);
								printf("COEFF_SIM: rf_freqs[%d] = %lf GHz\n", f, rf_freqs);
								printf("\n");
								printf("COEFF_SIM: coeff_sim_r[x] = %f\n", coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams)]);
								printf("COEFF_SIM: coeff_sim_i[x] = %f\n", coeff_sim[2 * coeff_idx(a, 0, b, f, nants, n_pol, max_n_beams) + 1]);
								printf("COEFF_SIM: coeff_sim_r[y] = %f\n", coeff_sim[2 * coeff_idx(a, 1, b, f, nants, n_pol, max_n_beams)]);
								printf("COEFF_SIM: coeff_sim_i[y] = %f\n", coeff_sim[2 * coeff_idx(a, 1, b, f, nants, n_pol, max_n_beams) + 1]);
								printf("\n");
							}
						}
					}
				}
			}
		}
	}
	if (sim_flag == 4)
	{
		for (int a = 0; a < nants; a++)
		{
			for (int f = 0; f < n_chan; f++)
			{
				coeff_sim[2 * coeff_idx(a, 0, 2, f, nants, n_pol, max_n_beams)] = 1;
			}
		}
	}

	return coeff_sim;
}

// Generate delays for bfr5 file
double *delay_bfr5(int nants, int n_beam, int sim_flag, int telescope_flag)
{
	int n_time_bf5 = 0;
	if (telescope_flag == 0)
	{
		n_time_bf5 = NTIMES_BFR5;
	}
	else if (telescope_flag == 1)
	{
		n_time_bf5 = NTIMES_VLA_BFR5;
	}

	//printf("delay_bfr5: n_ant = %d, n_beam = %d\n", nants, n_beam);
	double *tau; // Delay
	int tau_size = nants * n_beam * n_time_bf5;
	tau = (double *)calloc(tau_size, sizeof(double));

	if (sim_flag == 0)
	{
		float c = 3e8;					  // Speed of light
		float c_freq = 1504.5830078125e6; // Center frequency
		float lambda = c / c_freq;		  // Wavelength
		float d = lambda / 2;			  // Distance between antennas

		float theta = 0; // Beam angle from 58 to 122 degrees

		int b = 0;
		float degree_offset = 0;
		degree_offset = 45;
		// int tsamp_chunk = 0;
		// float tsamp_offset = 0;
		int cen_beam = 0;
		cen_beam = 90; // Angle of boresight beam relative to array

		for (int t = 0; t < n_time_bf5; t++)
		{
			// tsamp_chunk = (int)ceil((float)n_time_bf5 / n_beam);					 // Chunk of time samples between beams
			// tsamp_offset = ((t % tsamp_chunk) / (float)tsamp_chunk) * degree_offset; // Degree offset between time samples of a chunk of times samples between beams

			for (int a = 0; a < nants; a++)
			{
				for (b = 0; b < n_beam; b++)
				{
					if (telescope_flag == 0)
					{
						theta = ((b - ((float)n_beam / 2)) * degree_offset + cen_beam) * PI / 180; // Beam angle from 58 to 122 degrees - Given SOI at 90 deg or moving across array, the beam with the most power is beamm 33
					}
					else if (telescope_flag == 1)
					{
						theta = ((((float)b + 0.5) - ((float)n_beam / 2)) * degree_offset + cen_beam) * PI / 180; // Beam angle from 70 to 110 degrees - Given SOI at 90 deg or moving across array, the beam with the most power is beamm 33
																												  // if (t == 0 || t == 50 || t == 150)
																												  //{
																												  //	printf("DELAY_FUNC: tsamp_chunk = %d\n", tsamp_chunk);
																												  //	printf("DELAY_FUNC: tsamp_offset = %f\n", tsamp_offset);
																												  // }
					}

					tau[delay_idx(a, b, t, nants, n_beam)] = (a * d * cos(theta) / c) * 1e9; // Delay in nanoseconds (the beamformer recipe file expects nanoseconds)
					/*
					if ((t == 0 || t == 50 || t == 150) && (a == 0 || a == 10 || a == 21))
					{
						printf("DELAY_FUNC: theta = %f\n", (theta * 180 / PI));
						printf("DELAY_FUNC: tau(a=%d, b=%d, t=%d, nants=%d, n_beam=%d) = %e ns\n", a, b, t, nants, n_beam, tau[delay_idx(a, b, t, nants, n_beam)]);
					}
					*/
				}
				if ((t == 0 || t == 50 || t == 150) && (a == 0 || a == 10 || a == 21))
				{
					printf(" \n");
				}
			}
		}
	}

	return tau;
}