if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Matrix

use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;

# check creating matrix function
$function_1=new Cmiss::Function::Matrix(n_columns=>2,values=>[1,5,-3,8,2,4]);
$variable_1=$function_1->output();
print "$function_1 $variable_1\n";
print "\n";

# check sub-matrix
$function_2=$function_1->sub_matrix(row_low=>2,row_high=>3,column_low=>2,column_high=>2);
print "$function_2\n";
print "\n";

# check evaluating with no arguments
$function_3=$variable_1->evaluate();
$variable_2=$function_3->output();
print "$variable_2 $variable_1\n";
# make another function to be used as an variable when evaluating
$function_4=new Cmiss::Function::Matrix(n_columns=>2,values=>[9,-1,6,7,11,3]);
print "$function_4 $function_1\n";
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$function_5=$variable_1->evaluate(input=>$function_1->input(),value=>$function_4);
$variable_3=$function_5->output();
print "$variable_3 $variable_1\n";
# check evaluating with arguments with the arguments not being for the function
#   being evaluated.  Also checks that the function which was created by
#   evaluating has the correct type (Scalar, variable_value is only defined for
#   Scalar)
$function_6=$variable_1->evaluate(input=>$function_4->input(),value=>$function_4);
$variable_4=$function_6->output();
print "$variable_4 $variable_1\n";
print "\n";

# check setting variable values
($function_1->input())->set_value($function_4);
print "$function_1 $variable_1\n";
print "\n";

#check solving
$function_7=new Cmiss::Function::Matrix(n_columns=>3,values=>[0,1,0,2,0,0,0,0,3]);
$function_8=new Cmiss::Function::Matrix(n_columns=>3,values=>[1,0,0,0,1,0,0,0,1]);
$function_9=$function_7->solve($function_8);
print "$function_7 $function_8 $function_9\n";
print "\n";

# evaluate derivative
$function_10=$variable_1->evaluate_derivative(independent=>[$variable_1]);
$variable_5=$function_10->output();
print "$variable_1 $variable_5\n";
print "\n";

# test entry
$variable_6=$function_1->row(2);
print "$function_1 $variable_6\n";
$variable_7=$function_1->column(1);
print "$function_1 $variable_7\n";
$variable_8=$function_1->entry(2,1);
print "$function_1 $variable_8\n";
$function_11=$variable_6->evaluate_derivative(independent=>[$variable_7]);
print "$function_11\n";
$function_12=$variable_7->evaluate_derivative(independent=>[$variable_8]);
print "$function_12\n";
