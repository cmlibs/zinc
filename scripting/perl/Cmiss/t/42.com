if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Matrix::Trace

use Cmiss::Function;
use Cmiss::Function::Matrix;
use Cmiss::Function::Matrix::Trace;
use Cmiss::Function_variable;
use Cmiss::Function::Composite;

# check creating matrix trace function
$function_1=new Cmiss::Function::Matrix(n_columns=>3,values=>[1,5,-3,8,2,4,9,0,-7]);
$variable_1=$function_1->output();
$function_2=new Cmiss::Function::Matrix::Trace($variable_1);
$variable_2=$function_2->output();
print "$function_2 $variable_2\n";
# non-square matrix
$function_3=new Cmiss::Function::Matrix(n_columns=>2,values=>[-1,-5,3,-8,-2,-4]);
$variable_3=$function_3->output();
$function_4=new Cmiss::Function::Matrix::Trace($variable_3);
if (defined($function_4)&&($function_4))
{
	print "$function_4\n";
} else
{
	print "undefined\n";
}
print "\n";

# check evaluating with no arguments
$function_5=$variable_2->evaluate();
print "$function_5\n";
print "\n";

# evaluate derivative
$function_6=$variable_2->evaluate_derivative(independent=>[$variable_1]);
print "$function_6\n";
# second derivative
$function_7=$variable_2->evaluate_derivative(independent=>[$variable_1,$variable_1]);
print "$function_7\n";
print "\n";

# make another function to be used as a value when evaluating
$function_8=new Cmiss::Function::Matrix(n_columns=>3,values=>[9,-1,6,7,11,3,8,2,2]);
print "$function_8\n";
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$function_9=$variable_2->evaluate(input=>$function_1->input(),value=>$function_8);
print "$function_9 $function_1\n";
# check evaluating with arguments with the arguments not being for the function
#   being evaluated.  Also checks that the function which was created by
#   evaluating has the correct type (Scalar, variable_value is only defined for
#   Scalar)
$function_10=$variable_2->evaluate(input=>$function_8->input(),value=>$function_8);
print "$function_10 $function_1\n";
