package Cmiss::Example_path;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Value ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	$example
	$TESTING
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);

our $VERSION = '0.01';

#Currently implemented by loading cmgui command data and then copying the
#variable from the default execution package (cmiss).
#I have put it into this separate module so that in the future we may
#find the $example directory without the full command_data.

use Cmiss::Cmgui_command_data;
our $example = $cmiss::example;
our $TESTING = $cmiss::TESTING;

1;
__END__

=head1 NAME

Cmiss::Example_path - Perl extension to export example directory

=head1 SYNOPSIS

  use Cmiss::Example_path qw($example);

=head1 ABSTRACT

  Exports the $example directory from Cmgui.

=head1 DESCRIPTION

  Exports the $example directory from Cmgui.

=head2 EXPORT

None by default.

=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <s.blackett@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2004 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

