import os

# Parallel build
SetOption('num_jobs', os.sysconf('SC_NPROCESSORS_ONLN'))

env = Environment(
    LINKFLAGS = '-g',
    CPPFLAGS = '-Wall -g -Os')
env.Program('cards', Glob('*.cc'))
