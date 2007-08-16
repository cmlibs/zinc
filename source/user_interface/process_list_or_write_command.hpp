/*******************************************************************************
FILE : process_list_or_write_command.hpp

LAST MODIFIED : 15 August 2007

DESCRIPTION :
Class built for gfx write all and gfx list all_commands. This class
will process either list commands or write to comfile commands
==============================================================================*/
#if !defined (PROCESS_LIST_OR_WRITE_COMMAND_H)
#define PROCESS_LIST_OR_WRITE_COMMAND_H

extern "C" {
#include <stdarg.h>
}

#define MESSAGE_STRING_SIZE 1000
static char message_string[MESSAGE_STRING_SIZE];

class Process_list_or_write_command_class
{
public:
	 Process_list_or_write_command_class()
	 {
	 };

	 virtual int process_command(enum Message_type message_type,char *format,...) = 0;

	 virtual ~Process_list_or_write_command_class() { }
};

class Process_list_command_class : public Process_list_or_write_command_class
{
public:
	 Process_list_command_class()
	 {
	 };

	 int process_command(enum Message_type message_type,char *format,...)
	 {
			int return_code;
			va_list ap;
			va_start(ap, format);
			vsprintf(message_string,format,ap);
			return_code = display_message(message_type, message_string, "");
			va_end(ap);
			return (return_code);
	 };
};

class Process_write_command_class : public Process_list_or_write_command_class
{
public:
	 Process_write_command_class()
	 {
	 };

	 int process_command(enum Message_type message_type,char *format,...)
	 {
			int return_code;
			va_list ap;
			va_start(ap, format);
			vsprintf(message_string,format,ap);
			return_code = write_message_to_file(message_type, message_string,"");
			va_end(ap);
			return (return_code);
	 } 
};

#endif /* define PROCESS_LIST_OR_WRITE_COMMAND_H */
