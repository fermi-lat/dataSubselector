# -*- python -*-
#
# $Id: SConscript,v 1.47 2012/12/19 16:59:34 sfegan Exp $
# Authors: James Chiang <jchiang@slac.stanford.edu>
# Version: dataSubselector-08-05-00
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
