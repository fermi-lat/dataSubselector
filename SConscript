# -*- python -*-
#
# $Id: SConscript,v 1.38 2012/03/18 00:51:45 jchiang Exp $
# Authors: James Chiang <jchiang@slac.stanford.edu>
# Version: dataSubselector-08-03-03
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

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

progEnv.Tool('registerTargets', package = 'dataSubselector', 
             staticLibraryCxts = [[dataSubselectorLib, libEnv]],
             binaryCxts = [[gtselectBin, progEnv], [gtmktimeBin, progEnv],
                           [gtvcutBin, progEnv]], 
             testAppCxts = [[test_dataSubselectorBin, testEnv]],
             includes = listFiles(['dataSubselector/*.h']),
             pfiles = listFiles(['pfiles/*.par']),
             data = listFiles(['data/*'], recursive = True))
