__author__ = 'Salvatore Carotenuto'

import serial
import time
import string


class SerialHandler():

    def __init__(self, params, logger):
        self.params = self.parse_params(params)
        self.logger = logger
        self.active = True
        self.message_delimiters = {'start': None, 'end': None}
        self.message_token_separator = None
        self.response_token_separator = None
        self.reading = False
        self.rx_buffer = ""
        self.message_callback_function = None

    def parse_params(self, params):
        parsed_params = {'port': params['port'],
                         'baudrate':  params['baudrate'],
                         'bytesize':  serial.EIGHTBITS,
                         'parity':  serial.PARITY_NONE,
                         'stopbits': serial.STOPBITS_ONE
                         }
        # bytesize
        if params['bytesize'] == 'FIVEBITS':
            parsed_params['bytesize'] = serial.FIVEBITS
        elif params['bytesize'] == 'SIXBITS':
            parsed_params['bytesize'] = serial.SIXBITS
        elif params['bytesize'] == 'SEVENBITS':
            parsed_params['bytesize'] = serial.SEVENBITS
        elif params['bytesize'] == 'EIGHTBITS':
            parsed_params['bytesize'] = serial.EIGHTBITS
        # parity
        if params['parity'] == 'PARITY_NONE':
            parsed_params['parity'] = serial.PARITY_NONE
        elif params['parity'] == 'PARITY_EVEN':
            parsed_params['parity'] = serial.PARITY_EVEN
        elif params['parity'] == 'PARITY_ODD':
            parsed_params['parity'] = serial.PARITY_ODD
        elif params['parity'] == 'PARITY_MARK':
            parsed_params['parity'] = serial.PARITY_MARK
        elif params['parity'] == 'PARITY_SPACE':
            parsed_params['parity'] = serial.PARITY_SPACE
        # stopbits
        if params['stopbits'] == 'STOPBITS_ONE':
            parsed_params['stopbits'] = serial.STOPBITS_ONE
        elif params['stopbits'] == 'STOPBITS_ONE_POINT_FIVE':
            parsed_params['stopbits'] = serial.STOPBITS_ONE_POINT_FIVE
        elif params['stopbits'] == 'STOPBITS_TWO':
            parsed_params['stopbits'] = serial.STOPBITS_TWO
        #
        return parsed_params

    def set_message_delimiters(self, start, end):
        self.message_delimiters['start'] = start
        self.message_delimiters['end'] = end

    def set_message_token_separator(self, separator):
        self.message_token_separator = separator

    def set_response_token_separator(self, separator):
        self.response_token_separator = separator

    def set_message_callback(self, callback):
        self.message_callback_function = callback

    def begin(self):
        self.ser = serial.Serial(port=self.params['port'], baudrate=self.params['baudrate'], parity=self.params['parity'],
                                 stopbits=self.params['stopbits'], bytesize=self.params['bytesize'], timeout=0)
        #
        if self.ser.isOpen():
            self.logger.info("serial connected to: %s (%d,%d,%s,%d)" % (
                self.ser.portstr, self.ser.baudrate, self.ser.bytesize, self.ser.parity, self.ser.stopbits))

    def stop(self):
        self.active = False
        self.ser.close()

    def write_message(self, topic, payload):
        if self.ser.isOpen():
            message = self.message_delimiters['start'] + topic
            if(payload):
                message += (self.message_token_separator + payload)
            message += self.message_delimiters['end']
            message += '\n'
            #
            self.ser.write(message.encode('utf-8'))
            #
            time.sleep(0.1)

    def read_response(self, wait):
        if wait:
            time.sleep(wait)
        #
        self.rx_buffer = ""
        response = {
            'all': None,
            'tokens': None
        }
        try:
            data = self.ser.read(self.ser.in_waiting).decode()
            for c in data:
                # DEBUG
                # print(c, ord(c))
                #
                if not self.reading:
                    if c == self.message_delimiters['start']:
                        self.reading = True
                        self.rx_buffer = ""
                else:
                    if c == self.message_delimiters['end']:
                        self.reading = False
                        # print("LINE:", self.rx_buffer)
                        response['all'] = self.rx_buffer
                        if self.response_token_separator:
                            response['tokens'] = self.rx_buffer.split(
                                self.response_token_separator)
                    else:
                        if c in string.printable:
                            self.rx_buffer += c
            #
            time.sleep(0.001)
            #
        except:
            self.ser.close()
            self.active = False
        #
        return response

    def clear_rx_buffer(self):
        self.rx_buffer = ""
