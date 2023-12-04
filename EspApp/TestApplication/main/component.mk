#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

include ../../application.mk

COMPONENT_ADD_INCLUDEDIRS := . ../../Application/components/UxEspCppLibrary
