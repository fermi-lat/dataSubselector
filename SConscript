# -*- python -*-
#
# $Id: SConscript,v 1.19 2009/07/14 23:24:09 jrb Exp $
# Authors: James Chiang <jchiang@slac.stanford.edu>
# Version: dataSubselector-06-04-02
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('dataSubselectorLib', depsOnly = 1)
dataSubselectorLib = libEnv.StaticLibrary('dataSubselector', 
                                          listFiles(['src/*.cxx']))

progEnv.Tool('dataSubselectorLib')

testEnv = progEnv.Clone()
testEnv.Tool('addLibrary', library = baseEnv['cppunitLibs'])
test_dataSubselectorBin = testEnv.Program('test_dataSubselector', 
                                          listFiles(['src/test/*.cxx']))

gtselectBin = progEnv.Program('gtselect', 
                              listFiles(['src/dataSubselector/*.cxx']))

gtmktimeBin = progEnv.Program('gtmktime', listFiles(['src/gtmaketime/*.cxx']))

gtvcutBin = progEnv.Program('gtvcut', listFiles(['src/viewCuts/*.cxx']))

#progEnv.Tool('registerObjects', package = 'dataSubselector', 
#             libraries = [dataSubselectorLib],
#             binaries = [gtselectBin, gtmktimeBin, gtvcutBin], 
#             testApps = [test_dataSubselectorBin],
#             includes = listFiles(['dataSubselector/*.h']),
#             pfiles = listFiles(['pfiles/*.par']),
#             data = listFiles(['data/*'], recursive = True))
progEnv.Tool('registerTargets', package = 'dataSubselector', 
             staticLibraryCxts = [dataSubselectorLib],
             binaryCxts = [gtselectBin, gtmktimeBin, gtvcutBin], 
             testAppCxts = [test_dataSubselectorBin],
             includes = listFiles(['dataSubselector/*.h']),
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True))
