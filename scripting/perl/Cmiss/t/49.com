if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Matrix::Divide_by_scalar

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function::Matrix::Divide_by_scalar;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");

# check creating divide by scalar variable
$fun_1a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$fun_1=new Cmiss::Function::Matrix::Divide_by_scalar($fun_1a->output(),$fun_1a->component(number=>1));
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
# make another variable to be used as a variable when evaluating
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$fun_4a=$fun_1a->output()->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_4a\n";
$fun_4=$var_1->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_1($fun_3)=$fun_4\n";
$return_code=($fun_1a->element_xi())->set_value($fun_3);
$fun_4a=$var_1->evaluate();
print "$return_code $fun_1 $fun_4a\n";
$fun_4b=$fun_1a->output()->evaluate(input=>$fun_1a->xi(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1]));
print "$fun_4b\n";
$fun_4b=$var_1->evaluate(input=>$fun_1a->xi(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1]));
print "$fun_1 $fun_4b\n";
print "\n";

# check evaluating with arguments with the arguments not being for the variable
#   being evaluated.  Also checks that the variable which was created by
#   evaluating has the correct type (Scalar, value is only defined for Scalar)
$fun_5a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$fun_5=$var_1->evaluate(input=>$fun_5a->xi(),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1]));
print "$fun_1 $fun_5\n";
print "\n";

# check evaluating first derivative
$fun_6a=$fun_1a->output()->evaluate();
$fun_6b=$fun_1a->output()->evaluate_derivative(independent=>[$fun_1a->xi()]);
print "$fun_6a $fun_6b\n";
$fun_6=$var_1->evaluate_derivative(independent=>[$fun_1a->xi()]);
print "$fun_1 derivative $fun_6\n";
$fun_6a=$var_1->evaluate_derivative(independent=>[$fun_1a->element_xi()]);
print "$fun_6a\n";
$fun_6b=$var_1->evaluate_derivative(independent=>[new Cmiss::Function_variable::Composite($fun_1a->xi(index=>1),$fun_1a->xi(index=>3))]);
print "$fun_6b\n";
print "\n";

# check evaluating second derivative
$var_2=$fun_1a->xi();
$fun_8a=$fun_1a->output()->evaluate();
$fun_8b=$fun_1a->output()->evaluate_derivative(independent=>[$var_2,$var_2]);
print "$fun_8a $fun_8b\n";
$fun_8=$var_1->evaluate_derivative(independent=>[$var_2,$var_2]);
print "$fun_8\n";
$var_2a=new Cmiss::Function_variable::Composite($fun_1a->xi(index=>1),$fun_1a->xi(index=>3));
$fun_8a=$var_1->evaluate_derivative(independent=>[$var_2a,$var_2]);
print "$fun_8a\n";
print "\n";
