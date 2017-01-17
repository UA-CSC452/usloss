#!/usr/bin/env python2.7

import unittest
import sys
import optparse
import subprocess

class Base(unittest.TestCase):

    def CheckStatus(self, cmd, status=0):
        global options
        cmd = [str(x) for x in cmd]
        with open("/dev/null", "w") as fd:
            kwargs = {}
            if options.verbosity < 3:
                kwargs['stdout'] = fd
                kwargs['stderr'] = fd
            result = subprocess.call(cmd, **kwargs)
            self.assertEqual(status, result)

    def CheckOutput(self, cmd, output):
        global options
        cmd = [str(x) for x in cmd]
        with open("/dev/null", "w") as fd:
            kwargs = {}
            if options.verbosity < 3:
                kwargs['stderr'] = fd
            result = subprocess.check_output(cmd, **kwargs)
            self.assertEqual(output, result)

class Clock(Base):

    def test_clock(self):
        self.CheckStatus(['tests/clock'])

class Context(Base):

    def test_context0(self):
        output = "Startup\nTest0\nFinish\n"
        self.CheckOutput(['tests/context0'], output)

    def test_context1(self):
        output = "Startup\n"
        for i in range(0,4):
            for j in range(0,2):
                output += "Test%d\n" % j
        output += "Finish\n"
        self.CheckOutput(['tests/context1'], output)

class Interrupt(Base):

    def setUp(self):
        with open("term0.in", "w") as fd:
            print >> fd, "Hello World!"

    def test_interrupt(self):
        self.CheckOutput(['tests/interrupt'], "Hello World!\n")

class PSR(Base):

    def test_psr(self):
        self.CheckOutput(['tests/psr'], "0x1 0x3 0x1 0x2\n")

    def test_psr1(self):
        self.CheckStatus(['tests/psr1'])

    def test_invalid_psr_0(self):
        self.CheckStatus(['tests/psr2', 0], 1)

    def test_invalid_psr_0xffffffff(self):
        self.CheckStatus(['tests/psr2', 0xffffffff], 1)

class Syscall(Base):

    def test_syscall(self):
        self.CheckStatus(['tests/syscall'])

    def test_syscall2(self):
        self.CheckStatus(['tests/syscall2'])

class Term(Base):

    def test_termread(self):
        self.CheckStatus(['tests/termread'])

    def test_termwrite(self):
        self.CheckStatus(['tests/termwrite'])

    def test_termwritebig(self):
        self.CheckStatus(['tests/termwritebig'])


class MMU(Base):

    def test_mmu(self):
        self.CheckStatus(['tests/mmu'], -11) # ends with seg fault

    def test_mmu2(self):
        self.CheckStatus(['tests/mmu2'], 0)

    def test_mmu3(self):
        self.CheckStatus(['tests/mmu3'], 0)

    def test_no_access(self):
        self.CheckStatus(['tests/mmu4', 0], 0)

    def test_readonly(self):
        self.CheckStatus(['tests/mmu4', 1], 0)

    def test_readwrite(self):
        self.CheckStatus(['tests/mmu4', 2], 0)

    def test_make_no_access(self):
        self.CheckStatus(['tests/mmu4', 3], 0)

    def test_make_readonly(self):
        self.CheckStatus(['tests/mmu4', 4], 0)

    def test_pagefault(self):
        self.CheckStatus(['tests/mmu4', 5], 0)

def main(args):
    global options
    usage = "Usage %prog [options] [test0 test1 ...]"

    parser = optparse.OptionParser(version="1.0", usage=usage)

    parser.add_option("-q", "--quiet",
                      action="store_const", const=0, dest="verbosity",
                      help="run quietly")

    parser.add_option("-v", "--verbose",
                      action="count", dest="verbosity",
                      help="increase verbosity")

    parser.add_option("-F", "--failfast",
                      action="store_true", dest="failfast", default=False,
                      help="stop on first failed test [default: %default")

    cmd = [args[0]]
    (options, args) = parser.parse_args(args[1:])

    unittest.main(argv=cmd + args, verbosity=options.verbosity, failfast=options.failfast)


if __name__ == "__main__":
    main(sys.argv)



