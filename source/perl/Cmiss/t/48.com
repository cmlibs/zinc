if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Linear_span

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Linear_span;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Composite;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");
# check creating finite element variable
$fun_1a=new Cmiss::Function::Finite_element(region=>$heart,name=>'coordinates');
$var_1a=$fun_1a->output();
$fun_1=new Cmiss::Function::Linear_span(spanned=>$fun_1a->output(),spanning=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)));
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
print "\n";

# make another variable to be used as an variable when evaluating
$fun_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
# check evaluating with arguments with the arguments being for the variable
#   being evaluated.  Check that variable being evaluated isn't changed
$fun_4=$var_1->evaluate(input=>$fun_1a->element_xi(),value=>$fun_3);
print "$fun_1($fun_3)=$fun_4\n";
$return_code=($fun_1a->element_xi())->set_value($fun_3);
$fun_4a=$var_1->evaluate();
print "$return_code $fun_1 $fun_4a\n";
$fun_4a__1=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[1,0,0,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__1\n";
$fun_4a__2=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,1,0,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__2\n";
$fun_4a__3=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__3\n";
$fun_4a__4=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,1,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__4\n";
$fun_4a__5=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,1,0,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__5\n";
$fun_4a__6=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,1,0,0,0,0,0,0,0,0,0]));
print "$fun_4a__6\n";
$fun_4a__7=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,1,0,0,0,0,0,0,0,0]));
print "$fun_4a__7\n";
$fun_4a__8=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,1,0,0,0,0,0,0,0]));
print "$fun_4a__8\n";
$fun_4a__9=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,1,0,0,0,0,0,0]));
print "$fun_4a__9\n";
$fun_4a_10=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,1,0,0,0,0,0]));
print "$fun_4a_10\n";
$fun_4a_11=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,1,0,0,0,0]));
print "$fun_4a_11\n";
$fun_4a_12=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,1,0,0,0]));
print "$fun_4a_12\n";
$fun_4a_13=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,1,0,0]));
print "$fun_4a_13\n";
$fun_4a_14=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,0,1,0]));
print "$fun_4a_14\n";
$fun_4a_15=$var_1a->evaluate(input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,0,0,1]));
print "$fun_4a_15\n";
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
$fun_6=$var_1->evaluate_derivative(independent=>[$fun_1a->xi()]);
print "$fun_1 derivative $fun_6\n";
$fun_6a__1=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[1,0,0,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__1\n";
$fun_6a__2=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,1,0,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__2\n";
$fun_6a__3=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__3\n";
$fun_6a__4=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,1,0,0,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__4\n";
$fun_6a__5=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,1,0,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__5\n";
$fun_6a__6=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,1,0,0,0,0,0,0,0,0,0]));
print "$fun_6a__6\n";
$fun_6a__7=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,1,0,0,0,0,0,0,0,0]));
print "$fun_6a__7\n";
$fun_6a__8=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,1,0,0,0,0,0,0,0]));
print "$fun_6a__8\n";
$fun_6a__9=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,1,0,0,0,0,0,0]));
print "$fun_6a__9\n";
$fun_6a_10=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,1,0,0,0,0,0]));
print "$fun_6a_10\n";
$fun_6a_11=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,1,0,0,0,0]));
print "$fun_6a_11\n";
$fun_6a_12=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,1,0,0,0]));
print "$fun_6a_12\n";
$fun_6a_13=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,1,0,0]));
print "$fun_6a_13\n";
$fun_6a_14=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,0,1,0]));
print "$fun_6a_14\n";
$fun_6a_15=$var_1a->evaluate_derivative(independent=>[$fun_1a->xi()],input=>$fun_1a->nodal_values(node=>$heart->get_node(name=>28)),value=>new Cmiss::Function::Matrix(n_columns=>1,values=>[0,0,0,0,0,0,0,0,0,0,0,0,0,0,1]));
print "$fun_6a_15\n";
$fun_6a=$var_1->evaluate_derivative(independent=>[$fun_1a->element_xi()]);
print "$fun_6a\n";
$fun_6b=$var_1->evaluate_derivative(independent=>[new Cmiss::Function_variable::Composite($fun_1a->xi(index=>1),$fun_1a->xi(index=>3))]);
print "$fun_6b\n";
$fun_6c=$var_1->evaluate_derivative(independent=>[$fun_5a->xi()]);
if (defined($fun_6c)&&($fun_6c))
{
	print "$fun_6c\n";
} else
{
	print "undefined\n";
}
print "\n";

# check evaluating second derivative
$var_2=$fun_1a->xi();
$fun_8=$var_1->evaluate_derivative(independent=>[$var_2,$var_2]);
print "$fun_8\n";
$var_2a=new Cmiss::Function_variable::Composite($fun_1a->xi(index=>1),$fun_1a->xi(index=>3));
$fun_8a=$var_1->evaluate_derivative(independent=>[$var_2,$var_2a]);
$fun_8a=$var_1->evaluate_derivative(independent=>[$var_2a,$var_2]);
print "$fun_8a\n";
