if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Variable_new::Element_xi

use Cmiss::Region;
use Cmiss::Variable_new;
use Cmiss::Variable_new::Element_xi;
use Cmiss::Variable_new::Vector;
use Cmiss::Variable_new::Matrix;
use Cmiss::Variable_new::Derivative_matrix;
use Cmiss::Variable_new_input;
use Cmiss::Variable_new_input::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating element/xi variable
$var_1=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
print "$var_1\n";
# check evaluating with no arguments
$var_2=$var_1->evaluate();
print "$var_2, $var_1\n";
# make another variable to be used as an input when evaluating
$var_3=new Cmiss::Variable_new::Element_xi(element=>$heart->get_element(name=>22),xi=>[0.2,0.3,0.4]);
print "$var_3, $var_1\n";
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$var_4=$var_1->evaluate($var_1->input_element_xi(),$var_3);
print "$var_4, $var_1\n";
$var_4a=$var_1->evaluate($var_1->input_xi(1,3),$var_3);
print "$var_4a, $var_1\n";
$var_4b=$var_1->evaluate($var_1->input_xi(1,3),new Cmiss::Variable_new::Vector(.1,.7));
print "$var_4b, $var_1\n";
# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, input_value is only defined for
#   Scalar)
$var_5=$var_1->evaluate($var_2->input_element_xi(),$var_3);
print "$var_5, $var_1\n";
# check evaluating first derivative
$var_6=$var_1->evaluate_derivative(independent=>[$var_1->input_xi()]);
print "$var_6\n";
$var_6a=$var_1->evaluate_derivative(independent=>[$var_1->input_element_xi()]);
print "$var_6a\n";
$var_6b=$var_1->evaluate_derivative(independent=>[$var_1->input_xi(1,3)]);
print "$var_6b\n";
$var_7=$var_1->evaluate_derivative(independent=>[$var_2->input_xi()]);
print "$var_7\n";
# check evaluating second derivative
$input_1=$var_1->input_xi();
$var_8=$var_1->evaluate_derivative(independent=>[$input_1,$input_1]);
print "$var_8\n";
$input_1a=$var_1->input_xi(1,3);
$var_8a=$var_1->evaluate_derivative(independent=>[$input_1,$input_1a]);
print "$var_8a\n";
$var_8b=$var_8a->matrix($input_1a);
print "$var_8b\n";
# check creating composite input
$input_2=new Cmiss::Variable_new_input::Composite($input_1,$var_2->input_xi(2));
# check evaluating first derivative wrt composite
$var_9=$var_1->evaluate_derivative(independent=>[$input_2]);
print "$var_9\n";
$var_10=$var_1->evaluate_derivative(independent=>[new Cmiss::Variable_new_input::Composite($var_2->input_xi(2,3),$input_1)]);
print "$var_10\n";
# check getting and setting input values
$var_11=$var_1->get_input_value($input_1);
print "$var_11\n";
print "$var_1, $var_3\n";
$return_code=$var_1->set_input_value($input_1,$var_3);
print "$return_code $var_1\n";
print "$var_2, $var_3\n";
$return_code=$var_2->set_input_value($var_2->input_element_xi(),$var_3);
print "$return_code $var_4\n";
$var_12=$var_1->get_input_value($var_2->input_xi());
if (defined($var_12))
{
	print "$var_12\n";
} else
{
	print "undefined\n";
}
$return_code=$var_1->set_input_value($var_2->input_xi(),$var_2);
print "$return_code $var_1\n";
