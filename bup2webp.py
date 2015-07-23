#!/usr/bin/env python
import sys
import os

def bup2webp(file_in, file_out):
        outfile=open(file_out, 'wb')
        infile =open(file_in, 'rb')
        if infile.read(3) != 'bup':
            raise ValueError('%s is NOT bup format' % file_in)
        infile.seek(0, 2)
        size = infile.tell() - 64
# bup file is webp format with 64 bits prepend header
#0123456789ABCDEF|
#bup-????--------|
#----------------|
#----------------|
#----------------|
#RIFF************|
#***webp pic data|
#EOF
        infile.seek(64, 0)
        outfile.write(infile.read(size))
        outfile.flush()
        outfile.close()
        infile.close()

if __name__ == "__main__":
    for fname in sys.argv[1:]:
        print fname
        fstem, ext = os.path.splitext(fname)
        print fstem, ext
        bup2webp(fname, "%s.webp" % fstem)
