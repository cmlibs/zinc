use ExtUtils::MakeMaker;

#Recursively look up from here for typemap files.
my $typemap_file = "typemap";
my @typemap_array = ();
while ( -f $typemap_file )
  {
	 #We want the typemap closest to us to have the highest precedence which
	 #is last in the list.
	 unshift @typemap_array, $typemap_file;
	 $typemap_file = "../$typemap_file";
  }

unshift @typemap_array, "$ENV{CMGUI_DEV_SOURCE}/perl/Cmiss/Scene_viewer/typemap";

WriteMakefile(
    'NAME'		=> 'Cmiss::Graphics_window',
    'VERSION_FROM'	=> 'Graphics_window.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Graphics_window.pm', # retrieve abstract from module
       AUTHOR     => '"Shane Blackett <s.blackett@auckland.ac.nz>"') : ()),
    'LIBS'		=> [''], # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
	# Insert -I. if you add *.h files later:
    'INC'		=> "-I$ENV{CMGUI_DEV_SOURCE}", # e.g., '-I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
   'TYPEMAPS'    =>  \@typemap_array,
);
