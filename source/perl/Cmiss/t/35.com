if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian

use Cmiss::Function;
use Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;
use Cmiss::Function::Composite;

# check creating prolate_spheroidal_to_rectangular_cartesian function
$fun_1=new Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian(0.870552,0.479966,0.423234,35.25);
print "$fun_1\n";
# check evaluating
$var_1=$fun_1->output();
$fun_2=$var_1->evaluate();
print "$fun_2 $var_1\n";
# make another function to be used as an variable when evaluating
$fun_3=new Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian();
print "$fun_3 $var_1\n";
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$fun_4=$var_1->evaluate(input=>$fun_1->focus(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[1]));
print "$fun_4 $var_1\n";
# check evaluating with arguments with the arguments not being for the function
#   being evaluated.  Also checks that the function which was created by
#   evaluating has the correct type
#   (Prolate_spheroidal_to_rectangular_cartesian, focus is only defined
#   for Prolate_spheroidal_to_rectangular_cartesian)
$fun_5=$var_1->evaluate(input=>$fun_3->focus(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[1]));
print "$fun_5 $var_1\n";
# check evaluating first derivative
$fun_6=$var_1->evaluate_derivative(independent=>[$fun_1->prolate()]);
print "$fun_6\n";
$fun_6a=$var_1->evaluate_derivative(independent=>[$fun_1->focus()]);
print "$fun_6a\n";
$fun_7=$var_1->evaluate_derivative(independent=>[$fun_3->prolate()]);
print "$fun_7\n";
# check evaluating second derivative
$fun_8=$var_1->evaluate_derivative(independent=>[$fun_1->prolate(),$fun_1->prolate()]);
print "$fun_8\n";
$fun_8a=$var_1->evaluate_derivative(independent=>[$fun_1->prolate(),$fun_1->focus()]);
print "$fun_8a\n";
$fun_8b=$var_1->evaluate_derivative(independent=>[$fun_1->focus(),$fun_1->focus()]);
print "$fun_8b\n";
# check creating composite variable
$var_2=new Cmiss::Function_variable::Composite($fun_1->prolate(),$fun_1->focus());
# check evaluating first derivative wrt composite
$fun_9=$var_1->evaluate_derivative(independent=>[$var_2]);
print "$fun_9\n";
# check evaluating second derivative wrt composite
$fun_10=$var_1->evaluate_derivative(independent=>[$var_2,$var_2]);
print "$fun_10\n";
## check getting and setting values
$fun_11=($fun_1->lambda())->evaluate();
print "$fun_11\n";
$fun_11a=($fun_1->mu())->evaluate();
print "$fun_11a\n";
$fun_11b=($fun_1->theta())->evaluate();
print "$fun_11b\n";
$fun_11c=($fun_1->focus())->evaluate();
print "$fun_11c\n";
$fun_11d=($fun_1->prolate())->evaluate();
print "$fun_11d\n";
$fun_11e=$var_2->evaluate();
print "$fun_11e\n";
$return_code=($fun_1->focus())->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[1]));
$fun_11f=$var_1->evaluate();
print "$fun_11f\n";
