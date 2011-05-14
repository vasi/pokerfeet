import os

# Parallel build
SetOption('num_jobs', os.sysconf('SC_NPROCESSORS_ONLN'))

env = Environment(
    LINKFLAGS = '-g',
    CPPFLAGS = '-Wall -g -O2')
env.Program('cards', Glob('*.cc'))
