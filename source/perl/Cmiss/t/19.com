if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Matrix

use Cmiss::Variable_new;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new::Matrix;
use Cmiss::Variable_new::Scalar;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# check creating matrix variable
$var_1=new Cmiss::Variable_new::Matrix(n_columns=>2,values=>[1,5,-3,8,2,4]);
print "$var_1\n";
# check sub-matrix
$var_1a=$var_1->sub_matrix(row_low=>2,row_high=>3,column_low=>2,column_high=>2);
print "$var_1a\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_2 $var_1\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Matrix(n_columns=>2,values=>[9,-1,6,7,11,3]);
print "$var_3 $var_1\n";
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
#???DB.  Something wrong value not being set
$var_4=$var_1->evaluate($var_1->input_values(1,2),new Cmiss::Variable_new::Scalar(value=>-3));
print "$var_4 $var_1\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, input_value is only defined for
#   Scalar)
$var_5=$var_1->evaluate($var_2->input_values(3,1),new Cmiss::Variable_new::Scalar(value=>-3));
print "$var_5 $var_1\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1->input_values()]);
print "$var_6\n";
$var_6a=$var_1->evaluate_derivative(independent=>[$var_1->input_values(1,2)]);
print "$var_6a\n";
$var_6b=$var_1->evaluate_derivative(independent=>[$var_1->input_values(1,2,3,1)]);
print "$var_6b\n";
$var_7=$var_1->evaluate_derivative(independent=>[$var_2->input_values()]);
print "$var_7\n";
# check evaluating second derivative
$input_1=$var_1->input_values(1,2);
$var_8=$var_1->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_8\n";
$input_1a=$var_1->input_values(1,2,3,1);
$var_8a=$var_1->evaluate_derivative(independent=>[$input_1,$input_1a]);
print "$var_8a\n";
$var_8b=$var_8a->matrix($input_1a);
print "$var_8b\n";
# check creating composite input
$input_2=new Cmiss::Variable_new_input::Composite($input_1,$var_2->input_values(3,2));
$var_8c=$var_8a->matrix($input_2);
print "$var_8c\n";
# check evaluating first derivative wrt composite
$var_9=$var_1->evaluate_derivative(independent=>[$input_2]);
print "$var_9\n";
$var_10=$var_1->evaluate_derivative(independent=>[new Cmiss::Variable_new_input::Composite($var_2->input_values(),$input_1)]);
print "$var_10\n";
# check getting and setting input values
$var_11=$var_1->get_input_value($input_1);
print "$var_11\n";
$var_11a=$var_1->get_input_value($input_1a);
print "$var_11a\n";
$return_code=$var_1->set_input_value($input_1,new Cmiss::Variable_new::Vector(-3));
print "$return_code $var_1\n";
$return_code=$var_1->set_input_value($input_1a,new Cmiss::Variable_new::Vector(2,4));
print "$return_code $var_1\n";
$var_12=$var_1->get_input_value($var_2->input_values());
print "$var_12\n";
$var_12a=$var_1->get_input_value($var_1->input_values());
print "$var_12a\n";
$return_code=$var_1->set_input_value($var_2->input_values(),$var_2);
print "$return_code $var_1\n";
$return_code=$var_1->set_input_value($var_1->input_values(),$var_2);
print "$return_code $var_1\n";
$return_code=1;
