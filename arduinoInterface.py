'''
	Drew Sinha
	Pincus Lab
	arduinoInterface.py
	
	Provides a simple template for communicating with an Arduino board using an attached serial connection
'''

import serialInterface;

class ArduinoInterface:
	_port  = None		# (serialInterface.SerialInterface obj) Holds the serial connection wrapper for interacting with the board
	board = None		# (string) Arduino board type that we are using	
	verbose = None		# Flag for debugging purposes (NOTE: currently unimplemented!)

	# Constructor
	def __init__(self, inSerialPort=None, inReadTimeout=0, inWriteTimeout=0, inVerbose=False, inBoard = 'micro'):
		# Check for valid assignment for serial port
		if inSerialPort==None:	# If no port, information provided, raise error
			raise RuntimeError('(ArduinoInterface.__init__()) No valid port supplied for generating port')
		elif isinstance(inSerialPort, serialInterface.SerialInterface):		# If a Serial object, directly copy it
			self._port = inSerialPort
		else:	# Otherwise, assume some valid system reference for port is given and instantiate with default timeout values
			if (inBoard=='micro'):
				baudrateValue = 9600
			else:
				raise Exception("(ArduinoInterface.__init__()) board not supported - baud rate default value not given.");
				# TODO Implement more baudrates...
			
			#Assign interface object
			self._port = serialInterface.SerialInterface(inSerialPort, baudrateValue, inReadTimeout, inWriteTimeout, False)
			
		# Populate instance variables
		self.board = inBoard
		self.verbose = inVerbose
	
	# A simple method simulating an acquisition routine while interacting with the board 	
	def startAcquisition(self):	
		self._port.write_data('aq\n'.encode('ascii'))	# Write the command sequence to the board; TODO: Think about adding a "CMD-" in front of commands
		while True:		# Event loop for reading string data from the serial connection
			msg = self._port.read_data().decode('ascii')	#Read the data through the serial connection
			
			# Then, process according to what type of information is in the message
			if(msg[0:3]=='ACK'):	# Acknowledgement - sent upon successful initiation of a command (i.e. "aq\n" for acquisition)
				print(msg)
			elif(msg[0:3]=='DBG'):	# Debugging - Sent for obvious purposes
				print(msg)
			elif(msg[0:3]=='DAT'):	# Data - This is what we are interested in; here this will just be a message saying there was an event
				print(msg)
				print('all done - breaking now!')
				break	# Finish the event loop and exit the method
			elif(msg[0:3]=='ERR'):	# Error - Sent to notify of an error on the board
				raise Exception(msg)
			elif(msg[0:3]=='MSG'):	# Message - Generic information from the board
				print(msg)
			else:					# Unhandled types
				print('Start of message:')	# Dump the message
				print(msg)
				print('End of message')
				raise AttributeError('(ArduinoInterface.startAcquisition()) Message type ' + msg[0:3] + ' not supported')	# Raise an error
