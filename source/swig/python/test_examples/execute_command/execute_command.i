/* execute_command.i */

struct Cmiss_command_data;
%module execute_command
%{
extern int Cmiss_command_data_execute_command(struct Cmiss_command_data *command_data, const char *command);
extern struct Cmiss_command_data *create_Cmiss_command_data(int argc, char *argv[], char *version_string);
%}

extern int Cmiss_command_data_execute_command(struct Cmiss_command_data *command_data, const char *command);
extern struct Cmiss_command_data *create_Cmiss_command_data(int argc, char *argv[], char *version_string);

