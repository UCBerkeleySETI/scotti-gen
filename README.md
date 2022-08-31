# scotti-gen
scotti-gen - Simulated Controlled Technosignature Testing Instrument generator. Used to generate GUPPI RAW and beamformer recipe files with simulated data and phase information respectively.

To compile the test code and generate simulated RAW and BFR5 files, use the following command:

`gcc scotti_gen.c sim_data_coeff.c -o scotti_gen -lm -lhdf5`

Dependencies are the hdf5 and math libraries as seen in the compile command above.

To learn the different arguments necessary to run the executable/program, use the `-h` or `--help` option when running the executable.

The output is written to filterbank files which for now can be analyzed with plot\_waterfall\_blimpy.py in the post\_processing sub-directory which uses blimpy to generate a waterfall plot. The script is run with the filterbank file as the first argument. Or to generate plots with more user control, one can use plot\_upchan\_bf\_filterbank\_file.py in the same sub-directory. This file requires a little more. The variables filename, telescope_flag, mode_flag, and num\_raw\_files, must be changed within the script. I would make these variables arguments, but this is made difficult with a remote connenction in VS code using Jupyter notebook to enable plotting. 
