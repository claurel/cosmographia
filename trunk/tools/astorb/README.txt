Minor Planet Tools for Cosmographia

There are three python scripts, all of which operate on ASCII asteroid orbit
files in the format described here:

   http://www.naic.edu/~nolan/astorb.html


=== astorb2bin ===

Command line: astorb2bin <input file> <output file>

Converts an ASCII astorb file to the binary format used by Cosmographia.


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





   

