#  Copyright (c) 2017 Kurt Jacobson

#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:

#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.

#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

# note: original code taken from https://gist.github.com/KurtJacobson/c87425ad8db411c73c6359933e5db9f9
# modified by Salvatore Carotenuto

from copy import copy
from logging import Formatter


class ColoredFormatter(Formatter):

	COLORS = {
		# Reset
		#Color_Off: '0m',       # Text Reset

		# Regular Colors
		'Black': '0;30m',     # Black
		'Red'  : '0;31m',          # Red
		'Green': '0;32m',        # Green
		'Yellow':'0;33m',       # Yellow
		'Blue'  :'0;34m',         # Blue
		'Purple':'0;35m',       # Purple
		'Cyan'  :'0;36m',         # Cyan
		'White' :'0;37m',        # White
		# Bold
		'BBlack' : '1;30m',  # Black
		'BRed'   : '1;31m',  # Red
		'BGreen' : '1;32m',  # Green
		'BYellow': '1;33m',  # Yellow
		'BBlue'  : '1;34m',  # Blue
		'BPurple': '1;35m',  # Purple
		'BCyan'  : '1;36m',  # Cyan
		'BWhite' : '1;37m',  # White

		# Underline
		'UBlack' : '4;30m',  # Black
		'URed'   : '4;31m',  # Red
		'UGreen' : '4;32m',  # Green
		'UYellow': '4;33m',  # Yellow
		'UBlue'  : '4;34m',  # Blue
		'UPurple': '4;35m',  # Purple
		'UCyan'  : '4;36m',  # Cyan
		'UWhite' : '4;37m',  # White

		# Background
		'On_Black' : '40m',  # Black
		'On_Red'   : '41m',  # Red
		'On_Green' : '42m',  # Green
		'On_Yellow': '43m',  # Yellow
		'On_Blue'  : '44m',  # Blue
		'On_Purple': '45m',  # Purple
		'On_Cyan'  : '46m',  # Cyan
		'On_White' : '47m',  # White

		# High Intensity
		'IBlack' : '0;90m',  # Black
		'IRed'   : '0;91m',  # Red
		'IGreen' : '0;92m',  # Green
		'IYellow': '0;93m',  # Yellow
		'IBlue'  : '0;94m',  # Blue
		'IPurple': '0;95m',  # Purple
		'ICyan'  : '0;96m',  # Cyan
		'IWhite' : '0;97m',  # White

		# Bold High Intensity
		'BIBlack' : '1;90m',  # Black
		'BIRed'   : '1;91m',  # Red
		'BIGreen' : '1;92m',  # Green
		'BIYellow': '1;93m',  # Yellow
		'BIBlue'  : '1;94m',  # Blue
		'BIPurple': '1;95m',  # Purple
		'BICyan'  : '1;96m',  # Cyan
		'BIWhite' : '1;97m',  # White

		# High Intensity backgrounds
		'On_IBlack' : '0;100m',  # Black
		'On_IRed'   : '0;101m',  # Red
		'On_IGreen' : '0;102m',  # Green
		'On_IYellow': '0;103m',  # Yellow
		'On_IBlue'  : '0;104m',  # Blue
		'On_IPurple': '0;105m',  # Purple
		'On_ICyan'  : '0;106m',  # Cyan
		'On_IWhite' : '0;107m'   # White
		}

	COLOR_MAPPING = {
		#'DEBUG'   : 37,  # white
		'DEBUG'   : COLORS['Purple'],
		'INFO'    : COLORS['Cyan'],
		'WARNING' : COLORS['Yellow'],
		'ERROR'   : COLORS['Red'],
		'CRITICAL': COLORS['On_IRed']
	}
	COLOR_PREFIX = '\033['
	COLOR_RESET  = '\033[0m'


	def __init__(self, pattern):
		Formatter.__init__(self, pattern)

	def format(self, record):
		colored_record = copy(record)
		levelname = colored_record.levelname
		seq = ColoredFormatter.COLOR_MAPPING.get(levelname, 37) # default white
		colored_levelname = ('{0}{1}{2}{3}').format(ColoredFormatter.COLOR_PREFIX, seq, levelname, ColoredFormatter.COLOR_RESET)
		colored_record.levelname = colored_levelname
		colored_message = ('{0}{1}{2}{3}').format(ColoredFormatter.COLOR_PREFIX, seq, record.getMessage(), ColoredFormatter.COLOR_RESET)
		colored_record.msg = colored_message
		return Formatter.format(self, colored_record)
