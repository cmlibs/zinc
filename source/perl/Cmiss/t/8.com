if (!defined $path)
{
  $path = ".";
}
use Cmiss::Cmgui_command_data;
use Cmiss::Value::Element_xi;
use Cmiss::Value::FE_value_vector;
use Cmiss::Value::Derivative_matrix;
use Cmiss::Variable::Derivative;
use Cmiss::Variable::Element_xi;
use Cmiss::Variable::Finite_element;
use Cmiss::Region;
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
$coordinates=new Cmiss::Variable::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var');
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$element_xi_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
$matrix=$result->matrix(independent=>[$element_xi_var]);
print "$matrix\n";
Cmiss::Value::Matrix->set_string_convert_max_columns(20);
print "$result\n";
$element_xi=0;
$coordinates=0;
$heart=0;
$root=0;
