if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Element

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element;
use Cmiss::Function_variable;

# set up regions
$heart=new Cmiss::Region();
$heart->read_file(name=>"$path/heart.exnode");
$heart->read_file(name=>"$path/heart.exelem");

# check creating element/xi function
$function_1=new Cmiss::Function::Element(element=>$heart->get_element(name=>21));
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
$function_3=new Cmiss::Function::Element(element=>$heart->get_element(name=>22));
$function_4=$variable_1->evaluate(input=>$function_1->input(),value=>$function_3);
print "$function_3\n";
print "$function_4\n";
print "$function_1\n";
