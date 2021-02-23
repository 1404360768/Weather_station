#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Weather_station

EXTRA_COMPONENT_DIRS := components/lcd
EXTRA_COMPONENT_DIRS += components/getwifi
EXTRA_COMPONENT_DIRS += components/station
EXTRA_COMPONENT_DIRS += components/time_date


include $(IDF_PATH)/make/project.mk

