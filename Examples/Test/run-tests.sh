#!/bin/sh
#
# Runs the tests defined for the examples
# (Yup, this is nearly identical to the run-tests in the parent directory, automake doesn't seem to support an easy way to supply parameters to tests)

# The directory containing the definition whose tests need to be run
definition_dir=${srcdir}

# Name of the tameparse runner
tameparse=../../parsetool/tameparse

# Iterate through the items in the  directory
for definition_file in ${definition_dir}/*.tp
do
	${tameparse} --run-tests ${definition_file}
done
