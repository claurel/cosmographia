Minor Planet Tools for Cosmographia

There are three python scripts, all of which operate on ASCII asteroid orbit
files in the format described here:

   http://www.naic.edu/~nolan/astorb.html


=== astorb2bin ===

Converts an ASCII astorb file to the binary format used by Cosmographia.

Usage: astorb2bin.py [options]

Options:
  -h, --help                  show this help message and exit
  -o FILE, --out=FILE         Write binary catalog to FILE
  -i FILE, --in=FILE          Read asteroid orbital data from FILE
  -d FILE, --discovery=FILE   Read discovery dates from FILE

Example:
   astorb2bin.py -i astorb.dat -o allasteroids.bin -d NumberedMPs.txt

One of the fields in Cosmographia's binary format for asteroid orbits is
the discovery date. Objects will be plotted only when the simulating time
is after the discovery date. The iastorb inputcatalog doesn't contain
discovery dates so they are inferred from the provisional designation or
looked up in an auxilliary catalog of discovery circumstances. The file
NumberedMPs.txt in this directory may be used, but it is better to grab
the latest catalog from the Minor Planet Center:

   http://www.minorplanetcenter.net/iau/lists/NumberedMPs000001.html


=== astorb2json ===

Command line: astorb2json <input file>

Converts an ASCII astorb file to a JSON catalog file. This will create
individual objects for each body in the catalog file. For large numbers
(> 1000) of objects, it is better to use binary catalog files.

Note: astorb2json writes to standard output.


=== astorbfilter ===

Command line: astorbfilter <group index> <input file> <output file>

Filters minor planets based on orbital elements for one of several families.
The output file is in the same ASCII format as the input. Group index may be
one of:

  2 - Hildas
  7 - Plutino
  8 - Classical KBOs





   

