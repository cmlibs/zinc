if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Matrix::Sum

use Cmiss::Function;
use Cmiss::Function::Matrix;
use Cmiss::Function::Matrix::Sum;
use Cmiss::Function_variable;
use Cmiss::Function::Composite;

# check creating matrix sum function
$function_1=new Cmiss::Function::Matrix(n_columns=>2,values=>[1,5,-3,8,2,4]);
$variable_1=$function_1->output();
$function_2=new Cmiss::Function::Matrix(n_columns=>2,values=>[-1,-5,3,-8,-2,-4]);
$variable_2=$function_2->output();
$function_3=new Cmiss::Function::Matrix(n_columns=>2,values=>[-9,-4,2,0,5,-11]);
$variable_3=$function_3->output();
$function_4=new Cmiss::Function::Matrix::Sum($variable_1,$variable_2);
$variable_4=$function_4->output();
$function_5=new Cmiss::Function::Matrix::Sum($variable_3,$variable_4);
$variable_5=$function_5->output();
print "$function_4 $variable_4\n";
print "\n";

# get value without evaluating
$function_6=$function_5->sub_matrix(row_low=>1,row_high=>3,column_low=>1,column_high=>2);
print "$function_6\n";
print "\n";

# check evaluating with no arguments
$function_7=$variable_5->evaluate();
print "$function_7\n";
print "\n";

# get value without evaluating
$function_9=$function_5->sub_matrix(row_low=>1,row_high=>3,column_low=>1,column_high=>2);
print "$function_9\n";
print "\n";

# evaluate derivative
$function_9=$variable_5->evaluate_derivative(independent=>[$variable_1]);
print "$function_9\n";
$function_10=$variable_5->evaluate_derivative(independent=>[$function_4->input()]);
print "$function_10\n";
$function_11=$variable_5->evaluate_derivative(independent=>[$function_5->input()]);
print "$function_11\n";
print "\n";

# check sub-matrix
$function_12=$function_5->sub_matrix(row_low=>2,row_high=>3,column_low=>2,column_high=>2);
print "$function_12\n";
print "\n";

# make another function to be used as a value when evaluating
$function_13=new Cmiss::Function::Matrix(n_columns=>2,values=>[9,-1,6,7,11,3]);
print "$function_13 $function_5\n";
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$function_14=$variable_5->evaluate(input=>$function_1->input(),value=>$function_13);
print "$function_14 $function_1\n";
# check evaluating with arguments with the arguments not being for the function
#   being evaluated.  Also checks that the function which was created by
#   evaluating has the correct type (Scalar, variable_value is only defined for
#   Scalar)
$function_15=$variable_5->evaluate(input=>$function_13->input(),value=>$function_13);
print "$function_15 $function_1\n";
