package Cmiss::Value::Derivative_matrix;

use 5.008;
use strict;
use warnings;
use Carp;

require Cmiss::Value;
require Cmiss::Value::Matrix;
require Cmiss::Variable;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Value Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Value::Derivative_matrix ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	new
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmiss::Value::Derivative_matrix::constant not defined" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#XXX	if ($] >= 5.00561) {
#XXX	    *$AUTOLOAD = sub () { $val };
#XXX	}
#XXX	else {
	    *$AUTOLOAD = sub { $val };
#XXX	}
    }
    goto &$AUTOLOAD;
}

#???debug
use Devel::Peek;

# Named argument
#???DB.  Change to passing the hash into create (xs)?
sub new
{
	my ($class, %arg) = @_;
	my ($dependent,$independent,$matrices,$objref);

	$matrices=$arg{matrices};
	$dependent=$arg{dependent};
	if ($dependent)
	{
		$independent=$arg{independent};
		if ($independent)
		{
			$objref=create($dependent,$independent,$matrices);
			if ($objref)
			{
				bless $objref,$class;
			}
			else
			{
				croak "Could not create $class";
			}
		}
		else
		{
			croak "Missing independent";
		}
	}
	else
	{
		croak "Missing dependent";
	}
}

# Overload string conversion
use overload '""' => \&string_convert, fallback => 1;

sub string_convert
{
	my %type=get_type(shift);
	my $dependent_variable=$type{dependent};
	my $independent_variables_ref=$type{independent};
	my $matrices_ref=$type{matrices};
	my $result_string='';
	my $i;

	if (($dependent_variable)&&($independent_variables_ref))
	{
		my @independent_variables= @$independent_variables_ref;
		my $order=@independent_variables;

		$result_string=$result_string."d";
		if ($order>1)
		{
			$result_string=$result_string."$order";
		}
		$result_string=$result_string."($dependent_variable)/";
		for ($i=0;$i<$order;$i++)
		{
			$result_string=$result_string."d($independent_variables[$i])";
		}
		if ($matrices_ref)
		{
			my @matrices= @$matrices_ref;
			
			$result_string=$result_string."=$matrices[-1]";
		}
	}
	$result_string;
}

sub matrix
{
	my ($self,%args)=@_;
	my ($independent,$objref);

	$independent=$args{independent};
	if ($independent)
	{
		$objref=get_matrix($self,$independent);
		if ($objref)
		{
			$objref;
		}
		else
		{
			croak "Could not get matrix";
		}
	}
	else
	{
		croak "Missing independent";
	}
}

require XSLoader;
XSLoader::load('Cmiss::Value::Derivative_matrix', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Value::Derivative_matrix - Perl extension for Cmiss derivative matrices

=head1 SYNOPSIS

  use Cmiss::Value::Derivative_matrix;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Value::Derivative_matrix.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Value::Derivative_matrix, created by h2xs. It looks like
the author of the extension was negligent enough to leave the stub
unedited.

=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

David Bullivant, <d.bullivant@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

