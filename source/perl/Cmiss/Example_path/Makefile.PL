use ExtUtils::MakeMaker;

#
# Problems with "Manifying" in a FAT32 filesystem because Cmiss::Cmgui_command_data.3 is not
#   a valid file name
WriteMakefile(
    'NAME'    => 'Cmiss::Example_path',
  #          
    'VERSION_FROM'  => 'Example_path.pm', # finds $VERSION
    'PREREQ_PM'    => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Example_path.pm', # retrieve abstract from module
       AUTHOR     => '"Shane Blackett <s.blackett@auckland.ac.nz>"') : ()),
    'LIBS'    => [''], # e.g., '-lm'
    'DEFINE'    => '-DPERL_INTERPRETER', # e.g., '-DHAVE_SOMETHING'
  # Insert -I. if you add *.h files later:
    'INC'    => "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
  # Un-comment this if you add C files to link with later:
    # 'OBJECT'    => '$(O_FILES)', # link all the C files too
  # Include the typemap files of any directories above here
  #  'TYPEMAPS'    =>  \@typemap_array,
);
