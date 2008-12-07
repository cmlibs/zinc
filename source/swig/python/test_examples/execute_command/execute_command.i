/* execute_command.i */

%module execute_command
%{
extern int Cmiss_command_data_execute_command(struct Cmiss_command_data *command_data, const char *command);
%}

extern int Cmiss_command_data_execute_command(struct Cmiss_command_data *command_data, const char *command);

