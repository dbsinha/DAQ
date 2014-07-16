'''
	Drew Sinha
	Pincus Lab
	testArduino.py
	
	Prototyping script for interacting with an Arduino board through an ArduinoInterface
'''

import arduinoInterface

myAI = arduinoInterface.ArduinoInterface('/dev/ttyACM1',None,1,False,'micro')	# Instantiate the interface; parameters read as: "path_to_connection","timeout for reading","timeout/delay for writing", "Be verbose - currently unimplemented","type of board"
myAI.startAcquisition()	# Call acquisition routine
