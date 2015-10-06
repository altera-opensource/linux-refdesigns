#!/bin/sh

# Copyright (C) 2015 Altera Corporation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

# Internals
SELF="$(basename $0)"

# Constants
readonly BAR_COUNT=24			# Total count of display bars
readonly QUANTUM_LEVEL=4096		# Quantum levels for 12bits ADC
readonly ADC_DEFAULT_SYSFS="/sys/bus/iio/devices/iio:device0/in_voltage0_adc1-ch7_raw"

# Variables
RAW=			# Raw Value read from ADC
VAR=			# Nominalize ADC value to Bar Count
BAR=			# Number of bars to be displayed
ADC_SYSFS=		# ADC sysfs to read

# Display help
help() {
	echo "ADC Demonstration to display ADC reading"
	echo "usage: ${SELF} -t <ADC sysfs>"
	echo "-t file: specifies ADC sysfs. Default points to,"
	echo "         ${ADC_DEFAULT_SYSFS}"
	echo "-h     : this message."
	return 0
}

# Display function
display() {
	RAW="$(cat ${ADC_SYSFS})"
	VAR="$(expr ${RAW} \* ${BAR_COUNT} / ${QUANTUM_LEVEL})"
	BAR="$(printf '%0.s#' $(seq 0 ${VAR}))"
	printf "\r\033[2JADC Demo: Press CTRL-C to quit.\n"
	printf "Raw Value: %-4s [%-24s] \033[1;44f" ${RAW} ${BAR}
	return 0
}

# Script starts here
# Option parsing
while [ $# -gt 0 ]; do
	case $1 in
		-h) help; exit 0 ;;
		-t) ADC_SYSFS=$2 ; shift ;;
		*)  echo "${SELF}: error: unknown option ${1}" ; help ; exit 1 ;;
	esac
	shift
done

# Error checks
if [ -z "${ADC_SYSFS}" ]; then
	ADC_SYSFS=${ADC_DEFAULT_SYSFS}
fi

if [ ! -f "${ADC_SYSFS}" ]; then
	echo "${SELF}: error: ${ADC_SYSFS}: no such file."
	exit 1
fi

# Start display
while [ true ]; do
	display
done
