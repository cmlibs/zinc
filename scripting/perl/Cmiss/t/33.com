if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Element_xi

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");

# check creating element/xi function
$function_1=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.3,0.4,0.5]);
$variable_1=$function_1->output();
print "$function_1\n";
print "$variable_1\n";
print "\n";

# check evaluating with no arguments
$function_2=$variable_1->evaluate();
$variable_2=$function_2->output();
print "$function_2\n";
print "$variable_2\n";
print "\n";

# check evaluating
$function_3=new Cmiss::Function::Element_xi(element=>$heart->get_element(name=>22),xi=>[0.6,0.7,0.8]);
$function_4=$variable_1->evaluate(input=>$function_1->input(),value=>$function_3);
print "$function_3\n";
print "$function_4\n";
print "$function_1\n";
print "\n";
$function_5=$variable_1->evaluate(input=>$function_1->element(),value=>new Cmiss::Function::Element(element=>$heart->get_element(name=>22)));
print "$function_5\n";
print "$function_1\n";
print "\n";
$function_6=$variable_1->evaluate(input=>$function_1->xi(),value=>new Cmiss::Function::Matrix((n_columns=>3,values=>[0.6,0.7,0.8])));
print "$function_6\n";
print "$function_1\n";
print "\n";
$function_7=$variable_1->evaluate(input=>$function_1->element(),value=>$function_3);
print "$function_7\n";
print "$function_1\n";
print "\n";
$function_8=$variable_1->evaluate(input=>$function_1->xi(),value=>$function_3);
print "$function_8\n";
print "$function_1\n";
print "\n";
$function_9=$variable_1->evaluate(input=>$function_1->element(),value=>new Cmiss::Function::Matrix((n_columns=>3,values=>[0.6,0.7,0.8])));
print "$function_9\n";
print "$function_1\n";
print "\n";
$function_10=$variable_1->evaluate(input=>$function_1->xi(),value=>new Cmiss::Function::Matrix((n_columns=>1,values=>[0.6,0.7,0.8])));
print "$function_10\n";
print "$function_1\n";
print "\n";
$function_11=$variable_1->evaluate(input=>$function_1->xi(),value=>new Cmiss::Function::Matrix((n_columns=>1,values=>[0.6,0.7])));
print "$function_11\n";
print "$function_1\n";
print "\n";
$function_12=$variable_1->evaluate(input=>$function_1->xi(),value=>new Cmiss::Function::Matrix((n_columns=>1,values=>[0.6,0.7,0.8,0.9])));
print "$function_12\n";
print "$function_1\n";
print "\n";

$function_13=$variable_1->evaluate(input=>$function_1->xi(index=>2),value=>new Cmiss::Function::Matrix((n_columns=>1,values=>[0.6,0.7,0.8,0.9])));
print "$function_13\n";
print "$function_1\n";
print "\n";

# check evaluating_derivative
$function_21=$variable_1->evaluate_derivative(independent=>[$function_1->xi()]);
print "$function_21\n";
$function_22=$variable_1->evaluate_derivative(independent=>[$function_1->input()]);
print "$function_22\n";
$function_23=$variable_1->evaluate_derivative(independent=>[$function_1->element()]);
if (defined($function_23)&&($function_23))
{
	print "$function_23\n";
} else
{
	print "undefined\n";
}
$function_24=$variable_1->evaluate_derivative(independent=>[$function_1->xi(index=>3)]);
print "$function_24\n";
$function_25=$variable_1->evaluate_derivative(independent=>[$function_1->xi(index=>2)]);
print "$function_25\n";
$function_26=$variable_1->evaluate_derivative(independent=>[$function_1->xi(index=>1)]);
print "$function_26\n";
$function_27=($function_1->xi(index=>1))->evaluate_derivative(independent=>[$function_1->xi()]);
print "$function_27\n";
