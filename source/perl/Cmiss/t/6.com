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
use Cmiss::Variable::Nodal_value;
use Cmiss::Region;
$cmgui_command_data = new Cmiss::Cmgui_command_data();
$cmgui_command_data->execute_command("gfx read nodes $path/heart");
$cmgui_command_data->execute_command("gfx read elements $path/heart");
$root=$cmgui_command_data->get_cmiss_root_region();
$heart=$root->get_sub_region(name=>'heart');
$coordinates=new Cmiss::Variable::Finite_element(fe_field=>$heart->get_field(name=>'coordinates'));
$element_xi=new Cmiss::Value::Element_xi(element=>$heart->get_element(name=>21),xi=>[0.5,0.5,0.5]);
$element_xi_var=new Cmiss::Variable::Element_xi(name=>'element_xi_var');
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates);
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
$matrix=$result->matrix(independent=>[$element_xi_var]);
print "$matrix\n";
$matrix=$result->matrix(independent=>[$nodal_value_var]);
Cmiss::Value::Matrix->set_string_convert_max_columns(50);
# Because nodes 28 and 13 are on the central axis the modify (decreasing in xi1)
#   for theta uses the values at the outside nodes (9, 24, 51 and 81) - quick
#   fix for when don't have versions/calculation on axis.  This means that the
#   values at 28 and 13 aren't contributing and you could say that the third
#   rows of the derivative matrices for 28 and 13 should be zero.  However this
#   is a special point/discontinuity and should be ignored?
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28=$sub_matrix\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24=$sub_matrix\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81=$sub_matrix\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13=$sub_matrix\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9=$sub_matrix\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51=$sub_matrix\n";
$matrix=$result->matrix(independent=>[$nodal_value_var,$element_xi_var]);
# Node 28
$sub_matrix=$matrix->sub_matrix(column_low=>172,column_high=>186);
print "Node 28, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>793,column_high=>807);
print "Node 28, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1414,column_high=>1428);
print "Node 28, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>28));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
# Node 24
$sub_matrix=$matrix->sub_matrix(column_low=>148,column_high=>153);
print "Node 24, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>769,column_high=>774);
print "Node 24, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1390,column_high=>1395);
print "Node 24, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>24));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
# Node 81
$sub_matrix=$matrix->sub_matrix(column_low=>508,column_high=>513);
print "Node 81, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1129,column_high=>1134);
print "Node 81, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1750,column_high=>1755);
print "Node 81, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>81));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
# Node 13
$sub_matrix=$matrix->sub_matrix(column_low=>73,column_high=>87);
print "Node 13, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>694,column_high=>708);
print "Node 13, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1315,column_high=>1329);
print "Node 13, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>13));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
# Node 9
$sub_matrix=$matrix->sub_matrix(column_low=>49,column_high=>54);
print "Node 9, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>670,column_high=>675);
print "Node 9, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1291,column_high=>1296);
print "Node 9, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>9));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
# Node 51
$sub_matrix=$matrix->sub_matrix(column_low=>328,column_high=>333);
print "Node 51, xi1=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>949,column_high=>954);
print "Node 51, xi2=$sub_matrix\n";
$sub_matrix=$matrix->sub_matrix(column_low=>1570,column_high=>1575);
print "Node 51, xi3=$sub_matrix\n";
$nodal_value_var=new Cmiss::Variable::Nodal_value(name=>'nodal_value_var',fe_variable=>$coordinates,node=>$heart->get_node(name=>51));
$d_coordinates=new Cmiss::Variable::Derivative(name=>"d_coordinates",dependent=>$coordinates,independent=>[$nodal_value_var,$element_xi_var]);
$result=$d_coordinates->evaluate($element_xi_var,$element_xi);
print "$result\n";
$element_xi=0;
$coordinates=0;
$heart=0;
$root=0;
