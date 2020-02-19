/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "ultima/ultima8/misc/console.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/graphics/fonts/fixed_width_font.h"
#include "ultima/shared/std/misc.h"
#include "common/file.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima8 {

#define MAXPRINTMSG 4096

// The console
Console *con;

// Console out/err pointers
console_ostream<char> *ppout;
console_err_ostream<char> *pperr;

//
// Constructor
//
Console::Console() : _current(0), _xOff(0), _display(0), _lineWidth(-1),
		_totalLines(0), _visLines(0), _wordWrap(true), _cr(false),
		_putChar_count(0), _stdOutputEnabled(0xFFFFFFFF),
		_stdout_redir(0), _stderr_redir(0), _conFont(0),
		_autoPaint(0), _msgMask(MM_ALL), _frameNum(0),
		commandCursorPos(0), commandInsert(true), commandHistoryPos(0) {
	con = this;
	_lineWidth = -1;

	CheckResize(0);

	Std::memset(_times, 0, sizeof(_times));

	// Set output pointers
	ppout = &_strOut;
	pperr = &_errOut;

	// Lets try adding a Console command!
	AddConsoleCommand("Console::CmdList", ConCmd_CmdList);
	AddConsoleCommand("Console::CmdHistory", ConCmd_CmdHistory);

	PrintInternal("Console initialized.\n");
}

//
// Destructor
//
Console::~Console() {
	RemoveConsoleCommand(Console::ConCmd_CmdList);
	RemoveConsoleCommand(Console::ConCmd_CmdHistory);

	// Need to do this first
	PrintPutchar();

	ppout = nullptr;
	pperr = nullptr;
}

/*
================
Con_Clear_f
================
*/
void Console::Clear() {
	Std::memset(_text, ' ', CON_TEXTSIZE);
	_putChar_count = 0;
}


