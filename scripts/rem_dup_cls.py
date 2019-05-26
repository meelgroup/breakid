#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2014  Mate Soos
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

from __future__ import with_statement  # Required in 2.5
from __future__ import print_function
import subprocess
import os
import sys
import time

fname = sys.argv[1]

cls = []
maxvar = 0
with open(fname, "r") as f:
    for line in f:
        cl = []
        line = line.strip()

        # skip empty line
        if len(line) == 0:
            continue

        # skip header and comments
        if line[0] == 'c' or line[0] == 'p':
            continue

        line = line.split()
        for x in line:
            try:
                lit = int(x)
            except ValueError:
                print("ERROR: CNF incorrect: ", line)
                print("Tried parsing literal:", x)
                exit(-1)

            if lit == 0:
                cl = sorted(cl)
                cls.append(cl)
                break
            else:
                maxvar = max(abs(lit), maxvar)
                cl.append(lit)


unique_data = [list(x) for x in set(tuple(x) for x in cls)]

print("p cnf %d %d" % (maxvar, len(unique_data)))
for cl in unique_data:
    txt = ""
    for lit in cl:
        txt += "%d " % lit
    txt+="0"
    print(txt)
