if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Gradient

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Gradient;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating derivative function
$fun_1a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$var_1a=$fun_1a->xi();
$fun_1=new Cmiss::Function::Gradient(dependent=>$fun_1a->output(),independent=>$var_1a);
print "$fun_1\n";
print "\n";

# check evaluating with no arguments
$var_1=$fun_1->output();
$fun_2=$var_1->evaluate();
print "$var_1\n";
print "$fun_1\n";
if (defined($fun_2)&&($fun_2))
{
	print "$fun_2\n";
} else
{
	print "undefined\n";
}
# make another function to be used as a value when evaluating
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the function
#   being evaluated.  Check that function being evaluated isn't changed
$fun_4=$var_1->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_1($fun_3)=$fun_4\n";
$fun_4a=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a],input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_4a\n";
$return_code=($fun_1a->element_xi())->set_value($fun_3);
$fun_4b=$var_1->evaluate();
print "$return_code $fun_1, $fun_4b\n";
print "\n";

# check evaluating first derivative
$fun_6=$var_1->evaluate_derivative(independent=>[$var_1a]);
print "$fun_6\n";
$fun_6a=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a,$var_1a]);
print "$fun_6a\n";
$fun_7=new Cmiss::Function::Gradient(dependent=>$fun_1->output(),independent=>$var_1a);
$var_7=$fun_7->output();
$fun_7a=$var_7->evaluate();
print "$fun_7a\n";
# wrong independent
$fun_7b=$var_7->evaluate_derivative(independent=>[$var_1]);
print "$fun_7b\n";
$fun_7b=$var_7->evaluate_derivative(independent=>[$var_1a]);
print "$fun_7b\n";
$fun_7c=($fun_1a->output())->evaluate_derivative(independent=>[$var_1a,$var_1a,$var_1a]);
print "$fun_7c\n";
