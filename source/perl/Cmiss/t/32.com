if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Function::Element

use Cmiss::Cmgui_command_data;
use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Composite;
use Cmiss::Function::Element;
use Cmiss::Function_variable;

# set up regions
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');

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
