'''
	Drew Sinha
	Pincus Lab
	serialInterface.py
	
	Defines a class that wraps the pySerial object in a more convenient form for reading and writing data through a serial connection
'''

import serial
import datetime
from warnings import warn
from time import sleep


class SerialInterface:
	_serialPort = None		# (serial.Serial obj) Holds assignment for the serial port connection
	readTimeout = None		# (int/NoneType) Timeout in between reads (setting to None results in blocking for input string and no timeout
	writeTimeout = None		# (int) Timeout for delaying in between writes
	DEBUG = None			# (boolean) Flag for turning on debugging
	
	# Constructor
	def __init__(self, inSerialPort=None, inBaudRate=9600, inReadTimeout=0, inWriteTimeout=0, inDEBUG = False):
		# Check for valid assignment for Incubator port
		if inSerialPort==None:	# If no port, information provided, raise error
			raise RuntimeError('(SerialInterface.__init__()) No valid port supplied for generating SerialInterface')
		elif isinstance(inSerialPort, serial.Serial):		# If a Serial object, directly copy it
			self._serialPort = inSerialPort
		else:	# Otherwise, assume some valid system reference for port is given and instantiate with default timeout values
			self._serialPort = serial.Serial(port=inSerialPort, baudrate=inBaudRate, timeout=0,writeTimeout=0)
		
		# Populate instance variables
		self.readTimeout = inReadTimeout
		self.writeTimeout = inWriteTimeout
		self.DEBUG = inDEBUG
		
	# Property for managing the port
	def getSerialPort(self):
		return self._serialPort	
	def setSerialPort(self, inPort):
		self._serialPort = inPort
		self._serialPort.flush()	# Flush the port
		self._serialPort.flushInput()
		self._serialPort.flushOutput()
	serialPort = property(getSerialPort,setSerialPort)
	
	
	# Methods for reading and writing data
	def read_data(self, EOLChar = '\n'):
		# Returns buffered data up to, but not including, the character EOLChar lying in buffer
		# Because of issues with some of Serial's read/write functionality, this handles all the necessary buffering and blocking
		enter_time = datetime.datetime.now()	# Get current time for working with timeout and pre-allocate return variable
		outBuffer=b''
		while 1:	
			tempBuffer = self._serialPort.read(1)	# Read next character in device buffer
			if tempBuffer:  # If the device buffer is not empty
				outBuffer += tempBuffer
				for dummyNum in range(0, len(tempBuffer)):	# Keep reading from the buffer to get remainder of the data
					outBuffer += self._serialPort.read(1)
				if outBuffer[len(outBuffer)-1]==ord(EOLChar):		# If we have reached the end of the message
					return outBuffer[:len(outBuffer)-1]	# Return buffer
			elif self.readTimeout is None:	# If timeout is None, just pass (we want to block)
				pass
			elif datetime.datetime.now() - enter_time > datetime.timedelta(seconds=self.readTimeout):	# If we timed out
				# Raise some warnings about timing out
				if len(outBuffer) > 0:
					warn('(SerialInterface.read_data()) Buffer read timed out with data still in buffer - %s' % outBuffer)
				else:
					warn('(SerialInterface.read_data()) Buffer read timed out with no data in buffer');
				return outBuffer	# Return the buffer
				
	def write_data(self,inMessage):
		# Writes data to device (incubator) then blocks for "timeout" seconds
		# Because of issues with some of pySerial's read/write functionality, this handles all the necessary buffering and blocking
		self._serialPort.write(inMessage)
		sleep(self.writeTimeout)
		if(self.DEBUG): 
			print(datetime.datetime.now()-starttime)
	
	# Helper function for getting state of the serial port connection	
	def isOpen(self):
		return _serialPort.isOpen()
