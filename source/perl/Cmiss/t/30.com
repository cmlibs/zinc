if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function_variable::Intersection

use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Function_variable::Intersection;

# check creating intersection variable
$function_1=new Cmiss::Function::Matrix(n_columns=>2,values=>[1,5,-3,8,2,4]);
$function_2=new Cmiss::Function::Matrix(n_columns=>3,values=>[9,-1,6,7,11,3]);
$variable_1=new Cmiss::Function_variable::Intersection($function_1->output(),$function_2->output());
$function_2a=$variable_1->evaluate();
#print "$function_1 $function_2 $function_2a\n";
if (defined($function_1)&&($function_1))
{
	print "$function_1";
} else
{
	print "undefined";
}
print " ";
if (defined($function_2)&&($function_2))
{
	print "$function_2";
} else
{
	print "undefined";
}
print " ";
if (defined($function_2a)&&($function_2a))
{
	print "$function_2a";
} else
{
	print "undefined";
}
print "\n";
print "\n";

# check evaluating
$function_3=$variable_1->evaluate();
if (defined($function_3)&&($function_3))
{
	print "$function_3\n";
} else
{
	print "undefined\n";
}
print "\n";

# check evaluating_derivative
$function_3=$variable_1->evaluate_derivative(independent=>[$function_1->output()]);
print "$function_3\n";
$function_4=$variable_1->evaluate_derivative(independent=>[new Cmiss::Function_variable::Intersection($function_2->output(),$function_1->output())]);
print "$function_4\n";
$function_5=$variable_1->evaluate_derivative(independent=>[$variable_1]);
print "$function_5\n";
print "\n";

# check entry
$variable_2=new Cmiss::Function_variable::Intersection($function_1->row(2),$function_1->column(1),$function_1->entry(2,1));
$function_6=$variable_2->evaluate();
print "$function_1 $variable_2 $function_6\n";
$function_7=($function_1->output())->evaluate_derivative(independent=>[$variable_2]);
print "$function_7\n";