/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Console::Dump(const char *name) {
	int     l, x;
	char    *line;
	Common::DumpFile f;
	char    buffer[1024];

	// Need to do this first
	PrintPutchar();

	Printf("Dumped console _text to %s.\n", name);

	if (!f.open(name)) {
		Print("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = _current - _totalLines + 1 ; l <= _current ; l++) {
		line = _text + (l % _totalLines) * _lineWidth;
		for (x = 0 ; x < _lineWidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != _lineWidth)
			break;
	}

	// write the remaining lines
	buffer[_lineWidth] = 0;
	for (; l <= _current ; l++) {
		line = _text + (l % _totalLines) * _lineWidth;
		Std::strncpy(buffer, line, _lineWidth);
		for (x = _lineWidth - 1 ; x >= 0 ; x--) {
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x = 0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		f.writeString(buffer);
		f.writeByte('\n');
	}

	f.close();
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Console::CheckResize(int scrwidth) {
	int     i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char    tbuf[CON_TEXTSIZE];

	// Need to do this first
	PrintPutchar();

	if (!_conFont)
		width = (scrwidth >> 3) - 2;
	else
		width = (scrwidth / _conFont->_width) - 2;

	if (width == _lineWidth)
		return;

	if (width < 1) {        // video hasn't been initialized yet
		width = 78;
		_lineWidth = width;
		_totalLines = CON_TEXTSIZE / _lineWidth;
		Std::memset(_text, ' ', CON_TEXTSIZE);
	} else {
		oldwidth = _lineWidth;
		_lineWidth = width;
		oldtotallines = _totalLines;
		_totalLines = CON_TEXTSIZE / _lineWidth;
		numlines = oldtotallines;

		if (_totalLines < numlines)
			numlines = _totalLines;

		numchars = oldwidth;

		if (_lineWidth < numchars)
			numchars = _lineWidth;

		Std::memcpy(tbuf, _text, CON_TEXTSIZE);
		Std::memset(_text, ' ', CON_TEXTSIZE);

		for (i = 0 ; i < numlines ; i++) {
			for (j = 0 ; j < numchars ; j++) {
				_text[(_totalLines - 1 - i) * _lineWidth + j] =
				    tbuf[((_current - i + oldtotallines) %
				          oldtotallines) * oldwidth + j];
			}
		}
	}

	_current = _totalLines - 1;
	_display = _current;
}


//
// Internal Methods
//

// Print a _text string to the console
void Console::PrintInternal(const char *txt) {
	int     y;
	int     c, l;

	// Need to do this first
	PrintPutchar();

	while (0 != (c = *txt)) {
		if (_wordWrap) {
			// count word length
			for (l = 0 ; l < _lineWidth ; l++)
				if (txt[l] <= ' ')
					break;

			// word wrap
			if (l != _lineWidth && (_xOff + l > _lineWidth))
				_xOff = 0;
		}

		txt++;

		if (_cr) {
			_current--;
			_cr = false;
		}

		if (!_xOff) {
			Linefeed();
			// mark time for transparent overlay
			if (_current >= 0) _times[_current % CON_NUM_TIMES] = _frameNum;
		}

		switch (c) {
		case '\n':
			_xOff = 0;
			break;

		case '\r':
			_xOff = 0;
			_cr = true;
			break;

		default:    // _display character and advance
			y = _current % _totalLines;
			_text[y * _lineWidth + _xOff] = static_cast<char>(c);
			_xOff++;
			if (_xOff >= _lineWidth) _xOff = 0;
			break;
		}

	}
}

// Print a _text string to the console
void Console::PrintRawInternal(const char *txt, int n) {
	int     y;
	int     c, l;

	// Need to do this first
	PrintPutchar();

	for (int i = 0; i < n; i++) {
		c = *txt;

		if (_wordWrap) {
			// count word length
			for (l = 0 ; l < _lineWidth && l < n; l++)
				if (txt[l] <= ' ') break;

			// word wrap
			if (l != _lineWidth && (_xOff + l > _lineWidth))
				_xOff = 0;
		}

		txt++;

		if (_cr) {
			_current--;
			_cr = false;
		}

		if (!_xOff) {
			Linefeed();
			// mark time for transparent overlay
			if (_current >= 0) _times[_current % CON_NUM_TIMES] = _frameNum;
		}

		switch (c) {
		case '\n':
			_xOff = 0;
			break;

		case '\r':
			_xOff = 0;
			_cr = true;
			break;

		default:    // _display character and advance
			y = _current % _totalLines;
			_text[y * _lineWidth + _xOff] = static_cast<char>(c);
			_xOff++;
			if (_xOff >= _lineWidth) _xOff = 0;
			break;
		}

	}
}

// Add a linefeed to the buffer
void Console::Linefeed(void) {
	_xOff = 0;
	_display++;
	_current++;
	Std::memset(&_text[(_current % _totalLines)*_lineWidth], ' ', _lineWidth);

	if (_autoPaint) _autoPaint();
}

// Print a _text string to the console
void Console::PutcharInternal(int c) {
	// Add the character
	_putChar_buf[_putChar_count] = static_cast<char>(c);

	// Increment the counter
	_putChar_count++;

	// If it was a space or less, or we've hit the limit we'll add it to the
	// actual buffer
	if (c <= ' ' || _putChar_count == (CON_PUTCHAR_SIZE - 1)) PrintPutchar();
}

// Print the Putchar data, if possible
void Console::PrintPutchar() {
	if (!_putChar_count) return;

	// Get the count
	//int count = _putChar_count; //Darke: UNUSED?

	// Terminate the string
	_putChar_buf[_putChar_count] = 0;

	// Clear the counter
	_putChar_count = 0;

	// Print it
	PrintInternal(_putChar_buf);
}

//
// STDOUT Methods
//

// Print a _text string to the console, and output to stdout
void Console::Print(const char *txt) {
	if (_stdOutputEnabled & CON_STDOUT)
		debug("%s", txt);

	if (_stdout_redir)
		_stdout_redir->write(txt, Std::strlen(txt));
	PrintInternal(txt);
}

// Print a _text string to the console, and output to stdout, with message filtering
void Console::Print(const MsgMask mm, const char *txt) {
	if (mm & _msgMask) Print(txt);
}

// printf, and output to stdout
int32 Console::Printf(const char *fmt, ...) {
	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stdout, with message filtering.
int32 Console::Printf(const MsgMask mm, const char *fmt, ...) {
	if (!(mm & _msgMask)) return 0;

	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stdout (va_list)
int32 Console::vPrintf(const char *fmt, va_list argptr) {
	Common::String str = Common::String::vformat(fmt, argptr);

	if (_stdOutputEnabled & CON_STDOUT) {
		debug("%s", str.c_str());
	}

	if (_stdout_redir)
		_stdout_redir->write(str.c_str(), str.size());
	PrintInternal(str.c_str());

	return str.size();
}

// Print a _text string to the console, and output to stdout
void Console::PrintRaw(const char *txt, int n) {
	if (_stdOutputEnabled & CON_STDOUT) {
		Common::String str(txt, txt + n);
		debug("%s", str.c_str());
	}

	if (_stdout_redir) _stdout_redir->write(txt, n);
	PrintRawInternal(txt, n);
}

// putchar, and output to stdout
void Console::Putchar(int c) {
	if (_stdOutputEnabled & CON_STDOUT)
		debug("%c", c);

	if (_stdout_redir) _stdout_redir->write1(c);
	PutcharInternal(c);
}


//
// STDERR Methods
//

// Print a _text string to the console, and output to stderr
void Console::Print_err(const char *txt) {
	if (_stdOutputEnabled & CON_STDERR)
		debug("%s", txt);

	if (_stderr_redir) _stderr_redir->write(txt, Std::strlen(txt));
	PrintInternal(txt);
}

// Print a _text string to the console, and output to stderr, with message filtering
void Console::Print_err(const MsgMask mm, const char *txt) {
	if (mm & _msgMask) Print_err(txt);
}

// printf, and output to stderr
int32 Console::Printf_err(const char *fmt, ...) {
	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf_err(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stderr, with message filtering
int32 Console::Printf_err(const MsgMask mm, const char *fmt, ...) {
	if (!(mm & _msgMask)) return 0;

	va_list argptr;

	va_start(argptr, fmt);
	int32 count = vPrintf_err(fmt, argptr);
	va_end(argptr);

	return count;
}

// printf, and output to stderr (va_list)
int32 Console::vPrintf_err(const char *fmt, va_list argptr) {
	Common::String str = Common::String::format(fmt, argptr);

	if (_stdOutputEnabled & CON_STDERR)
		debug("%s", str.c_str());

	if (_stderr_redir)
		_stderr_redir->write(str.c_str(), str.size());
	PrintInternal(str.c_str());

	return str.size();
}

// Print a _text string to the console, and output to stderr
void Console::PrintRaw_err(const char *txt, int n) {
	if (_stdOutputEnabled & CON_STDERR)
		debug("%d", n);

	if (_stderr_redir) _stderr_redir->write(txt, n);
	PrintRawInternal(txt, n);
}

// putchar, and output to stderr
void Console::Putchar_err(int c) {
	if (_stdOutputEnabled & CON_STDERR)
		debug("%c", c);

	if (_stderr_redir) _stderr_redir->write1(c);
	PutcharInternal(c);
}

void Console::ScrollConsole(int32 lines) {
	_display += lines;

	if (_display < 0) _display = 0;
	if (_display > _current) _display = _current;
}

//
// Console commands
//

void Console::AddConsoleCommand(const ArgsType &command, Console::Function function) {
	ConsoleCommands[command] = function;
}

void Console::RemoveConsoleCommand(Console::Function function) {
	for (CommandsMap::iterator it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it) {
		if (it->_value == function) {
			//pout << "Removing command: " << it->_key << Std::endl;
			it->_value = 0;
		}
	}
}
void Console::ExecuteConsoleCommand(const Console::ArgsType &args) {
	Console::ArgvType argv;
	StringToArgv(args, argv);

	ExecuteConsoleCommand(argv);
}

void Console::ExecuteConsoleCommand(const Console::ArgvType &argv) {
	CommandsMap::iterator it;

	// Empty?!?
	if (argv.empty())
		return;

	// Get the command name. Transparently handle conversions from original GUIApp
	Common::String commandName = argv[0];
	if (commandName.hasPrefix("GUIApp::"))
		commandName = "Ultima8Engine::" + Common::String(commandName.c_str() + 8);

	// Handle the command
	it = ConsoleCommands.find(commandName);

	if (it != ConsoleCommands.end() && it->_value)
		it->_value(argv);
	else
		Print(Common::String::format("Unknown command: %s\n", argv[0].c_str()).c_str());
}

void Console::ExecuteCommandBuffer() {
	if (commandBuffer.empty()) return;

	Console::ArgsType args = commandBuffer;
	commandBuffer.clear();

	// TODO: Fix this
	//pout << "]" << args << Std::endl;

	ExecuteConsoleCommand(args);

	commandHistory.push_back(args);
	commandHistoryPos = 0;
	commandCursorPos = 0;
}

void Console::ScrollCommandHistory(int num) {
	int total = commandHistory.size();

	// No history, don't do anything
	if (!total) return;

	if ((commandHistoryPos - num) <= 0) {
		if (commandHistoryPos == 1) return;
		commandHistoryPos = 1;
	} else
		commandHistoryPos -= num;

	if (commandHistoryPos > total)
		commandHistoryPos = total;

	commandBuffer = commandHistory[total - commandHistoryPos];
	commandCursorPos = commandBuffer.size();
}

void Console::ClearCommandBuffer() {
	commandBuffer.clear();
	commandCursorPos = 0;
}

void Console::AddCharacterToCommandBuffer(int ch) {
	// Enter (execute command)
	if (ch == Console::Enter) {

		ExecuteCommandBuffer();
	}
	// Backspace
	else if (ch == Console::Backspace) {

		DeleteCommandBufferChars(-1);
	}
	// Tab (command completion)
	else if (ch == Console::Tab) {

		if (!commandBuffer.empty()) {

			int count = 0;
			Console::ArgsType common;
			CommandsMap::iterator it;
			CommandsMap::iterator found;

			for (it = ConsoleCommands.begin(); it != ConsoleCommands.end(); ++it)
				if (it->_value) {
					if (it->_key.compareToIgnoreCase(commandBuffer))
						continue;

					if (!count) {
						common = it->_key;
						found = it;
					} else {
#ifdef TODO
						Console::ArgsType::iterator it1 = common.begin();
						Console::ArgsType::const_iterator it2 = it->_key.begin();
						int comsize = 0;

						while (it1 != common.end()) {
							if (!Console::ArgsType::traits_type::eq(*it1, *it2)) break;

							comsize++;
							++it1;
							++it2;
						}

						common.resize(comsize);
#endif
					}
					count++;
				}

			if (count) {
#ifdef TODO

				if (count > 1) {
					pout << "]" << commandBuffer << Std::endl;

					ArgsType args = "CmdList \"";
					args += commandBuffer;
					args += '\"';

					ArgvType argv;
					StringToArgv(args, argv);

					ConCmd_CmdList(argv);
					commandBuffer = common;
				} else
					commandBuffer = common;

				commandCursorPos = commandBuffer.size();
#endif
			}
		}
	}
	// Add the character to the command buffer
	else {
#ifdef TODO
		if (commandCursorPos == static_cast<int>(commandBuffer.size())) {
			commandBuffer += ch;
		} else if (commandInsert) {
			commandBuffer.insert(commandCursorPos, 1, ch);
		} else {
			commandBuffer[commandCursorPos] = ch;
		}

		commandCursorPos++;
#endif
	}
}

void Console::DeleteCommandBufferChars(int num) {
	if (!num || commandBuffer.empty()) return;

	if (num < 0) {
		num = -num;
		if (num > commandCursorPos) num = commandCursorPos;
		commandCursorPos -= num;
	} else {
		if ((num + commandCursorPos) > static_cast<int>(commandBuffer.size()))
			num = commandBuffer.size() - commandCursorPos;
	}

	commandBuffer.erase(commandCursorPos, num);
}

void Console::MoveCommandCursor(int num) {
	commandCursorPos += num;

	if (commandCursorPos < 0) commandCursorPos = 0;
	if (commandCursorPos > static_cast<int>(commandBuffer.size())) commandCursorPos = static_cast<int>(commandBuffer.size());
}

void Console::ConCmd_CmdList(const Console::ArgvType &argv) {
	CommandsMap::iterator it;
	int i = 0;

	//pout << Std::endl;

	if (argv.size() > 1) {
		for (size_t a = 1; a < argv.size(); a++) {
			const ArgsType &arg = argv[a];

			for (it = con->ConsoleCommands.begin(); it != con->ConsoleCommands.end(); ++it)
				if (it->_value) {
					if (it->_key.compareToIgnoreCase(arg)) continue;

					// TODO: Fix this
					//pout << " " << it->_key << Std::endl;
					i ++;
				}
		}
	} else {
		for (it = con->ConsoleCommands.begin(); it != con->ConsoleCommands.end(); ++it)
			if (it->_value) {
				// TODO
				//pout << " " << it->_key << Std::endl;
				i ++;
			}
	}

	// TODO
	//pout << i << " commands" << Std::endl;
}

void Console::ConCmd_CmdHistory(const Console::ArgvType & /*argv*/) {
#ifdef TODO
	Std::vector<ArgsType>::iterator it;

	for (it = con->commandHistory.begin(); it != con->commandHistory.end(); ++it)
		pout << " " << *it << Std::endl;

	pout << con->commandHistory.size() << " commands" << Std::endl;
#endif
}

/*
==============================================================================

DRAWING

==============================================================================
*/

void Console::DrawConsole(RenderSurface *surf, int height) {
	int             i, x, y;
	int             rows;
	int             row;
	int             lines;
/// char            version[64];

	// Need to do this first
	PrintPutchar();

	lines = height;
	if (lines <= 0)
		return;

	//if (lines > viddef.height)
	//  lines = viddef.height;

	//Com_sprintf (version, sizeof(version), "v%4.2f", VERSION);
	//for (x=0 ; x<5 ; x++)
	//  re.DrawChar (viddef.width-44+x*8, lines-12, 128 + version[x] );

	// draw the _text
	_visLines = lines;

#if 0
	rows = (lines - 8) >> 3;    // rows of _text to draw

	y = lines - 24;
#else
	rows = (lines / _conFont->_height) - 2;   // rows of _text to draw

	y = lines - (_conFont->_height * 3);
#endif

// draw from the bottom up
	if (_display != _current) {
		// draw arrows to show the buffer is backscrolled
		for (x = 0 ; x < _lineWidth ; x += 4)
			surf->PrintCharFixed(_conFont, '^', (x + 1) * _conFont->_width, y);

		y -= _conFont->_height;
		rows--;
	}

	row = _display;
	for (i = 0 ; i < rows ; i++, y -= _conFont->_height, row--) {
		if (row < 0)
			break;
		if (_current - row >= _totalLines)
			break;      // past scrollback wrap point

		char *txt = _text + (row % _totalLines) * _lineWidth;

		for (x = 0 ; x < _lineWidth ; x++) {
			surf->PrintCharFixed(_conFont, txt[x], (x + 1)*_conFont->_width, y);
			//  putchar (txt[x]);
		}
		//putchar ('\n');
	}

	const char *com = commandBuffer.c_str();
	int com_size = commandBuffer.size();
	int cur_pos = commandCursorPos;

	if (com_size >= (_lineWidth - 1)) {
		com_size = cur_pos;
	}

	//  prestep if horizontally scrolling
	if (com_size >= (_lineWidth - 1)) {
		com += 1 + com_size - (_lineWidth - 1);
		cur_pos = _lineWidth - 2;
	}

	y = lines - (_conFont->_height * 2);

	surf->PrintCharFixed(_conFont, ']', _conFont->_width, y);

	for (x = 0 ; x < (_lineWidth - 2) && com[x]; x++) {
		surf->PrintCharFixed(_conFont, com[x], (x + 2)*_conFont->_width, y);
		//  putchar (txt[x]);
	}

	// Now for cursor position
	if (commandInsert)
		surf->Fill32(0xFFFFFFFF, ((cur_pos + 2)*_conFont->_width) + 1, y, 2, _conFont->_height);
	else
		surf->Fill32(0xFFFFFFFF, ((cur_pos + 2)*_conFont->_width) + 1, y + _conFont->_height - 2, _conFont->_width, 2);
}


void Console::DrawConsoleNotify(RenderSurface *surf) {
	int     x, v;
	char    *txt;
	int     i;
	int     time;

	v = 0;
	for (i = _current - CON_NUM_TIMES + 1 ; i <= _current ; i++) {
		if (i < 0) continue;
		time = con->_times[i % CON_NUM_TIMES];
		if (time == 0) continue;

		time = _frameNum - time;
		//if (time > con_notifytime->value*1000)
		if (time > 150) // Each message lasts 5 seconds  (30*5=150 frames)
			continue;
		txt = _text + (i % _totalLines) * _lineWidth;

		for (x = 0 ; x < con->_lineWidth ; x++)
			surf->PrintCharFixed(_conFont, txt[x], (x + 1)*_conFont->_width, v);

		v += _conFont->_height;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
