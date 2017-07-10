#! /bin/bash

# The script builds the c

#Do not change until you are making required changes in the firmware code.  
HEADER=P8_
PIN_NUMBER41=41
PIN_NUMBER42=42
PIN_NUMBER43=43
PIN_NUMBER44=44
PIN_NUMBER45=45
PIN_NUMBER46=46

echo "-Building project"
	make install

echo "-Configuring pinmux"
	config-pin -a $HEADER$PIN_NUMBER41 pruin
	config-pin -q $HEADER$PIN_NUMBER41
	config-pin -a $HEADER$PIN_NUMBER42 pruin
	config-pin -q $HEADER$PIN_NUMBER42
	config-pin -a $HEADER$PIN_NUMBER43 pruin
	config-pin -q $HEADER$PIN_NUMBER43
	config-pin -a $HEADER$PIN_NUMBER44 pruin
	config-pin -q $HEADER$PIN_NUMBER44
	config-pin -a $HEADER$PIN_NUMBER45 pruin
	config-pin -q $HEADER$PIN_NUMBER45
	config-pin -a $HEADER$PIN_NUMBER46 pruin
	config-pin -q $HEADER$PIN_NUMBER46

echo "********************************************************"
echo -e "Done. Now \"echo S > /dev/rpmsg_pru31 && cat /dev/rpmsg_pru31\" and change the logical state of $HEADER$PIN_NUMBER"
echo "********************************************************"
