##### Library ######
add_library(
  dataSubselector STATIC
  src/BitMaskCut.cxx
  src/CutBase.cxx
  src/Cuts.cxx
  src/Gti.cxx
  src/GtiCut.cxx
  src/RangeCut.cxx
  src/SkyConeCut.cxx
  src/VersionCut.cxx
)
target_link_libraries(
  dataSubselector
  PUBLIC astro evtbin tip
  PRIVATE st_facilities facilities irfLoader irfInterface irfUtil
)
target_include_directories(
  dataSubselector PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)

###### Executables ######
add_executable(
  gtselect
  src/dataSubselector/CutController.cxx
  src/dataSubselector/dataSubselector.cxx
)
target_include_directories(
  gtselect PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src
  $<INSTALL_INTERFACE:>
)
add_executable(gtmktime src/gtmaketime/gtmaketime.cxx)
add_executable(gtvcut src/viewCuts/viewCuts.cxx)

target_link_libraries(gtselect PRIVATE dataSubselector st_facilities)
target_link_libraries(gtmktime PRIVATE dataSubselector st_facilities)
target_link_libraries(gtvcut PRIVATE dataSubselector st_facilities)

###### Tests ######
add_executable(test_dataSubselector src/test/test.cxx)
target_link_libraries(
  test_dataSubselector
  PRIVATE dataSubselector st_facilities CppUnit::CppUnit
)


###############################################################
# Installation
###############################################################
install(DIRECTORY dataSubselector DESTINATION ${FERMI_INSTALL_INCLUDEDIR})
install(DIRECTORY data/ DESTINATION ${FERMI_INSTALL_REFDATADIR}/dataSubselector)
install(DIRECTORY pfiles/ DESTINATION ${FERMI_INSTALL_PFILESDIR})

install(
  TARGETS dataSubselector gtmktime gtselect gtvcut test_dataSubselector
  EXPORT fermiTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
