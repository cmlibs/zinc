if (!defined $path)
{
  $path = ".";
}

# testing Cmiss::Node and Cmiss::Element

use Cmiss::Region;
use Cmiss::Function;
use Cmiss::Function::Element_xi;
use Cmiss::Function::Finite_element;
use Cmiss::Function::Matrix;
use Cmiss::Function_variable;
use Cmiss::Node;
use Cmiss::Node_field_creator;
use Cmiss::Element;

#Set up a region
$signals=new Cmiss::Region();

#Create a coordinate field
$coordinates = Cmiss::Function::Finite_element::create_standard_interpolation_rc_constant_time(region=>$signals, name=>"coordinates", number_of_components=>3,component_names=>["x","y","z"]);

#Create a template node and two nodes based on that template
#Using a template is far more efficient especially for multiple fields.
$node_template=new Cmiss::Node(identifier=>1, region=>$signals);

$field_creator = new Cmiss::Node_field_creator(number_of_components=>3);

$coordinates->define_on_Cmiss_node(node=>$node_template, node_field_creator=>$field_creator);

$nodeA=new Cmiss::Node(identifier=>1, template=>$node_template);
$nodeB=new Cmiss::Node(identifier=>2, template=>$node_template);

#Set values into these two nodes
$coordinate_values=$coordinates->nodal_values(node=>$nodeA);
$coordinate_values->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[4,5,6]));

$coordinate_values=$coordinates->nodal_values(node=>$nodeB);
$coordinate_values->set_value(new Cmiss::Function::Matrix(n_columns=>1,values=>[8,9,10]));

#Create an element
$element = Cmiss::Element::create_with_line_shape(identifier=>1001, region=>$signals, dimension=>1);
$coordinates->define_tensor_product_basis_on_element(element=>$element, dimension=>1, basis_type=>LINEAR_LAGRANGE);

$element->set_node(node_index=>0, node=>$nodeA);
$element->set_node(node_index=>1, node=>$nodeB);

#Merge all this into the region
#begin_change and end_change suppress notification of the changes to "clients" such
#as scene viewers.
$signals->begin_change();

$signals->merge_Cmiss_node(node=>$nodeA);
$signals->merge_Cmiss_node(node=>$nodeB);

$signals->merge_Cmiss_element(element=>$element);

$signals->end_change();

#Fetch the nodes and elements separately to check that something is merged in.
$display_node = $signals->get_node(name=>"1");
$values=$coordinates->nodal_values(node=>$display_node);
$output = $values->evaluate();
print "$display_node $coordinates $output\n";

$display_node = $signals->get_node(name=>"2");
$values=$coordinates->nodal_values(node=>$display_node);
$output = $values->evaluate();
print "$display_node $coordinates $output\n";

$display_element = $signals->get_element(name=>"1001");
$element_xi_location = new Cmiss::Function::Element_xi(element=>$display_element,xi=>[0.3]);
$values=$coordinates->output();
$output = $values->evaluate(input=>$coordinates->element_xi(), value=>$element_xi_location);
print "$element_xi_location $coordinates $output\n";

$element_xi_location = new Cmiss::Function::Element_xi(element=>$display_element,xi=>[0.8]);
$output = $values->evaluate(input=>$coordinates->element_xi(), value=>$element_xi_location);
print "$element_xi_location $coordinates $output\n";
