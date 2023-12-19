# PLCRA

This is a series of C, python, and bash files used to collect executable virtual memory from processes running on a Revolution Pi Connect+.  This work was done as part of the research work "Profiling Embedded Systems Processes for Remote Attestation".  

## Installation

This package is available from source here on github.

## Usage

Many of the files in this repository are stand alone tools.  The main approaches can be found in the dump/ directory.
    dvm.c -- uses a C file to dump all virtual memory from a given process.  The first command line argument is the PID     of the target process.

    dump_virtual_memory.py -- same as above, but as a python file.

The detect directory holds a modified version of each of the above files.  They have been modified  to pull only executable virtual memory.  Seperate files were used to reduce computational overhead as much as possible.  Detect also contains references to the [mandibule project](https://github.com/ixty/mandibule).  

## Contributing

Please feel free to modify this code as needed.  Pull requests are welcome, but responses may not be timely.

## License

[MIT](https://choosealicense.com/licenses/mit/)
